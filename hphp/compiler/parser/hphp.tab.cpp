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
#line 635 "hphp.tab.cpp"

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
     T_LOGICAL_OR = 263,
     T_LOGICAL_XOR = 264,
     T_LOGICAL_AND = 265,
     T_PRINT = 266,
     T_SR_EQUAL = 267,
     T_SL_EQUAL = 268,
     T_XOR_EQUAL = 269,
     T_OR_EQUAL = 270,
     T_AND_EQUAL = 271,
     T_MOD_EQUAL = 272,
     T_CONCAT_EQUAL = 273,
     T_DIV_EQUAL = 274,
     T_MUL_EQUAL = 275,
     T_MINUS_EQUAL = 276,
     T_PLUS_EQUAL = 277,
     T_BOOLEAN_OR = 278,
     T_BOOLEAN_AND = 279,
     T_IS_NOT_IDENTICAL = 280,
     T_IS_IDENTICAL = 281,
     T_IS_NOT_EQUAL = 282,
     T_IS_EQUAL = 283,
     T_IS_GREATER_OR_EQUAL = 284,
     T_IS_SMALLER_OR_EQUAL = 285,
     T_SR = 286,
     T_SL = 287,
     T_INSTANCEOF = 288,
     T_UNSET_CAST = 289,
     T_BOOL_CAST = 290,
     T_OBJECT_CAST = 291,
     T_ARRAY_CAST = 292,
     T_STRING_CAST = 293,
     T_DOUBLE_CAST = 294,
     T_INT_CAST = 295,
     T_DEC = 296,
     T_INC = 297,
     T_CLONE = 298,
     T_NEW = 299,
     T_EXIT = 300,
     T_IF = 301,
     T_ELSEIF = 302,
     T_ELSE = 303,
     T_ENDIF = 304,
     T_LNUMBER = 305,
     T_DNUMBER = 306,
     T_STRING = 307,
     T_STRING_VARNAME = 308,
     T_VARIABLE = 309,
     T_NUM_STRING = 310,
     T_INLINE_HTML = 311,
     T_CHARACTER = 312,
     T_BAD_CHARACTER = 313,
     T_ENCAPSED_AND_WHITESPACE = 314,
     T_CONSTANT_ENCAPSED_STRING = 315,
     T_ECHO = 316,
     T_DO = 317,
     T_WHILE = 318,
     T_ENDWHILE = 319,
     T_FOR = 320,
     T_ENDFOR = 321,
     T_FOREACH = 322,
     T_ENDFOREACH = 323,
     T_DECLARE = 324,
     T_ENDDECLARE = 325,
     T_AS = 326,
     T_SWITCH = 327,
     T_ENDSWITCH = 328,
     T_CASE = 329,
     T_DEFAULT = 330,
     T_BREAK = 331,
     T_GOTO = 332,
     T_CONTINUE = 333,
     T_FUNCTION = 334,
     T_CONST = 335,
     T_RETURN = 336,
     T_TRY = 337,
     T_CATCH = 338,
     T_THROW = 339,
     T_USE = 340,
     T_GLOBAL = 341,
     T_PUBLIC = 342,
     T_PROTECTED = 343,
     T_PRIVATE = 344,
     T_FINAL = 345,
     T_ABSTRACT = 346,
     T_STATIC = 347,
     T_VAR = 348,
     T_UNSET = 349,
     T_ISSET = 350,
     T_EMPTY = 351,
     T_HALT_COMPILER = 352,
     T_CLASS = 353,
     T_INTERFACE = 354,
     T_EXTENDS = 355,
     T_IMPLEMENTS = 356,
     T_OBJECT_OPERATOR = 357,
     T_DOUBLE_ARROW = 358,
     T_LIST = 359,
     T_ARRAY = 360,
     T_CLASS_C = 361,
     T_METHOD_C = 362,
     T_FUNC_C = 363,
     T_LINE = 364,
     T_FILE = 365,
     T_COMMENT = 366,
     T_DOC_COMMENT = 367,
     T_OPEN_TAG = 368,
     T_OPEN_TAG_WITH_ECHO = 369,
     T_CLOSE_TAG = 370,
     T_WHITESPACE = 371,
     T_START_HEREDOC = 372,
     T_END_HEREDOC = 373,
     T_DOLLAR_OPEN_CURLY_BRACES = 374,
     T_CURLY_OPEN = 375,
     T_DOUBLE_COLON = 376,
     T_NAMESPACE = 377,
     T_NS_C = 378,
     T_DIR = 379,
     T_NS_SEPARATOR = 380,
     T_YIELD = 381,
     T_XHP_LABEL = 382,
     T_XHP_TEXT = 383,
     T_XHP_ATTRIBUTE = 384,
     T_XHP_CATEGORY = 385,
     T_XHP_CATEGORY_LABEL = 386,
     T_XHP_CHILDREN = 387,
     T_XHP_ENUM = 388,
     T_XHP_REQUIRED = 389,
     T_TRAIT = 390,
     T_INSTEADOF = 391,
     T_TRAIT_C = 392,
     T_VARARG = 393,
     T_HH_ERROR = 394,
     T_FINALLY = 395,
     T_XHP_TAG_LT = 396,
     T_XHP_TAG_GT = 397,
     T_TYPELIST_LT = 398,
     T_TYPELIST_GT = 399,
     T_UNRESOLVED_LT = 400,
     T_COLLECTION = 401,
     T_SHAPE = 402,
     T_TYPE = 403,
     T_UNRESOLVED_TYPE = 404,
     T_NEWTYPE = 405,
     T_UNRESOLVED_NEWTYPE = 406,
     T_COMPILER_HALT_OFFSET = 407,
     T_AWAIT = 408,
     T_ASYNC = 409,
     T_TUPLE = 410,
     T_FROM = 411,
     T_WHERE = 412,
     T_JOIN = 413,
     T_IN = 414,
     T_ON = 415,
     T_EQUALS = 416,
     T_INTO = 417,
     T_LET = 418,
     T_ORDERBY = 419,
     T_ASCENDING = 420,
     T_DESCENDING = 421,
     T_SELECT = 422,
     T_GROUP = 423,
     T_BY = 424
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
#line 859 "hphp.tab.cpp"

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
#define YYLAST   13088

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  199
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  241
/* YYNRULES -- Number of rules.  */
#define YYNRULES  807
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1504

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   424

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   197,     2,   194,    47,    31,   198,
     189,   190,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   191,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   196,    30,     2,   195,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   192,    29,   193,    50,     2,     2,     2,
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
       5,     6,     7,     9,    10,    11,    12,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    27,    28,
      32,    33,    34,    35,    38,    39,    40,    41,    49,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    62,    63,
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
     184,   185,   186,   187,   188
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
     818,   829,   830,   842,   846,   850,   854,   858,   864,   867,
     870,   871,   878,   884,   889,   893,   895,   897,   901,   906,
     908,   910,   912,   914,   919,   921,   925,   928,   929,   932,
     933,   935,   939,   941,   943,   945,   947,   951,   956,   961,
     966,   968,   970,   973,   976,   979,   983,   987,   989,   991,
     993,   995,   999,  1001,  1005,  1007,  1009,  1011,  1012,  1014,
    1017,  1019,  1021,  1023,  1025,  1027,  1029,  1031,  1033,  1034,
    1036,  1038,  1040,  1044,  1050,  1052,  1056,  1062,  1067,  1071,
    1075,  1078,  1080,  1082,  1086,  1090,  1092,  1094,  1095,  1098,
    1103,  1107,  1114,  1117,  1121,  1128,  1130,  1132,  1134,  1141,
    1145,  1150,  1157,  1161,  1165,  1169,  1173,  1177,  1181,  1185,
    1189,  1193,  1197,  1201,  1204,  1207,  1210,  1213,  1217,  1221,
    1225,  1229,  1233,  1237,  1241,  1245,  1249,  1253,  1257,  1261,
    1265,  1269,  1273,  1277,  1280,  1283,  1286,  1289,  1293,  1297,
    1301,  1305,  1309,  1313,  1317,  1321,  1325,  1329,  1335,  1340,
    1342,  1345,  1348,  1351,  1354,  1357,  1360,  1363,  1366,  1369,
    1371,  1373,  1375,  1379,  1382,  1383,  1395,  1396,  1409,  1411,
    1413,  1419,  1423,  1429,  1433,  1436,  1437,  1440,  1441,  1446,
    1451,  1455,  1460,  1465,  1470,  1475,  1477,  1479,  1483,  1486,
    1490,  1495,  1498,  1502,  1504,  1507,  1509,  1512,  1514,  1516,
    1518,  1520,  1522,  1524,  1529,  1534,  1537,  1546,  1557,  1560,
    1562,  1566,  1568,  1571,  1573,  1575,  1577,  1579,  1582,  1587,
    1591,  1597,  1598,  1602,  1607,  1609,  1612,  1617,  1620,  1627,
    1628,  1630,  1635,  1636,  1639,  1640,  1642,  1644,  1648,  1650,
    1654,  1656,  1658,  1662,  1666,  1668,  1670,  1672,  1674,  1676,
    1678,  1680,  1682,  1684,  1686,  1688,  1690,  1692,  1694,  1696,
    1698,  1700,  1702,  1704,  1706,  1708,  1710,  1712,  1714,  1716,
    1718,  1720,  1722,  1724,  1726,  1728,  1730,  1732,  1734,  1736,
    1738,  1740,  1742,  1744,  1746,  1748,  1750,  1752,  1754,  1756,
    1758,  1760,  1762,  1764,  1766,  1768,  1770,  1772,  1774,  1776,
    1778,  1780,  1782,  1784,  1786,  1788,  1790,  1792,  1794,  1796,
    1798,  1800,  1802,  1804,  1806,  1808,  1810,  1812,  1814,  1816,
    1818,  1820,  1822,  1824,  1826,  1831,  1833,  1835,  1837,  1839,
    1841,  1843,  1845,  1847,  1850,  1852,  1853,  1854,  1856,  1858,
    1862,  1863,  1865,  1867,  1869,  1871,  1873,  1875,  1877,  1879,
    1881,  1883,  1885,  1887,  1891,  1894,  1896,  1898,  1901,  1904,
    1909,  1914,  1918,  1923,  1925,  1927,  1931,  1935,  1937,  1939,
    1941,  1943,  1947,  1951,  1955,  1958,  1959,  1961,  1962,  1964,
    1965,  1971,  1975,  1979,  1981,  1983,  1985,  1987,  1991,  1994,
    1996,  1998,  2000,  2002,  2004,  2007,  2010,  2015,  2020,  2024,
    2029,  2032,  2033,  2039,  2043,  2047,  2049,  2053,  2055,  2058,
    2059,  2065,  2069,  2072,  2073,  2077,  2078,  2083,  2086,  2087,
    2091,  2095,  2097,  2098,  2100,  2103,  2106,  2111,  2115,  2119,
    2122,  2127,  2130,  2135,  2137,  2139,  2141,  2143,  2145,  2148,
    2153,  2157,  2162,  2166,  2168,  2170,  2172,  2174,  2177,  2182,
    2187,  2191,  2193,  2195,  2199,  2207,  2214,  2223,  2233,  2242,
    2253,  2261,  2268,  2277,  2279,  2282,  2287,  2292,  2294,  2296,
    2301,  2303,  2304,  2306,  2309,  2311,  2313,  2316,  2321,  2325,
    2329,  2330,  2332,  2335,  2340,  2344,  2347,  2351,  2358,  2359,
    2361,  2366,  2369,  2370,  2376,  2380,  2384,  2386,  2393,  2398,
    2403,  2406,  2409,  2410,  2416,  2420,  2424,  2426,  2429,  2430,
    2436,  2440,  2444,  2446,  2449,  2452,  2454,  2457,  2459,  2464,
    2468,  2472,  2479,  2483,  2485,  2487,  2489,  2494,  2499,  2504,
    2509,  2512,  2515,  2520,  2523,  2526,  2528,  2532,  2536,  2537,
    2540,  2546,  2553,  2555,  2558,  2560,  2565,  2569,  2570,  2572,
    2576,  2580,  2582,  2584,  2585,  2586,  2589,  2593,  2595,  2601,
    2605,  2609,  2613,  2615,  2618,  2619,  2624,  2627,  2630,  2632,
    2634,  2636,  2641,  2648,  2650,  2659,  2665,  2667
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     200,     0,    -1,    -1,   201,   202,    -1,   202,   203,    -1,
      -1,   217,    -1,   233,    -1,   237,    -1,   242,    -1,   426,
      -1,   116,   189,   190,   191,    -1,   141,   209,   191,    -1,
      -1,   141,   209,   192,   204,   202,   193,    -1,    -1,   141,
     192,   205,   202,   193,    -1,   104,   207,   191,    -1,   214,
     191,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   174,    -1,   176,    -1,   177,    -1,
     179,    -1,   178,    -1,   180,    -1,   181,    -1,   182,    -1,
     183,    -1,   184,    -1,   185,    -1,   186,    -1,   187,    -1,
     188,    -1,   207,     8,   208,    -1,   208,    -1,   209,    -1,
     144,   209,    -1,   209,    90,   206,    -1,   144,   209,    90,
     206,    -1,   206,    -1,   209,   144,   206,    -1,   209,    -1,
     141,   144,   209,    -1,   144,   209,    -1,   210,    -1,   210,
     429,    -1,   210,   429,    -1,   214,     8,   427,    13,   373,
      -1,    99,   427,    13,   373,    -1,   215,   216,    -1,    -1,
     217,    -1,   233,    -1,   237,    -1,   242,    -1,   192,   215,
     193,    -1,    65,   310,   217,   264,   266,    -1,    65,   310,
      26,   215,   265,   267,    68,   191,    -1,    -1,    82,   310,
     218,   258,    -1,    -1,    81,   219,   217,    82,   310,   191,
      -1,    -1,    84,   189,   312,   191,   312,   191,   312,   190,
     220,   256,    -1,    -1,    91,   310,   221,   261,    -1,    95,
     191,    -1,    95,   319,   191,    -1,    97,   191,    -1,    97,
     319,   191,    -1,   100,   191,    -1,   100,   319,   191,    -1,
     145,    95,   191,    -1,   105,   274,   191,    -1,   111,   276,
     191,    -1,    80,   311,   191,    -1,   113,   189,   423,   190,
     191,    -1,   191,    -1,    75,    -1,    -1,    86,   189,   319,
      90,   255,   254,   190,   222,   257,    -1,    88,   189,   260,
     190,   259,    -1,    -1,   101,   225,   102,   189,   366,    73,
     190,   192,   215,   193,   227,   223,   230,    -1,    -1,   101,
     225,   159,   224,   228,    -1,   103,   319,   191,    -1,    96,
     206,   191,    -1,   319,   191,    -1,   313,   191,    -1,   314,
     191,    -1,   315,   191,    -1,   316,   191,    -1,   317,   191,
      -1,   100,   316,   191,    -1,   318,   191,    -1,   335,   191,
      -1,   100,   334,   191,    -1,   206,    26,    -1,    -1,   192,
     226,   215,   193,    -1,   227,   102,   189,   366,    73,   190,
     192,   215,   193,    -1,    -1,    -1,   192,   229,   215,   193,
      -1,   159,   228,    -1,    -1,    31,    -1,    -1,    98,    -1,
      -1,   232,   231,   428,   234,   189,   270,   190,   432,   299,
      -1,    -1,   303,   232,   231,   428,   235,   189,   270,   190,
     432,   299,    -1,    -1,   393,   302,   232,   231,   428,   236,
     189,   270,   190,   432,   299,    -1,    -1,   248,   245,   238,
     249,   250,   192,   277,   193,    -1,    -1,   393,   248,   245,
     239,   249,   250,   192,   277,   193,    -1,    -1,   118,   246,
     240,   251,   192,   277,   193,    -1,    -1,   393,   118,   246,
     241,   251,   192,   277,   193,    -1,    -1,   154,   247,   243,
     250,   192,   277,   193,    -1,    -1,   393,   154,   247,   244,
     250,   192,   277,   193,    -1,   428,    -1,   146,    -1,   428,
      -1,   428,    -1,   117,    -1,   110,   117,    -1,   109,   117,
      -1,   119,   366,    -1,    -1,   120,   252,    -1,    -1,   119,
     252,    -1,    -1,   366,    -1,   252,     8,   366,    -1,   366,
      -1,   253,     8,   366,    -1,   122,   255,    -1,    -1,   400,
      -1,    31,   400,    -1,   123,   189,   412,   190,    -1,   217,
      -1,    26,   215,    85,   191,    -1,   217,    -1,    26,   215,
      87,   191,    -1,   217,    -1,    26,   215,    83,   191,    -1,
     217,    -1,    26,   215,    89,   191,    -1,   206,    13,   373,
      -1,   260,     8,   206,    13,   373,    -1,   192,   262,   193,
      -1,   192,   191,   262,   193,    -1,    26,   262,    92,   191,
      -1,    26,   191,   262,    92,   191,    -1,   262,    93,   319,
     263,   215,    -1,   262,    94,   263,   215,    -1,    -1,    26,
      -1,   191,    -1,   264,    66,   310,   217,    -1,    -1,   265,
      66,   310,    26,   215,    -1,    -1,    67,   217,    -1,    -1,
      67,    26,   215,    -1,    -1,   269,     8,   157,    -1,   269,
     378,    -1,   157,    -1,    -1,   394,   305,   439,    73,    -1,
     394,   305,   439,    31,    73,    -1,   394,   305,   439,    31,
      73,    13,   373,    -1,   394,   305,   439,    73,    13,   373,
      -1,   269,     8,   394,   305,   439,    73,    -1,   269,     8,
     394,   305,   439,    31,    73,    -1,   269,     8,   394,   305,
     439,    31,    73,    13,   373,    -1,   269,     8,   394,   305,
     439,    73,    13,   373,    -1,   271,     8,   157,    -1,   271,
     378,    -1,   157,    -1,    -1,   394,   439,    73,    -1,   394,
     439,    31,    73,    -1,   394,   439,    31,    73,    13,   373,
      -1,   394,   439,    73,    13,   373,    -1,   271,     8,   394,
     439,    73,    -1,   271,     8,   394,   439,    31,    73,    -1,
     271,     8,   394,   439,    31,    73,    13,   373,    -1,   271,
       8,   394,   439,    73,    13,   373,    -1,   273,   378,    -1,
      -1,   319,    -1,    31,   400,    -1,   273,     8,   319,    -1,
     273,     8,    31,   400,    -1,   274,     8,   275,    -1,   275,
      -1,    73,    -1,   194,   400,    -1,   194,   192,   319,   193,
      -1,   276,     8,    73,    -1,   276,     8,    73,    13,   373,
      -1,    73,    -1,    73,    13,   373,    -1,   277,   278,    -1,
      -1,    -1,   301,   279,   307,   191,    -1,    -1,   303,   438,
     280,   307,   191,    -1,   308,   191,    -1,    -1,   302,   232,
     231,   428,   189,   281,   268,   190,   432,   300,    -1,    -1,
     393,   302,   232,   231,   428,   189,   282,   268,   190,   432,
     300,    -1,   148,   287,   191,    -1,   149,   293,   191,    -1,
     151,   295,   191,    -1,   104,   253,   191,    -1,   104,   253,
     192,   283,   193,    -1,   283,   284,    -1,   283,   285,    -1,
      -1,   213,   140,   206,   155,   253,   191,    -1,   286,    90,
     302,   206,   191,    -1,   286,    90,   303,   191,    -1,   213,
     140,   206,    -1,   206,    -1,   288,    -1,   287,     8,   288,
      -1,   289,   363,   291,   292,    -1,   146,    -1,   124,    -1,
     366,    -1,   112,    -1,   152,   192,   290,   193,    -1,   372,
      -1,   290,     8,   372,    -1,    13,   373,    -1,    -1,    51,
     153,    -1,    -1,   294,    -1,   293,     8,   294,    -1,   150,
      -1,   296,    -1,   206,    -1,   115,    -1,   189,   297,   190,
      -1,   189,   297,   190,    45,    -1,   189,   297,   190,    25,
      -1,   189,   297,   190,    42,    -1,   296,    -1,   298,    -1,
     298,    45,    -1,   298,    25,    -1,   298,    42,    -1,   297,
       8,   297,    -1,   297,    29,   297,    -1,   206,    -1,   146,
      -1,   150,    -1,   191,    -1,   192,   215,   193,    -1,   191,
      -1,   192,   215,   193,    -1,   303,    -1,   112,    -1,   303,
      -1,    -1,   304,    -1,   303,   304,    -1,   106,    -1,   107,
      -1,   108,    -1,   111,    -1,   110,    -1,   109,    -1,   173,
      -1,   306,    -1,    -1,   106,    -1,   107,    -1,   108,    -1,
     307,     8,    73,    -1,   307,     8,    73,    13,   373,    -1,
      73,    -1,    73,    13,   373,    -1,   308,     8,   427,    13,
     373,    -1,    99,   427,    13,   373,    -1,   189,   309,   190,
      -1,    63,   368,   371,    -1,    62,   319,    -1,   355,    -1,
     330,    -1,   189,   319,   190,    -1,   311,     8,   319,    -1,
     319,    -1,   311,    -1,    -1,   145,   319,    -1,   145,   319,
     122,   319,    -1,   400,    13,   313,    -1,   123,   189,   412,
     190,    13,   313,    -1,   172,   319,    -1,   400,    13,   316,
      -1,   123,   189,   412,   190,    13,   316,    -1,   320,    -1,
     400,    -1,   309,    -1,   123,   189,   412,   190,    13,   319,
      -1,   400,    13,   319,    -1,   400,    13,    31,   400,    -1,
     400,    13,    31,    63,   368,   371,    -1,   400,    24,   319,
      -1,   400,    23,   319,    -1,   400,    22,   319,    -1,   400,
      21,   319,    -1,   400,    20,   319,    -1,   400,    19,   319,
      -1,   400,    18,   319,    -1,   400,    17,   319,    -1,   400,
      16,   319,    -1,   400,    15,   319,    -1,   400,    14,   319,
      -1,   400,    60,    -1,    60,   400,    -1,   400,    59,    -1,
      59,   400,    -1,   319,    27,   319,    -1,   319,    28,   319,
      -1,   319,     9,   319,    -1,   319,    11,   319,    -1,   319,
      10,   319,    -1,   319,    29,   319,    -1,   319,    31,   319,
      -1,   319,    30,   319,    -1,   319,    44,   319,    -1,   319,
      42,   319,    -1,   319,    43,   319,    -1,   319,    45,   319,
      -1,   319,    46,   319,    -1,   319,    47,   319,    -1,   319,
      41,   319,    -1,   319,    40,   319,    -1,    42,   319,    -1,
      43,   319,    -1,    48,   319,    -1,    50,   319,    -1,   319,
      33,   319,    -1,   319,    32,   319,    -1,   319,    35,   319,
      -1,   319,    34,   319,    -1,   319,    36,   319,    -1,   319,
      39,   319,    -1,   319,    37,   319,    -1,   319,    38,   319,
      -1,   319,    49,   368,    -1,   189,   320,   190,    -1,   319,
      25,   319,    26,   319,    -1,   319,    25,    26,   319,    -1,
     422,    -1,    58,   319,    -1,    57,   319,    -1,    56,   319,
      -1,    55,   319,    -1,    54,   319,    -1,    53,   319,    -1,
      52,   319,    -1,    64,   369,    -1,    51,   319,    -1,   375,
      -1,   329,    -1,   328,    -1,   195,   370,   195,    -1,    12,
     319,    -1,    -1,   232,   231,   189,   321,   270,   190,   432,
     353,   192,   215,   193,    -1,    -1,   303,   232,   231,   189,
     322,   270,   190,   432,   353,   192,   215,   193,    -1,   332,
      -1,    79,    -1,   324,     8,   323,   122,   319,    -1,   323,
     122,   319,    -1,   325,     8,   323,   122,   373,    -1,   323,
     122,   373,    -1,   324,   377,    -1,    -1,   325,   377,    -1,
      -1,   166,   189,   326,   190,    -1,   124,   189,   413,   190,
      -1,    61,   413,   196,    -1,   366,   192,   415,   193,    -1,
     366,   192,   417,   193,    -1,   332,    61,   408,   196,    -1,
     333,    61,   408,   196,    -1,   329,    -1,   424,    -1,   189,
     320,   190,    -1,   336,   337,    -1,   400,    13,   334,    -1,
     175,    73,   178,   319,    -1,   338,   349,    -1,   338,   349,
     352,    -1,   349,    -1,   349,   352,    -1,   339,    -1,   338,
     339,    -1,   340,    -1,   341,    -1,   342,    -1,   343,    -1,
     344,    -1,   345,    -1,   175,    73,   178,   319,    -1,   182,
      73,    13,   319,    -1,   176,   319,    -1,   177,    73,   178,
     319,   179,   319,   180,   319,    -1,   177,    73,   178,   319,
     179,   319,   180,   319,   181,    73,    -1,   183,   346,    -1,
     347,    -1,   346,     8,   347,    -1,   319,    -1,   319,   348,
      -1,   184,    -1,   185,    -1,   350,    -1,   351,    -1,   186,
     319,    -1,   187,   319,   188,   319,    -1,   181,    73,   337,
      -1,   104,   189,   354,   378,   190,    -1,    -1,   354,     8,
      73,    -1,   354,     8,    31,    73,    -1,    73,    -1,    31,
      73,    -1,   160,   146,   356,   161,    -1,   358,    46,    -1,
     358,   161,   359,   160,    46,   357,    -1,    -1,   146,    -1,
     358,   360,    13,   361,    -1,    -1,   359,   362,    -1,    -1,
     146,    -1,   147,    -1,   192,   319,   193,    -1,   147,    -1,
     192,   319,   193,    -1,   355,    -1,   364,    -1,   363,    26,
     364,    -1,   363,    43,   364,    -1,   206,    -1,    64,    -1,
      98,    -1,    99,    -1,   100,    -1,   145,    -1,   172,    -1,
     101,    -1,   102,    -1,   159,    -1,   103,    -1,    65,    -1,
      66,    -1,    68,    -1,    67,    -1,    82,    -1,    83,    -1,
      81,    -1,    84,    -1,    85,    -1,    86,    -1,    87,    -1,
      88,    -1,    89,    -1,    49,    -1,    90,    -1,    91,    -1,
      92,    -1,    93,    -1,    94,    -1,    95,    -1,    97,    -1,
      96,    -1,    80,    -1,    12,    -1,   117,    -1,   118,    -1,
     119,    -1,   120,    -1,    63,    -1,    62,    -1,   112,    -1,
       5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,
     141,    -1,   104,    -1,   105,    -1,   114,    -1,   115,    -1,
     116,    -1,   111,    -1,   110,    -1,   109,    -1,   108,    -1,
     107,    -1,   106,    -1,   173,    -1,   113,    -1,   123,    -1,
     124,    -1,     9,    -1,    11,    -1,    10,    -1,   125,    -1,
     127,    -1,   126,    -1,   128,    -1,   129,    -1,   143,    -1,
     142,    -1,   171,    -1,   154,    -1,   156,    -1,   155,    -1,
     167,    -1,   169,    -1,   166,    -1,   212,   189,   272,   190,
      -1,   213,    -1,   146,    -1,   366,    -1,   111,    -1,   406,
      -1,   366,    -1,   111,    -1,   410,    -1,   189,   190,    -1,
     310,    -1,    -1,    -1,    78,    -1,   419,    -1,   189,   272,
     190,    -1,    -1,    69,    -1,    70,    -1,    79,    -1,   128,
      -1,   129,    -1,   143,    -1,   125,    -1,   156,    -1,   126,
      -1,   127,    -1,   142,    -1,   171,    -1,   136,    78,   137,
      -1,   136,   137,    -1,   372,    -1,   211,    -1,    42,   373,
      -1,    43,   373,    -1,   124,   189,   376,   190,    -1,   174,
     189,   376,   190,    -1,    61,   376,   196,    -1,   166,   189,
     327,   190,    -1,   374,    -1,   331,    -1,   213,   140,   206,
      -1,   146,   140,   206,    -1,   211,    -1,    72,    -1,   424,
      -1,   372,    -1,   197,   419,   197,    -1,   198,   419,   198,
      -1,   136,   419,   137,    -1,   379,   377,    -1,    -1,     8,
      -1,    -1,     8,    -1,    -1,   379,     8,   373,   122,   373,
      -1,   379,     8,   373,    -1,   373,   122,   373,    -1,   373,
      -1,    69,    -1,    70,    -1,    79,    -1,   136,    78,   137,
      -1,   136,   137,    -1,    69,    -1,    70,    -1,   206,    -1,
     380,    -1,   206,    -1,    42,   381,    -1,    43,   381,    -1,
     124,   189,   383,   190,    -1,   174,   189,   383,   190,    -1,
      61,   383,   196,    -1,   166,   189,   386,   190,    -1,   384,
     377,    -1,    -1,   384,     8,   382,   122,   382,    -1,   384,
       8,   382,    -1,   382,   122,   382,    -1,   382,    -1,   385,
       8,   382,    -1,   382,    -1,   387,   377,    -1,    -1,   387,
       8,   323,   122,   382,    -1,   323,   122,   382,    -1,   385,
     377,    -1,    -1,   189,   388,   190,    -1,    -1,   390,     8,
     206,   389,    -1,   206,   389,    -1,    -1,   392,   390,   377,
      -1,    41,   391,    40,    -1,   393,    -1,    -1,   396,    -1,
     121,   405,    -1,   121,   206,    -1,   121,   192,   319,   193,
      -1,    61,   408,   196,    -1,   192,   319,   193,    -1,   401,
     397,    -1,   189,   309,   190,   397,    -1,   411,   397,    -1,
     189,   309,   190,   397,    -1,   405,    -1,   365,    -1,   403,
      -1,   404,    -1,   398,    -1,   400,   395,    -1,   189,   309,
     190,   395,    -1,   367,   140,   405,    -1,   402,   189,   272,
     190,    -1,   189,   400,   190,    -1,   365,    -1,   403,    -1,
     404,    -1,   398,    -1,   400,   396,    -1,   189,   309,   190,
     396,    -1,   402,   189,   272,   190,    -1,   189,   400,   190,
      -1,   405,    -1,   398,    -1,   189,   400,   190,    -1,   400,
     121,   206,   429,   189,   272,   190,    -1,   400,   121,   405,
     189,   272,   190,    -1,   400,   121,   192,   319,   193,   189,
     272,   190,    -1,   189,   309,   190,   121,   206,   429,   189,
     272,   190,    -1,   189,   309,   190,   121,   405,   189,   272,
     190,    -1,   189,   309,   190,   121,   192,   319,   193,   189,
     272,   190,    -1,   367,   140,   206,   429,   189,   272,   190,
      -1,   367,   140,   405,   189,   272,   190,    -1,   367,   140,
     192,   319,   193,   189,   272,   190,    -1,   406,    -1,   409,
     406,    -1,   406,    61,   408,   196,    -1,   406,   192,   319,
     193,    -1,   407,    -1,    73,    -1,   194,   192,   319,   193,
      -1,   319,    -1,    -1,   194,    -1,   409,   194,    -1,   405,
      -1,   399,    -1,   410,   395,    -1,   189,   309,   190,   395,
      -1,   367,   140,   405,    -1,   189,   400,   190,    -1,    -1,
     399,    -1,   410,   396,    -1,   189,   309,   190,   396,    -1,
     189,   400,   190,    -1,   412,     8,    -1,   412,     8,   400,
      -1,   412,     8,   123,   189,   412,   190,    -1,    -1,   400,
      -1,   123,   189,   412,   190,    -1,   414,   377,    -1,    -1,
     414,     8,   319,   122,   319,    -1,   414,     8,   319,    -1,
     319,   122,   319,    -1,   319,    -1,   414,     8,   319,   122,
      31,   400,    -1,   414,     8,    31,   400,    -1,   319,   122,
      31,   400,    -1,    31,   400,    -1,   416,   377,    -1,    -1,
     416,     8,   319,   122,   319,    -1,   416,     8,   319,    -1,
     319,   122,   319,    -1,   319,    -1,   418,   377,    -1,    -1,
     418,     8,   373,   122,   373,    -1,   418,     8,   373,    -1,
     373,   122,   373,    -1,   373,    -1,   419,   420,    -1,   419,
      78,    -1,   420,    -1,    78,   420,    -1,    73,    -1,    73,
      61,   421,   196,    -1,    73,   121,   206,    -1,   138,   319,
     193,    -1,   138,    72,    61,   319,   196,   193,    -1,   139,
     400,   193,    -1,   206,    -1,    74,    -1,    73,    -1,   114,
     189,   423,   190,    -1,   115,   189,   400,   190,    -1,   115,
     189,   320,   190,    -1,   115,   189,   309,   190,    -1,     7,
     319,    -1,     6,   319,    -1,     5,   189,   319,   190,    -1,
       4,   319,    -1,     3,   319,    -1,   400,    -1,   423,     8,
     400,    -1,   367,   140,   206,    -1,    -1,    90,   438,    -1,
     167,   428,    13,   438,   191,    -1,   169,   428,   425,    13,
     438,   191,    -1,   206,    -1,   438,   206,    -1,   206,    -1,
     206,   162,   433,   163,    -1,   162,   430,   163,    -1,    -1,
     438,    -1,   430,     8,   438,    -1,   430,     8,   157,    -1,
     430,    -1,   157,    -1,    -1,    -1,    26,   438,    -1,   433,
       8,   206,    -1,   206,    -1,   433,     8,   206,    90,   438,
      -1,   206,    90,   438,    -1,    79,   122,   438,    -1,   435,
       8,   434,    -1,   434,    -1,   435,   377,    -1,    -1,   166,
     189,   436,   190,    -1,    25,   438,    -1,    51,   438,    -1,
     213,    -1,   124,    -1,   437,    -1,   124,   162,   438,   163,
      -1,   124,   162,   438,     8,   438,   163,    -1,   146,    -1,
     189,    98,   189,   431,   190,    26,   438,   190,    -1,   189,
     430,     8,   438,   190,    -1,   438,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   716,   716,   716,   725,   727,   730,   731,   732,   733,
     734,   735,   738,   740,   740,   742,   742,   744,   745,   750,
     751,   752,   753,   754,   755,   756,   757,   758,   759,   760,
     761,   762,   763,   764,   765,   766,   767,   768,   769,   773,
     775,   778,   779,   780,   781,   786,   787,   791,   792,   794,
     797,   803,   810,   817,   821,   827,   829,   832,   833,   834,
     835,   838,   839,   843,   848,   848,   854,   854,   861,   860,
     866,   866,   871,   872,   873,   874,   875,   876,   877,   878,
     879,   880,   881,   882,   883,   886,   884,   891,   899,   893,
     903,   901,   905,   906,   910,   911,   912,   913,   914,   915,
     916,   917,   918,   919,   920,   928,   928,   933,   939,   943,
     943,   951,   952,   956,   957,   961,   966,   965,   978,   976,
     990,   988,  1004,  1003,  1022,  1020,  1039,  1038,  1047,  1045,
    1057,  1056,  1068,  1066,  1079,  1080,  1084,  1087,  1090,  1091,
    1092,  1095,  1097,  1100,  1101,  1104,  1105,  1108,  1109,  1113,
    1114,  1119,  1120,  1123,  1124,  1125,  1129,  1130,  1134,  1135,
    1139,  1140,  1144,  1145,  1150,  1151,  1156,  1157,  1158,  1159,
    1162,  1165,  1167,  1170,  1171,  1175,  1177,  1180,  1183,  1186,
    1187,  1190,  1191,  1195,  1197,  1199,  1200,  1204,  1208,  1212,
    1217,  1222,  1227,  1232,  1238,  1247,  1249,  1251,  1252,  1256,
    1259,  1262,  1266,  1270,  1274,  1278,  1283,  1291,  1293,  1296,
    1297,  1298,  1300,  1305,  1306,  1309,  1310,  1311,  1315,  1316,
    1318,  1319,  1323,  1325,  1328,  1328,  1332,  1331,  1335,  1339,
    1337,  1352,  1349,  1362,  1364,  1366,  1368,  1370,  1374,  1375,
    1376,  1379,  1385,  1388,  1394,  1397,  1402,  1404,  1409,  1414,
    1418,  1419,  1425,  1426,  1431,  1432,  1437,  1438,  1442,  1443,
    1447,  1449,  1455,  1460,  1461,  1463,  1467,  1468,  1469,  1470,
    1474,  1475,  1476,  1477,  1478,  1479,  1481,  1486,  1489,  1490,
    1494,  1495,  1499,  1500,  1503,  1504,  1507,  1508,  1511,  1512,
    1516,  1517,  1518,  1519,  1520,  1521,  1522,  1526,  1527,  1530,
    1531,  1532,  1535,  1537,  1539,  1540,  1543,  1545,  1549,  1550,
    1552,  1553,  1554,  1557,  1561,  1562,  1566,  1567,  1571,  1572,
    1576,  1580,  1585,  1589,  1593,  1598,  1599,  1600,  1603,  1605,
    1606,  1607,  1610,  1611,  1612,  1613,  1614,  1615,  1616,  1617,
    1618,  1619,  1620,  1621,  1622,  1623,  1624,  1625,  1626,  1627,
    1628,  1629,  1630,  1631,  1632,  1633,  1634,  1635,  1636,  1637,
    1638,  1639,  1640,  1641,  1642,  1643,  1644,  1645,  1646,  1647,
    1648,  1649,  1650,  1652,  1653,  1655,  1657,  1658,  1659,  1660,
    1661,  1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,
    1671,  1672,  1673,  1674,  1676,  1675,  1688,  1687,  1699,  1703,
    1707,  1711,  1717,  1721,  1727,  1729,  1733,  1735,  1739,  1743,
    1744,  1748,  1755,  1762,  1764,  1769,  1770,  1771,  1775,  1779,
    1783,  1787,  1789,  1791,  1793,  1798,  1799,  1804,  1805,  1806,
    1807,  1808,  1809,  1813,  1817,  1821,  1825,  1830,  1835,  1839,
    1840,  1844,  1845,  1849,  1850,  1854,  1855,  1859,  1863,  1867,
    1871,  1873,  1877,  1878,  1879,  1880,  1884,  1890,  1899,  1912,
    1913,  1916,  1919,  1922,  1923,  1926,  1930,  1933,  1936,  1943,
    1944,  1948,  1949,  1951,  1955,  1956,  1957,  1958,  1959,  1960,
    1961,  1962,  1963,  1964,  1965,  1966,  1967,  1968,  1969,  1970,
    1971,  1972,  1973,  1974,  1975,  1976,  1977,  1978,  1979,  1980,
    1981,  1982,  1983,  1984,  1985,  1986,  1987,  1988,  1989,  1990,
    1991,  1992,  1993,  1994,  1995,  1996,  1997,  1998,  1999,  2000,
    2001,  2002,  2003,  2004,  2005,  2006,  2007,  2008,  2009,  2010,
    2011,  2012,  2013,  2014,  2015,  2016,  2017,  2018,  2019,  2020,
    2021,  2022,  2023,  2024,  2025,  2026,  2027,  2028,  2029,  2030,
    2031,  2032,  2033,  2034,  2038,  2043,  2044,  2047,  2048,  2049,
    2053,  2054,  2055,  2059,  2060,  2061,  2065,  2066,  2067,  2070,
    2072,  2076,  2077,  2078,  2080,  2081,  2082,  2083,  2084,  2085,
    2086,  2087,  2088,  2089,  2092,  2097,  2098,  2099,  2100,  2101,
    2103,  2105,  2106,  2108,  2109,  2113,  2116,  2122,  2123,  2124,
    2125,  2126,  2127,  2128,  2133,  2135,  2139,  2140,  2143,  2144,
    2148,  2151,  2153,  2155,  2159,  2160,  2161,  2163,  2166,  2170,
    2171,  2172,  2175,  2176,  2177,  2178,  2179,  2181,  2183,  2184,
    2189,  2191,  2194,  2197,  2199,  2201,  2204,  2206,  2210,  2212,
    2215,  2218,  2224,  2226,  2229,  2230,  2235,  2238,  2242,  2242,
    2247,  2250,  2251,  2255,  2256,  2261,  2262,  2266,  2267,  2271,
    2272,  2277,  2279,  2284,  2285,  2286,  2287,  2288,  2289,  2290,
    2292,  2295,  2297,  2301,  2302,  2303,  2304,  2305,  2307,  2309,
    2311,  2315,  2316,  2317,  2321,  2324,  2327,  2330,  2334,  2338,
    2345,  2349,  2353,  2360,  2361,  2366,  2368,  2369,  2372,  2373,
    2376,  2377,  2381,  2382,  2386,  2387,  2388,  2389,  2391,  2394,
    2397,  2398,  2399,  2401,  2403,  2407,  2408,  2409,  2411,  2412,
    2413,  2417,  2419,  2422,  2424,  2425,  2426,  2427,  2430,  2432,
    2433,  2437,  2439,  2442,  2444,  2445,  2446,  2450,  2452,  2455,
    2458,  2460,  2462,  2466,  2467,  2469,  2470,  2476,  2477,  2479,
    2481,  2483,  2485,  2488,  2489,  2490,  2494,  2495,  2496,  2497,
    2498,  2499,  2500,  2501,  2502,  2506,  2507,  2511,  2519,  2521,
    2525,  2528,  2534,  2535,  2541,  2542,  2549,  2552,  2556,  2559,
    2564,  2565,  2566,  2567,  2571,  2572,  2576,  2578,  2579,  2581,
    2585,  2591,  2593,  2597,  2600,  2603,  2611,  2614,  2617,  2618,
    2621,  2622,  2625,  2629,  2633,  2639,  2647,  2648
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_REQUIRE_ONCE", "T_REQUIRE", "T_EVAL",
  "T_INCLUDE_ONCE", "T_INCLUDE", "','", "T_LOGICAL_OR", "T_LOGICAL_XOR",
  "T_LOGICAL_AND", "T_PRINT", "'='", "T_SR_EQUAL", "T_SL_EQUAL",
  "T_XOR_EQUAL", "T_OR_EQUAL", "T_AND_EQUAL", "T_MOD_EQUAL",
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
  "T_BY", "'('", "')'", "';'", "'{'", "'}'", "'$'", "'`'", "']'", "'\"'",
  "'\\''", "$accept", "start", "$@1", "top_statement_list",
  "top_statement", "$@2", "$@3", "ident", "use_declarations",
  "use_declaration", "namespace_name", "namespace_string_base",
  "namespace_string", "namespace_string_typeargs",
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
  "expr_no_variable", "$@26", "$@27", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "collection_literal", "static_collection_literal",
  "dim_expr", "dim_expr_base", "query_expr", "query_assign_expr",
  "query_head", "query_body", "query_body_clauses", "query_body_clause",
  "from_clause", "let_clause", "where_clause", "join_clause",
  "join_into_clause", "orderby_clause", "orderings", "ordering",
  "ordering_direction", "select_or_group_clause", "select_clause",
  "group_clause", "query_continuation", "lexical_vars", "lexical_var_list",
  "xhp_tag", "xhp_tag_body", "xhp_opt_end_label", "xhp_attributes",
  "xhp_children", "xhp_attribute_name", "xhp_attribute_value", "xhp_child",
  "xhp_label_ws", "xhp_bareword", "simple_function_call",
  "fully_qualified_class_name", "static_class_name",
  "class_name_reference", "exit_expr", "backticks_expr", "ctor_arguments",
  "common_scalar", "static_scalar", "static_class_constant", "scalar",
  "static_array_pair_list", "possible_comma", "hh_possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_shape_pair_list_ae",
  "non_empty_static_shape_pair_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@28", "non_empty_user_attributes",
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
       0,   256,   257,   258,   259,   260,   261,   262,    44,   263,
     264,   265,   266,    61,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,    63,    58,   278,   279,   124,
      94,    38,   280,   281,   282,   283,    60,    62,   284,   285,
     286,   287,    43,    45,    46,    42,    47,    37,    33,   288,
     126,    64,   289,   290,   291,   292,   293,   294,   295,   296,
     297,    91,   298,   299,   300,   301,   302,   303,   304,   305,
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
     416,   417,   418,   419,   420,   421,   422,   423,   424,    40,
      41,    59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   199,   201,   200,   202,   202,   203,   203,   203,   203,
     203,   203,   203,   204,   203,   205,   203,   203,   203,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   206,
     206,   206,   206,   206,   206,   206,   206,   206,   206,   207,
     207,   208,   208,   208,   208,   209,   209,   210,   210,   210,
     211,   212,   213,   214,   214,   215,   215,   216,   216,   216,
     216,   217,   217,   217,   218,   217,   219,   217,   220,   217,
     221,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   222,   217,   217,   223,   217,
     224,   217,   217,   217,   217,   217,   217,   217,   217,   217,
     217,   217,   217,   217,   217,   226,   225,   227,   227,   229,
     228,   230,   230,   231,   231,   232,   234,   233,   235,   233,
     236,   233,   238,   237,   239,   237,   240,   237,   241,   237,
     243,   242,   244,   242,   245,   245,   246,   247,   248,   248,
     248,   249,   249,   250,   250,   251,   251,   252,   252,   253,
     253,   254,   254,   255,   255,   255,   256,   256,   257,   257,
     258,   258,   259,   259,   260,   260,   261,   261,   261,   261,
     262,   262,   262,   263,   263,   264,   264,   265,   265,   266,
     266,   267,   267,   268,   268,   268,   268,   269,   269,   269,
     269,   269,   269,   269,   269,   270,   270,   270,   270,   271,
     271,   271,   271,   271,   271,   271,   271,   272,   272,   273,
     273,   273,   273,   274,   274,   275,   275,   275,   276,   276,
     276,   276,   277,   277,   279,   278,   280,   278,   278,   281,
     278,   282,   278,   278,   278,   278,   278,   278,   283,   283,
     283,   284,   285,   285,   286,   286,   287,   287,   288,   288,
     289,   289,   289,   289,   290,   290,   291,   291,   292,   292,
     293,   293,   294,   295,   295,   295,   296,   296,   296,   296,
     297,   297,   297,   297,   297,   297,   297,   298,   298,   298,
     299,   299,   300,   300,   301,   301,   302,   302,   303,   303,
     304,   304,   304,   304,   304,   304,   304,   305,   305,   306,
     306,   306,   307,   307,   307,   307,   308,   308,   309,   309,
     309,   309,   309,   310,   311,   311,   312,   312,   313,   313,
     314,   315,   316,   317,   318,   319,   319,   319,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   321,   320,   322,   320,   320,   323,
     324,   324,   325,   325,   326,   326,   327,   327,   328,   329,
     329,   330,   331,   332,   332,   333,   333,   333,   334,   335,
     336,   337,   337,   337,   337,   338,   338,   339,   339,   339,
     339,   339,   339,   340,   341,   342,   343,   344,   345,   346,
     346,   347,   347,   348,   348,   349,   349,   350,   351,   352,
     353,   353,   354,   354,   354,   354,   355,   356,   356,   357,
     357,   358,   358,   359,   359,   360,   361,   361,   362,   362,
     362,   363,   363,   363,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   365,   366,   366,   367,   367,   367,
     368,   368,   368,   369,   369,   369,   370,   370,   370,   371,
     371,   372,   372,   372,   372,   372,   372,   372,   372,   372,
     372,   372,   372,   372,   372,   373,   373,   373,   373,   373,
     373,   373,   373,   373,   373,   374,   374,   375,   375,   375,
     375,   375,   375,   375,   376,   376,   377,   377,   378,   378,
     379,   379,   379,   379,   380,   380,   380,   380,   380,   381,
     381,   381,   382,   382,   382,   382,   382,   382,   382,   382,
     383,   383,   384,   384,   384,   384,   385,   385,   386,   386,
     387,   387,   388,   388,   389,   389,   390,   390,   392,   391,
     393,   394,   394,   395,   395,   396,   396,   397,   397,   398,
     398,   399,   399,   400,   400,   400,   400,   400,   400,   400,
     400,   400,   400,   401,   401,   401,   401,   401,   401,   401,
     401,   402,   402,   402,   403,   403,   403,   403,   403,   403,
     404,   404,   404,   405,   405,   406,   406,   406,   407,   407,
     408,   408,   409,   409,   410,   410,   410,   410,   410,   410,
     411,   411,   411,   411,   411,   412,   412,   412,   412,   412,
     412,   413,   413,   414,   414,   414,   414,   414,   414,   414,
     414,   415,   415,   416,   416,   416,   416,   417,   417,   418,
     418,   418,   418,   419,   419,   419,   419,   420,   420,   420,
     420,   420,   420,   421,   421,   421,   422,   422,   422,   422,
     422,   422,   422,   422,   422,   423,   423,   424,   425,   425,
     426,   426,   427,   427,   428,   428,   429,   429,   430,   430,
     431,   431,   431,   431,   432,   432,   433,   433,   433,   433,
     434,   435,   435,   436,   436,   437,   438,   438,   438,   438,
     438,   438,   438,   438,   438,   438,   439,   439
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
      10,     0,    11,     3,     3,     3,     3,     5,     2,     2,
       0,     6,     5,     4,     3,     1,     1,     3,     4,     1,
       1,     1,     1,     4,     1,     3,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     3,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     5,     1,     3,     5,     4,     3,     3,
       2,     1,     1,     3,     3,     1,     1,     0,     2,     4,
       3,     6,     2,     3,     6,     1,     1,     1,     6,     3,
       4,     6,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     5,     4,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       1,     1,     3,     2,     0,    11,     0,    12,     1,     1,
       5,     3,     5,     3,     2,     0,     2,     0,     4,     4,
       3,     4,     4,     4,     4,     1,     1,     3,     2,     3,
       4,     2,     3,     1,     2,     1,     2,     1,     1,     1,
       1,     1,     1,     4,     4,     2,     8,    10,     2,     1,
       3,     1,     2,     1,     1,     1,     1,     2,     4,     3,
       5,     0,     3,     4,     1,     2,     4,     2,     6,     0,
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
       0,   648,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   722,     0,   710,   565,
       0,   571,   572,    19,   598,   698,    84,   573,     0,    66,
       0,     0,     0,     0,     0,     0,     0,     0,   115,     0,
       0,     0,     0,     0,     0,   290,   291,   292,   295,   294,
     293,     0,     0,     0,     0,   138,     0,     0,     0,   577,
     579,   580,   574,   575,     0,     0,   581,   576,     0,     0,
     556,    20,    21,    22,    24,    23,     0,   578,     0,     0,
       0,     0,   582,     0,   296,    25,    26,    27,    29,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,     0,
      83,    56,   702,   566,     0,     0,     4,    45,    47,    50,
     597,     0,   555,     0,     6,   114,     7,     8,     9,     0,
       0,   288,   327,     0,     0,     0,     0,     0,     0,     0,
     325,   391,   390,   312,   398,     0,     0,   311,   664,   557,
       0,   600,   389,   287,   667,   326,     0,     0,   665,   666,
     663,   693,   697,     0,   379,   599,    10,   295,   294,   293,
       0,     0,    45,   114,     0,   764,   326,   763,     0,   761,
     760,   393,     0,     0,   363,   364,   365,   366,   388,   386,
     385,   384,   383,   382,   381,   380,   558,     0,   777,   557,
       0,   346,   344,     0,   726,     0,   607,   310,   561,     0,
     777,   560,     0,   570,   705,   704,   562,     0,     0,   564,
     387,     0,     0,     0,   315,     0,    64,   317,     0,     0,
      70,    72,     0,     0,    74,     0,     0,     0,   799,   803,
       0,     0,    45,   798,     0,   800,     0,     0,    76,     0,
       0,     0,     0,   105,     0,     0,     0,     0,    40,    41,
     215,     0,     0,   214,   140,   139,   220,     0,     0,     0,
       0,     0,   774,   126,   136,   718,   722,   747,     0,   584,
       0,     0,     0,   745,     0,    15,     0,    49,     0,   318,
     130,   137,   462,   405,     0,   768,   322,   327,     0,   325,
     326,     0,     0,   567,     0,   568,     0,     0,     0,   104,
       0,     0,    52,   208,     0,    18,   113,     0,   135,   122,
     134,   293,   114,   289,    95,    96,    97,    98,    99,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   710,    94,   701,   701,   102,
     732,     0,     0,     0,     0,     0,   286,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   345,
     343,     0,   668,   653,   701,     0,   659,   208,   701,     0,
     703,   694,   718,     0,   114,     0,     0,   650,   645,   607,
       0,     0,     0,     0,   730,     0,   410,   606,   721,     0,
       0,    52,     0,   208,   309,     0,   706,   653,   661,   563,
       0,    56,   176,     0,    81,     0,     0,   316,     0,     0,
       0,     0,     0,    73,    93,    75,   796,   797,     0,   794,
       0,     0,   778,     0,   773,     0,   100,    77,   103,     0,
       0,     0,     0,     0,     0,     0,   418,     0,   425,   427,
     428,   429,   430,   431,   432,   423,   445,   446,    56,     0,
      90,    92,    42,     0,    17,     0,     0,   216,     0,    79,
       0,     0,    80,   765,     0,     0,   327,   325,   326,     0,
       0,   146,     0,   719,     0,     0,     0,     0,   583,   746,
     598,     0,     0,   744,   603,   743,    48,     5,    12,    13,
      78,     0,   144,     0,     0,   399,     0,   607,     0,     0,
       0,     0,   308,   376,   672,    61,    55,    57,    58,    59,
      60,     0,   392,   601,   602,    46,     0,     0,     0,   609,
     209,     0,   394,   116,   142,     0,   349,   351,   350,     0,
       0,   347,   348,   352,   354,   353,   368,   367,   370,   369,
     371,   373,   374,   372,   362,   361,   356,   357,   355,   358,
     359,   360,   375,   700,     0,     0,   736,     0,   607,     0,
     767,   670,   693,   128,   132,   124,   114,     0,     0,   320,
     323,   329,   419,   342,   341,   340,   339,   338,   337,   336,
     335,   334,   333,   332,     0,   655,   654,     0,     0,     0,
       0,     0,     0,     0,   762,   643,   647,   606,   649,     0,
       0,   777,     0,   725,     0,   724,     0,   709,   708,     0,
       0,   655,   654,   313,   178,   180,   314,     0,    56,   160,
      65,   317,     0,     0,     0,     0,   172,   172,    71,     0,
       0,   792,   607,     0,   783,     0,     0,     0,   605,     0,
       0,   556,     0,    25,    50,   586,   555,   594,     0,   585,
      54,   593,     0,     0,   435,     0,     0,   441,   438,   439,
     447,     0,   426,   421,     0,   424,     0,     0,     0,     0,
      39,    43,     0,   213,   221,   218,     0,     0,   756,   759,
     758,   757,    11,   787,     0,     0,     0,   718,   715,     0,
     409,   755,   754,   753,     0,   749,     0,   750,   752,     0,
       5,   319,     0,     0,   456,   457,   465,   464,     0,     0,
     606,   404,   408,     0,   769,     0,     0,   669,   653,   660,
     699,     0,   776,   210,   554,   608,   207,     0,   652,     0,
       0,   144,   396,   118,   378,     0,   413,   414,     0,   411,
     606,   731,     0,     0,   208,   146,   144,   142,     0,   710,
     330,     0,     0,   208,   657,   658,   671,   695,   696,     0,
       0,     0,   631,   614,   615,   616,     0,     0,     0,    25,
     623,   622,   637,   607,     0,   645,   729,   728,     0,   707,
     653,   662,   569,     0,   182,     0,     0,    62,     0,     0,
       0,     0,     0,   152,   153,   164,     0,    56,   162,    87,
     172,     0,   172,     0,     0,   801,     0,   606,   793,   795,
     782,   781,     0,   779,   587,   588,   613,     0,   607,   605,
       0,     0,   407,   605,     0,   738,   420,     0,     0,     0,
     443,   444,   442,     0,     0,   422,     0,   106,     0,   109,
      91,    44,   217,     0,   766,    82,     0,     0,   775,   145,
     147,   223,     0,     0,   716,     0,   748,     0,    16,     0,
     143,   223,     0,     0,   401,     0,   770,     0,     0,   655,
     654,   779,     0,   211,    53,   197,     0,   609,   651,   807,
     652,   141,     0,   652,     0,   377,   735,   734,     0,   208,
       0,     0,     0,   144,   120,   570,   656,   208,     0,     0,
     619,   620,   621,   624,   625,   635,     0,   607,   631,     0,
     618,   639,   631,   606,   642,   644,   646,     0,   723,   656,
       0,     0,     0,     0,   179,    67,     0,   317,   154,   718,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   166,
       0,   790,   791,     0,     0,   805,     0,   591,   606,   604,
       0,   596,     0,   607,     0,     0,   595,   742,     0,   607,
     433,     0,   434,   440,   448,   449,     0,    56,   219,   789,
     786,     0,   287,   720,   718,   321,   324,   328,     0,    14,
     287,   468,     0,     0,   470,   463,   466,     0,   461,     0,
     771,     0,     0,   208,   212,   784,   652,   196,   806,     0,
       0,   223,     0,   652,     0,   208,     0,   691,   223,   223,
       0,     0,   331,   208,     0,   685,     0,   628,   606,   630,
       0,   617,     0,     0,   607,     0,   636,   727,     0,    56,
       0,   175,   161,     0,     0,   151,    85,   165,     0,     0,
     168,     0,   173,   174,    56,   167,   802,   780,     0,   612,
     611,   589,     0,   606,   406,   592,   590,     0,   412,   606,
     737,     0,     0,     0,     0,   148,     0,     0,   285,     0,
       0,     0,   127,   222,   224,     0,   284,     0,   287,     0,
     751,   131,   459,     0,     0,   400,   656,   208,     0,     0,
     451,   195,   807,     0,   199,   784,   287,   784,     0,   733,
       0,   690,   287,   287,   223,   652,     0,   684,   634,   633,
     626,     0,   629,   606,   638,   627,    56,   181,    63,    68,
     155,     0,   163,   169,    56,   171,     0,     0,   403,     0,
     741,   740,     0,    56,   110,   788,     0,     0,   149,   252,
     250,   556,    24,     0,   246,     0,   251,   262,     0,   260,
     265,     0,   264,     0,   263,     0,   114,   226,     0,   228,
       0,   717,   460,   458,   469,   467,   208,     0,   688,   785,
       0,     0,     0,   200,     0,     0,   123,   451,   784,   692,
     129,   133,   287,     0,   686,     0,   641,     0,   177,     0,
      56,   158,    86,   170,   804,   610,     0,     0,     0,     0,
       0,     0,   236,   240,     0,     0,   233,   520,   519,   516,
     518,   517,   537,   539,   538,   508,   498,   514,   513,   475,
     485,   486,   488,   487,   507,   491,   489,   490,   492,   493,
     494,   495,   496,   497,   499,   500,   501,   502,   503,   504,
     506,   505,   476,   477,   478,   481,   482,   484,   522,   523,
     532,   531,   530,   529,   528,   527,   515,   534,   524,   525,
     526,   509,   510,   511,   512,   535,   536,   540,   542,   541,
     543,   544,   521,   546,   545,   479,   548,   550,   549,   483,
     553,   551,   552,   547,   480,   533,   474,   257,   471,     0,
     234,   278,   279,   277,   270,     0,   271,   235,   304,     0,
       0,     0,     0,   114,     0,   687,     0,    56,     0,   203,
       0,   202,   280,    56,   117,     0,     0,   125,   784,   632,
       0,    56,   156,    69,     0,   402,   739,   436,   108,   307,
     150,     0,     0,   254,   247,     0,     0,     0,   259,   261,
       0,     0,   266,   273,   274,   272,     0,     0,   225,     0,
       0,     0,     0,   689,     0,   454,   609,     0,   204,     0,
     201,     0,    56,   119,     0,   640,     0,     0,     0,    88,
     237,    45,     0,   238,   239,     0,     0,   253,   256,   472,
     473,     0,   248,   275,   276,   268,   269,   267,   305,   302,
     229,   227,   306,     0,   455,   608,     0,   395,     0,   206,
     281,     0,   121,     0,   159,   437,     0,   112,     0,   287,
     255,   258,     0,   652,   231,     0,   452,   450,   205,   397,
     157,     0,     0,    89,   244,     0,   286,   303,   185,     0,
     609,   298,   652,   453,     0,   111,     0,     0,   243,   784,
     652,   184,   299,   300,   301,   807,   297,     0,     0,     0,
     242,     0,   183,   298,     0,   784,     0,   241,   282,    56,
     230,   807,     0,   187,     0,    56,     0,     0,   188,     0,
     232,     0,   283,     0,   191,     0,   190,   107,   192,     0,
     189,     0,   194,   193
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   720,   507,   172,   257,   258,
     118,   119,   120,   121,   122,   123,   301,   526,   527,   426,
     225,  1209,   432,  1141,  1427,   688,   254,   468,  1389,   860,
     987,  1443,   317,   173,   528,   749,   904,  1031,   529,   544,
     767,   491,   765,   530,   512,   766,   319,   273,   290,   129,
     751,   723,   706,   869,  1157,   951,   813,  1343,  1212,   640,
     819,   431,   648,   821,  1064,   635,   804,   807,   942,  1449,
    1450,   896,   897,   538,   539,   262,   263,   267,   992,  1093,
    1175,  1321,  1433,  1452,  1351,  1393,  1394,  1395,  1163,  1164,
    1165,  1352,  1358,  1402,  1168,  1169,  1173,  1314,  1315,  1316,
    1334,  1480,  1094,  1095,   174,   131,  1465,  1466,  1319,  1097,
     132,   219,   427,   428,   133,   134,   135,   136,   137,   138,
     139,   140,   748,   903,   516,   517,   973,   518,   974,   141,
     142,   143,   667,   144,   145,   251,   146,   252,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   678,   679,   852,
     465,   466,   467,   685,  1191,  1376,   147,   513,  1183,   514,
     882,   728,  1008,  1005,  1307,  1308,   148,   149,   150,   213,
     220,   304,   414,   151,   836,   671,   152,   837,   408,   746,
     838,   791,   923,   925,   926,   927,   793,  1043,  1044,   794,
     616,   399,   182,   183,   153,   899,   382,   383,   739,   154,
     214,   176,   156,   157,   158,   159,   160,   161,   162,   574,
     163,   216,   217,   494,   205,   206,   577,   578,   978,   979,
     282,   283,   714,   164,   484,   165,   521,   166,   244,   274,
     312,   441,   832,  1110,   704,   651,   652,   653,   245,   246,
    1019
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1182
static const yytype_int16 yypact[] =
{
   -1182,   132, -1182, -1182,  3845, 10065, 10065,   -72, 10065, 10065,
   10065, -1182, 10065, 10065, 10065, 10065, 10065, 10065, 10065, 10065,
   10065, 10065, 10065, 10065, 11916, 11916,  7903, 10065, 11965,   -66,
     -55, -1182, -1182, -1182, -1182, -1182, -1182, -1182, 10065, -1182,
     -55,   -51,   -38,    58,   -55,  8061, 12460,  8219, -1182, 11368,
    7549,   -64, 10065, 12396,    16, -1182, -1182, -1182,    27,   223,
      36,   157,   162,   179,   185, -1182, 12460,   195,   234, -1182,
   -1182, -1182, -1182, -1182,   276, 11439, -1182, -1182, 12460,  8377,
   -1182, -1182, -1182, -1182, -1182, -1182, 12460, -1182,   230,   238,
   12460, 12460, -1182, 10065, -1182, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, 10065,
   -1182, -1182,    23,   291,   302,   302, -1182,   391,   301,   -27,
   -1182,   246, -1182,    53, -1182,   425, -1182, -1182, -1182, 12444,
     372, -1182, -1182,   271,   294,   309,   322,   331,   348, 10895,
   -1182, -1182,   460, -1182,   485,   487,   349, -1182,   112,   357,
     417, -1182, -1182,   540,   110,  1162,   113,   370,   117,   122,
     371,   109, -1182,   121, -1182,   517, -1182, -1182, -1182,   441,
     395,   446, -1182,   425,   372, 12980,  1290, 12980, 10065, 12980,
   12980,  3831,   545, 12460, -1182, -1182,   537, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, 11669,   430, -1182,
     453,   480,   480, 11916, 12640,   407,   596, -1182,   441, 11669,
     430,   465,   467,   429,   124, -1182,   498,   113,  8535, -1182,
   -1182, 10065,  6253,    57, 12980,  7195, -1182, 10065, 10065, 12460,
   -1182, -1182, 10936,   431, -1182, 10977, 11368, 11368,   459, -1182,
     434,  1623,   611, -1182,   612, -1182, 12460,   553, -1182,   438,
   11018,   440,   412, -1182,    25, 11060, 12460,    59, -1182,    49,
   -1182, 11769,    60, -1182, -1182, -1182,   614,    62, 11916, 11916,
   10065,   443,   474, -1182, -1182, 11818,  7903,    40,   282, -1182,
   10223, 11916,   314, -1182, 12460, -1182,   -39,   301,   447, 12681,
   -1182, -1182, -1182,   558,   626,   552, 12980,   466, 12980,   469,
     550,  4003, 10065,   321,   448,   399,   321,    20,    29, -1182,
   12460, 11368,   472,  8731, 11368, -1182, -1182, 12204, -1182, -1182,
   -1182, -1182,   425, -1182, -1182, -1182, -1182, -1182, -1182, -1182,
   10065, 10065, 10065,  8927, 10065, 10065, 10065, 10065, 10065, 10065,
   10065, 10065, 10065, 10065, 10065, 10065, 10065, 10065, 10065, 10065,
   10065, 10065, 10065, 10065, 10065, 11965, -1182, 10065, 10065, -1182,
   10065, 11128, 12460, 12460, 12444,   564,   409,  7391, 10065, 10065,
   10065, 10065, 10065, 10065, 10065, 10065, 10065, 10065, 10065, -1182,
   -1182, 11456, -1182,   127, 10065, 10065, -1182,  8731, 10065, 10065,
      23,   128, 11818,   475,   425,  9123,  3060, -1182,   476,   655,
   11669,   478,   -13, 11128,   480,  9319, -1182,  9515, -1182,   482,
      -9, -1182,   135,  8731, -1182, 12009, -1182,   131, -1182, -1182,
   11101, -1182, -1182, 10065, -1182,   584,  6449,   661,   483, 12873,
     657,    33,    89, -1182, -1182, -1182, -1182, -1182, 11368,   594,
     486,   668, -1182, 11532, -1182,   499, -1182, -1182, -1182,   606,
   10065,   607,   608, 10065, 10065, 10065, -1182,   412, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182,   502, -1182, -1182, -1182,   495,
   -1182, -1182,    67, 12396, -1182, 12460, 10065,   480,    16, -1182,
   11532,   613, -1182,   480,    39,    44,   500,   501,  1242,   504,
   12460,   573,   508,   480,    83,   512, 12381, 12460, -1182, -1182,
     637,  2429,   -33, -1182, -1182, -1182,   301, -1182, -1182, -1182,
   -1182, 10065,   579,   542,   272, -1182,   585,   700,   519, 11368,
   11368,   697,    24,   650,   111, -1182, -1182, -1182, -1182, -1182,
   -1182,  3111, -1182, -1182, -1182, -1182,    95, 11916,   524,   712,
   12980,   708, -1182, -1182,   616, 12246,  2469, 13017,  3831, 10065,
   12939, 13039,  2369,  7374,  3452,  7439,  7597,  7597,  7597,  7597,
     954,   954,   954,   954,   509,   509,   479,   479,   479,   537,
     537,   537, -1182, 12980,   529,   534, 12736,   538,   725, 10065,
     -40,   547,   128, -1182, -1182, -1182,   425, 11720, 10065, -1182,
   -1182,  3831, -1182,  3831,  3831,  3831,  3831,  3831,  3831,  3831,
    3831,  3831,  3831,  3831, 10065,   -40,   548,   543,  3433,   554,
     546,  3557,    86,   549, -1182, 11621, -1182, 12460, -1182,   466,
      24,   430, 11916, 12980, 11916, 12777,   125,   136, -1182,   556,
   10065, -1182, -1182, -1182,  6057,    76, 12980,   -55, -1182, -1182,
   -1182, 10065, 11384, 11532, 12460,  6645,   557,   559, -1182,   123,
     621, -1182,   739,   562,  3376, 11368, 11532, 11532, 11532,   560,
      32,   615,   565,   567,   190, -1182,   618, -1182,   561, -1182,
   -1182, -1182, 10065,   581, 12980,   582,   750, 11142,   758, -1182,
   12980,  3600, -1182,   502,   694, -1182,  4161, 12340,   576, 12460,
   -1182, -1182, 10561, -1182, -1182,   756, 11916,   580, -1182, -1182,
   -1182, -1182, -1182,   680,   133, 12340,   587, 11818, 11867,   759,
   -1182, -1182, -1182, -1182,   578, -1182, 10065, -1182, -1182,  3172,
   -1182, 12980, 12340,   588, -1182, -1182, -1182, -1182,   768, 10065,
     558, -1182, -1182,   592, -1182, 11368, 12056, -1182,   143, -1182,
   -1182, 11368, -1182,   480, -1182,  9711, -1182, 11532,    30,   597,
   12340,   579, -1182, -1182, 12896, 10065, -1182, -1182, 10065, -1182,
   10065, -1182, 10602,   598,  8731,   573,   579,   616, 12460, 11965,
     480, 10643,   610,  8731, -1182, -1182,   144, -1182, -1182,   772,
   12103, 12103, 11621, -1182, -1182, -1182,   617,    43,   619,   620,
   -1182, -1182, -1182,   792,   622,   476,   480,   480,  9907, -1182,
     145, -1182, -1182, 10684,   296,   -55,  7195, -1182,   624,  4319,
     627, 11916,   628,   679,   480, -1182,   790, -1182, -1182, -1182,
   -1182,   397, -1182,   239, 11368, -1182, 11368,   594, -1182, -1182,
   -1182,   799,   623,   630, -1182, -1182,   688,   629,   803, 11532,
     686, 12460,   558, 11532, 12460, 11532, 12980, 10065, 10065, 10065,
   -1182, -1182, -1182, 10065, 10065, -1182,   412, -1182,   751, -1182,
   -1182, -1182, -1182, 11532,   480, -1182, 11368, 12460, -1182,   818,
   -1182, -1182,    87,   638,   480,  7707, -1182,  1011, -1182,  3648,
     818, -1182,   178,   196, 12980,   706, -1182,   640, 10065,   -40,
     643, -1182, 11916, 12980, -1182, -1182,   639,   830, -1182, 11368,
      30, -1182,   651,    30,   653, 12896, 12980, 12832,   656,  8731,
     654,   658,   659,   579, -1182,   429,   663,  8731,   664, 10065,
   -1182, -1182, -1182, -1182, -1182,   724,   652,   839, 11621,   716,
   -1182,   558, 11621, 11621, -1182, -1182, -1182, 11916, 12980, -1182,
     -55,   829,   789,  7195, -1182, -1182,   669, 10065,   480, 11818,
   11384,   671, 11532,  4477,   410,   673, 10065,    31,   251, -1182,
     696, -1182, -1182, 11299,   836, -1182, 11532, -1182, 11532, -1182,
     675, -1182,   744,   860,   681,   683, -1182,   747,   677,   866,
   12980, 11225, 12980, -1182, 12980, -1182,   691, -1182, -1182, -1182,
     786, 12340,   685, -1182, 11818, -1182, -1182,  3831,   693, -1182,
    1417, -1182,   280, 10065, -1182, -1182, -1182, 10065, -1182, 10065,
   -1182, 10728,   699,  8731,   480,   864,    38, -1182, -1182,   102,
     701, -1182,   702,    30, 10065,  8731,   703, -1182, -1182, -1182,
     704,   705, -1182,  8731,   709, -1182, 11621, -1182, 11621, -1182,
     711, -1182,   773,   713,   890,   714, -1182,   480,   880, -1182,
     717, -1182, -1182,   720,    92, -1182, -1182, -1182,   718,   721,
   -1182, 10853, -1182, -1182, -1182, -1182, -1182, -1182, 11368, -1182,
     785, -1182, 11532,   558, -1182, -1182, -1182, 11532, -1182, 11532,
   -1182, 10065,   726,  4635, 11368, -1182, 11368, 12340, -1182, 12325,
     761, 12188, -1182, -1182, -1182,   564, 11248,    65,   409,    98,
   -1182, -1182,   774, 10769, 10810, 12980,   730,  8731,   731, 11368,
     822, -1182, 11368,   849,   914,   864,  1508,   864,   741, 12980,
     742, -1182,  1724,  1911, -1182,    30,   743, -1182, -1182,   812,
   -1182, 11621, -1182,   558, -1182, -1182, -1182,  6057, -1182, -1182,
   -1182,  6841, -1182, -1182, -1182,  6057,   745, 11532, -1182,   814,
   -1182,   815,  3160, -1182, -1182, -1182,   927,    48, -1182, -1182,
   -1182,    66,   749,    68, -1182, 10381, -1182, -1182,    69, -1182,
   -1182, 12146, -1182,   753, -1182,   872,   425, -1182, 11368, -1182,
     564, -1182, -1182, -1182, -1182, -1182,  8731,   762, -1182, -1182,
     765,   763,   325,   935, 11532,   187, -1182,   822,   864, -1182,
   -1182, -1182,  2074,   767, -1182, 11621, -1182,   842,  6057,  7037,
   -1182, -1182, -1182,  6057, -1182, -1182, 11532, 11532, 10065,  4793,
   11532, 12340, -1182, -1182,   846, 12325, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182,   103, -1182,   761,
   -1182, -1182, -1182, -1182, -1182,    82,   341, -1182,   945,    72,
   12460,   872,   948,   425,   775, -1182,   330, -1182,   893,   956,
   11532, -1182, -1182, -1182, -1182,   784,   187, -1182,   864, -1182,
   11621, -1182, -1182, -1182,  4951, -1182, -1182, 11183, -1182, -1182,
   -1182,  2742,    50, -1182, -1182, 11532, 10381, 10381,   916, -1182,
   12146, 12146,   442, -1182, -1182, -1182, 11532,   904, -1182,   791,
      73, 11532, 12460, -1182,   905, -1182,   973,  5109,   970, 11532,
   -1182,  5267, -1182, -1182,   187, -1182,  5425,   793,   912,   884,
   -1182,   915,   867, -1182, -1182,   918,   846, -1182, -1182, -1182,
   -1182,   853, -1182,   980, -1182, -1182, -1182, -1182, -1182,   997,
   -1182, -1182, -1182,   823, -1182,   336,   824, -1182, 11532, -1182,
   -1182,  5583, -1182,   825, -1182, -1182,   826,   854, 12460,   409,
   -1182, -1182, 11532,    55, -1182,   946, -1182, -1182, -1182, -1182,
   -1182, 12340,   576, -1182,   868, 12460,   388, -1182, -1182,   835,
    1018,   436,    55, -1182,   955, -1182, 12340,   838, -1182,   864,
     141, -1182, -1182, -1182, -1182, 11368, -1182,   837,   840,    75,
   -1182,   209, -1182,   436,   365,   864,   841, -1182, -1182, -1182,
   -1182, 11368,   958,  1022,   209, -1182,  5741,   400,  1024, 11532,
   -1182,  5899, -1182,   986,  1048, 11532, -1182, -1182,  1049, 11532,
   -1182, 11532, -1182, -1182
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1182, -1182, -1182,  -468, -1182, -1182, -1182,    -4, -1182,   591,
     -15,   889,  1413, -1182,  1469, -1182,  -371, -1182,     8, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,  -376,
   -1182, -1182,  -158,    26,     0, -1182, -1182, -1182,     3, -1182,
   -1182, -1182, -1182,     4, -1182, -1182,   710,   707,   715,   917,
     298,  -682,   303,   345,  -384, -1182,   126, -1182, -1182, -1182,
   -1182, -1182, -1182,  -527,    12, -1182, -1182, -1182, -1182,  -375,
   -1182,  -860, -1182,  -309, -1182, -1182,   602, -1182,  -830, -1182,
   -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182, -1182,  -150,
   -1182, -1182, -1182, -1182, -1182,  -228, -1182,    -6,  -886, -1182,
   -1181,  -401, -1182,  -152,    71,  -128,  -389, -1182,  -233, -1182,
     -61,   -17,  1051,  -605,  -348, -1182, -1182,   -45, -1182, -1182,
    2661,   -56, -1182, -1182,  -676, -1182, -1182, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182,   723, -1182, -1182,   235, -1182,
     642, -1182, -1182, -1182, -1182, -1182, -1182, -1182,   241, -1182,
     644, -1182, -1182,   413,  -102, -1182,   215, -1182, -1182, -1182,
   -1182, -1182, -1182, -1182, -1182,  -849, -1182,  1203,    -7,  -322,
   -1182, -1182,   188,  1370,   719, -1182, -1182,  -725,  -377,  -888,
   -1182, -1182,   319,  -599,  -802, -1182, -1182, -1182, -1182, -1182,
     307, -1182, -1182, -1182,  -611,  -984,  -172,  -157,  -125, -1182,
   -1182,    10, -1182, -1182, -1182, -1182,    -8,   -91, -1182,  -234,
   -1182, -1182, -1182,  -366,   828, -1182, -1182, -1182, -1182, -1182,
     462,   228, -1182, -1182,   843, -1182, -1182, -1182,  -311,   -80,
    -155,  -283, -1182, -1051, -1182,   278, -1182, -1182, -1182,  -212,
   -1075
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -778
static const yytype_int16 yytable[] =
{
     117,   365,   323,   541,   126,   249,   291,   127,   128,  1017,
     294,   295,   124,   222,   155,   393,   792,   200,   200,   589,
     215,   212,   618,   226,   436,   437,   612,   230,   536,   442,
     125,   386,  1112,   572,   201,   202,   810,  1192,   259,   719,
    1020,   644,   233,  1022,   416,   242,   323,   696,   297,   320,
     634,  1000,   696,   299,   885,   411,  1221,  1062,  1396,   417,
     286,   314,   272,   287,  1195,   423,  1197,   473,   478,   902,
     481,    11,   391,  1178,  -249,   130,  1225,  1309,   609,    11,
    1367,  1367,   272,  1221,   912,   384,   272,   272,   381,   260,
    1360,   708,   418,   277,   708,   708,    11,   686,   503,   442,
     708,   496,   277,   741,   629,   310,   708,   503,   381,   266,
     840,  1361,   381,  -777,   970,   646,  1355,   178,   975,   300,
     823,   929,   311,   218,   575,   272,  1040,   469,   253,  1356,
    1045,   824,     3,  1113,   221,   311,   401,   898,   227,   475,
     731,   867,   805,   806,   264,   736,  1357,  1336,   409,  -777,
     607,   228,   508,   509,   610,  1383,   322,   689,   280,   281,
     718,   497,  -777,  1118,   545,  -777,   972,   280,   281,   279,
     388,  -676,  -680,  -673,   384,  1114,  -558,   524,  -674,   398,
     930,   627,    11,  -675,   470,  -711,   384,   895,  -677,   388,
     200,  1116,  -712,   310,    35,  1111,   200,  -714,  1122,  1123,
     394,   761,   200,  1422,  -678,  -679,  -713,   402,    35,   486,
     261,   310,  1448,   404,   487,   302,   385,   533,   117,   410,
    -198,   117,  1063,   645,   366,   430,   649,   534,  -608,   697,
     422,  1030,   155,   425,   698,   155,   613,   543,   323,  1222,
    1223,   472,   444,  1397,   315,  -186,   415,   229,   424,  -559,
     474,   479,   879,   482,   200,  1042,  1179,  -249,   742,  1226,
    1310,   200,   200,  1368,  1411,  1203,  1477,   809,   200,   506,
     582,   477,  1362,   709,   200,   828,   779,   993,   483,   483,
     488,   647,  1140,   291,   320,   493,   825,  1384,  1181,   898,
     582,   502,   898,   954,  1202,   958,   868,   117,  1472,  -682,
    -683,   389,  -676,  -680,  -673,   385,   535,   733,   734,  -674,
     242,   155,   582,   272,  -675,   390,  -711,   385,   725,  -677,
     389,   582,   590,  -712,   582,  1001,  1102,   125,  -714,   112,
    -777,  -608,   956,   957,  1046,  -678,  -679,  -713,  1002,   619,
     265,   872,  1053,  1006,   956,   957,   268,   215,   212,   277,
     737,   269,   311,   581,   278,   277,  1328,   580,   272,   272,
     272,  1374,   940,   941,   277,   738,  1363,  1435,   270,   303,
    1003,   831,   130,   606,   271,   277,   292,   605,  1332,  1333,
     306,  1098,  -777,  1364,   275,   200,  1365,   277,  1007,  1098,
    1474,   586,   503,   200,   277,   581,  1482,  1149,  1329,   621,
    1478,  1479,   493,  1375,   628,   898,  1487,   632,  1471,  1436,
     402,   631,   898,   279,   280,   281,   934,   309,   726,   498,
     280,   281,   117,   276,  1484,   763,   292,   293,   768,   280,
     281,  1493,   959,   727,   639,   313,   155,  1128,  1483,  1129,
     280,   281,   442,   833,  1065,   310,   953,   915,   737,  1451,
     772,   504,   280,   281,   799,   910,   316,  1207,   259,   280,
     281,   969,   324,   738,   918,   753,   763,  1405,  1451,   800,
      48,   691,   277,  1494,  1403,  1404,  1473,   503,    55,    56,
      57,   167,   168,   321,  1406,   325,   703,  1407,  1416,   955,
     956,   957,   713,   715,    55,    56,    57,   167,   168,   321,
     326,   801,  1059,   956,   957,  1098,   499,  1399,  1400,   411,
     505,  1098,  1098,   327,   898,    55,    56,    57,   167,   168,
     321,  -415,   328,   887,   352,   353,   354,   995,   355,   891,
     200,   499,  1206,   505,   499,   505,   505,   280,   281,   329,
     359,   272,  1462,  1463,  1464,    94,   357,   743,   358,   360,
    1039,   349,   350,   351,   352,   353,   354,   361,   355,   387,
    -681,    94,  1461,   395,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   305,   307,   308,  -416,  1458,
     200,  -558,    94,  1054,   392,   397,   355,   449,   450,   451,
     284,  1098,   311,   403,   452,   453,  1074,   770,   454,   455,
    1026,   381,  1080,   406,   407,  -557,  1339,   412,  1034,   379,
     380,   790,   960,   795,   961,   200,  1083,   200,   413,   415,
     808,   438,   434,   439,  -772,   443,   445,   480,  1099,   446,
     117,   448,   796,   489,   797,   200,   490,   515,   510,   519,
     816,   117,   520,   532,   155,   582,    55,    56,    57,    58,
      59,   321,   814,   818,   989,   155,   522,    65,   362,   523,
     125,   -51,    48,   617,   542,   615,   637,  1134,   620,   423,
     643,   381,   626,   650,   641,   654,   655,   672,  1137,   673,
     675,   676,   117,   684,   687,   861,   695,  1018,   914,   200,
     699,   700,   705,  1145,   363,   702,   155,   707,   716,   722,
     200,   200,   710,   724,  1108,   130,   864,   729,   730,   732,
     735,  -417,   125,    94,   744,   117,  1120,   493,   874,   126,
     745,   747,   127,   128,  1126,   756,    11,   124,   890,   155,
     757,   759,   889,   760,  1012,   750,   764,   773,   752,   774,
     524,  1385,   777,   826,   776,   125,   802,   827,   820,   839,
     822,   891,   829,   845,   842,   841,   843,   130,   844,   847,
     848,   215,   212,   849,   272,  1208,   853,   856,   859,   863,
     866,   865,   875,  1213,   876,  1156,   922,   922,   790,   871,
     881,   883,  1219,   886,  1086,   919,   900,   909,   943,  1087,
     130,    55,    56,    57,   167,   168,   321,  1088,  1187,   917,
     933,   950,   117,   952,   200,   117,   928,   963,   931,   932,
     966,   968,   935,   964,   944,   945,   155,   949,   947,   155,
     965,   948,   898,   498,   986,   967,   991,   994,  1009,  1015,
     996,  1010,  1013,  1089,  1090,   125,  1091,   971,  1016,  1344,
     976,   898,  1023,  1021,  1027,  1025,  1036,  1038,  1037,   898,
    1028,  1029,  1033,  1041,  1035,  1049,  1146,  1050,    94,  1066,
    1052,  1056,  1068,   990,  1060,  1071,  1072,  1322,  1073,  1077,
    1078,  1075,  1155,  1076,  1079,   117,  1084,  1324,  1092,   126,
     130,  1082,   127,   128,  1177,   200,  1100,   124,  1107,   155,
    1109,  1115,  1117,  1121,  1125,  1131,  1124,  1189,  1133,  1127,
    1018,  1130,  1014,  1132,  1135,   125,  1136,  1147,  1138,  1142,
    1139,  1167,  1143,   198,   198,    31,    32,   210,  1153,  1186,
    1182,  1188,  1193,  1048,   790,    37,  1190,  1194,   790,   790,
     200,  1198,  1199,  1204,  1205,  1214,  1216,  1217,   210,   117,
    1220,  1224,   200,   200,  1317,  1318,  1180,  1047,  1330,   117,
     130,  1051,  1325,   155,  1326,  1327,  1377,  1338,  1366,   493,
     814,  1371,  1381,   155,  1340,  1373,  1378,  1401,   323,  1379,
    1386,    69,    70,    71,    72,    73,  1382,  1409,  1414,   125,
    1410,  1415,   660,  1418,  1424,  1425,  1426,   200,    76,    77,
    -778,  -778,  -778,  -778,   347,   348,   349,   350,   351,   352,
     353,   354,    87,   355,   493,  -245,  1431,  1428,  1429,  1361,
    1432,  1421,  1434,  1442,  1437,  1441,  1440,    92,  1320,  1453,
     330,   331,   332,  1456,   130,  1459,  1460,  1475,  1468,  1470,
    1476,  1488,   790,  1485,   790,  1489,   333,  1495,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,  1498,
     355,  1499,  1501,  1096,   690,   913,  1455,   880,   911,   583,
     364,  1096,  1469,  1144,   585,  1354,  1055,  1467,   584,   117,
     693,  1359,   242,  1490,  1481,  1174,   198,  1172,  1370,   223,
     592,   985,   198,   155,   983,  1335,   855,  1004,   198,   682,
     924,   683,   936,  1032,   495,   962,     0,     0,  1486,   125,
       0,     0,   485,     0,  1491,     0,     0,     0,     0,     0,
       0,  1176,     0,     0,     0,   210,   210,   790,     0,     0,
     210,     0,     0,   117,     0,     0,     0,   117,     0,     0,
       0,   117,     0,     0,     0,     0,     0,   155,     0,  1211,
     198,   155,     0,     0,   130,   155,     0,   198,   198,     0,
       0,  1306,   670,   125,   198,  1372,     0,  1313,     0,   366,
     198,   125,     0,     0,   242,   367,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,  1096,     0,     0,
       0,     0,     0,  1096,  1096,     0,     0,     0,     0,   694,
     210,   790,     0,   210,   117,   117,  1323,   998,   130,   117,
       0,     0,     0,     0,     0,   117,   130,  1342,   155,   155,
       0,   379,   380,   155,     0,     0,     0,   199,   199,   155,
       0,   211,     0,     0,   125,     0,     0,     0,     0,   125,
    1369,     0,     0,     0,   210,   125,     0,     0,     0,     0,
       0,     0,     0,  1018,     0,   395,   368,   369,   370,   371,
     372,   373,   374,   375,   376,   377,   378,     0,     0,  1018,
       0,     0,     0,  1096,     0,     0,     0,  1445,     0,   130,
       0,   198,     0,   381,   130,     0,     0,     0,     0,   198,
     130,     0,  1413,     0,     0,     0,     0,     0,     0,     0,
       0,   379,   380,   395,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,     0,   272,     0,   323,     0,
       0,     0,     0,     0,     0,     0,     0,   210,     0,     0,
       0,     0,   664,     0,     0,     0,   790,     0,     0,     0,
     117,     0,     0,     0,     0,     0,     0,  1391,     0,   379,
     380,     0,  1306,  1306,   155,     0,  1313,  1313,     0,     0,
       0,     0,   815,   381,     0,     0,     0,     0,   272,   664,
     125,     0,     0,   117,     0,   834,   835,   117,     0,     0,
       0,     0,   117,     0,     0,     0,     0,   155,     0,     0,
       0,   155,     0,     0,     0,     0,   155,     0,     0,     0,
       0,     0,     0,   125,     0,     0,   199,   125,   210,   210,
       0,   381,   125,     0,     0,   130,     0,   117,     0,     0,
       0,     0,     0,     0,  1444,     0,   198,     0,     0,     0,
       0,   155,   701,     0,     0,     0,     0,     0,     0,     0,
       0,  1457,     0,     0,     0,     0,     0,   125,   130,     0,
       0,     0,   130,     0,     0,     0,     0,   130,    11,     0,
       0,     0,     0,     0,   199,     0,   894,     0,     0,     0,
       0,   199,   199,     0,     0,     0,   198,     0,   199,     0,
       0,     0,   117,     0,   199,     0,     0,   117,     0,     0,
       0,     0,   130,     0,     0,     0,   155,     0,     0,     0,
    1446,   155,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   198,   125,   198,     0,     0,  1086,   125,   243,     0,
       0,  1087,     0,    55,    56,    57,   167,   168,   321,  1088,
       0,   198,   664,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   210,   210,   664,   664,   664,     0,    11,
       0,     0,     0,     0,     0,     0,     0,   130,   211,     0,
       0,     0,   130,     0,   977,  1089,  1090,     0,  1091,     0,
       0,     0,     0,     0,     0,     0,   210,     0,     0,     0,
       0,     0,   988,     0,     0,   198,     0,     0,     0,     0,
      94,     0,     0,     0,   210,   199,   198,   198,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1086,     0,     0,
    1101,   210,  1087,     0,    55,    56,    57,   167,   168,   321,
    1088,     0,     0,     0,   210,     0,     0,     0,     0,     0,
     210,     0,     0,     0,     0,     0,   664,     0,     0,   210,
       0,     0,     0,     0,     0,     0,   668,     0,   236,     0,
       0,     0,     0,     0,     0,     0,  1089,  1090,   210,  1091,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1057,     0,     0,   237,     0,     0,     0,     0,     0,
       0,    94,     0,   668,     0,  1069,     0,  1070,     0,     0,
       0,     0,     0,     0,    33,     0,     0,     0,     0,     0,
     198,  1196,     0,     0,     0,   243,   243,     0,     0,     0,
     243,     0,     0,   210,     0,   210,     0,     0,     0,     0,
       0,   440,     0,     0,     0,     0,     0,     0,   664,     0,
       0,     0,   664,     0,   664,     0,     0,     0,     0,     0,
     199,     0,     0,     0,     0,     0,     0,   238,     0,     0,
       0,     0,   664,     0,     0,   210,     0,     0,     0,     0,
       0,     0,     0,     0,   171,    11,     0,    78,     0,   239,
       0,    81,    82,     0,    83,    84,    85,     0,     0,     0,
     243,   198,     0,   243,     0,     0,     0,     0,   210,   240,
     199,  1148,     0,     0,     0,     0,  1150,    95,  1151,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   241,   669,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1086,     0,   199,   198,   199,  1087,     0,
      55,    56,    57,   167,   168,   321,  1088,     0,   198,   198,
       0,   664,     0,     0,     0,   199,   668,     0,     0,     0,
     669,     0,   210,     0,     0,   664,   665,   664,     0,   668,
     668,   668,     0,     0,     0,     0,  1215,     0,     0,     0,
       0,     0,  1089,  1090,     0,  1091,     0,     0,     0,     0,
     210,     0,     0,   198,     0,     0,     0,     0,     0,     0,
     858,     0,     0,   665,     0,     0,     0,    94,     0,   199,
       0,     0,     0,     0,     0,     0,     0,   243,   870,     0,
     199,   199,   666,  1331,     0,     0,     0,  1200,     0,     0,
       0,     0,     0,     0,     0,   870,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1345,  1346,     0,     0,  1349,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   666,
     668,     0,    11,   901,     0,     0,     0,   210,     0,     0,
       0,   664,     0,     0,     0,     0,   664,     0,   664,     0,
       0,     0,   211,   210,     0,   210,   210,     0,   210,     0,
       0,     0,     0,     0,     0,   210,     0,     0,   243,   243,
       0,     0,     0,     0,     0,     0,     0,     0,   210,     0,
       0,   210,     0,     0,     0,     0,     0,     0,     0,     0,
    1086,     0,     0,   669,   199,  1087,     0,    55,    56,    57,
     167,   168,   321,  1088,     0,     0,   669,   669,   669,     0,
       0,     0,     0,     0,     0,     0,   664,     0,     0,     0,
       0,     0,   668,     0,     0,     0,   668,     0,   668,  1380,
       0,     0,     0,     0,     0,     0,   665,     0,     0,  1089,
    1090,     0,  1091,     0,     0,     0,   668,   210,     0,   665,
     665,   665,     0,     0,  1398,     0,     0,     0,     0,     0,
       0,     0,     0,   664,    94,  1408,     0,     0,     0,     0,
    1412,     0,     0,     0,     0,   199,     0,     0,  1419,     0,
       0,     0,     0,     0,  1201,   664,   664,     0,     0,   664,
     210,     0,   666,     0,   210,    11,     0,   669,     0,     0,
       0,     0,     0,   243,   243,   666,   666,   666,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1438,     0,     0,
     199,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1447,   199,   199,     0,   668,     0,     0,     0,     0,
     665,     0,     0,     0,     0,     0,     0,     0,     0,   668,
       0,   668,     0,  1086,     0,     0,     0,     0,  1087,     0,
      55,    56,    57,   167,   168,   321,  1088,     0,     0,     0,
       0,     0,     0,     0,  1085,     0,     0,   199,     0,     0,
       0,     0,     0,     0,   243,     0,     0,     0,  1496,   669,
     243,     0,     0,   669,  1500,   669,   666,     0,  1502,   664,
    1503,     0,  1089,  1090,     0,  1091,     0,     0,     0,     0,
       0,     0,     0,   669,     0,     0,     0,     0,     0,     0,
     210,     0,     0,     0,   664,     0,     0,    94,     0,     0,
       0,     0,   665,     0,     0,   664,   665,     0,   665,     0,
     664,     0,     0,     0,     0,     0,     0,  1337,   664,     0,
       0,     0,     0,     0,     0,   668,   665,     0,     0,     0,
     668,     0,   668,     0,     0,     0,     0,     0,     0,     0,
    1158,     0,  1166,   243,     0,   243,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   664,   666,     0,
       0,     0,   666,     0,   666,     0,     0,     0,     0,     0,
       0,   664,   669,     0,     0,     0,     0,     0,     0,     0,
     210,     0,   666,     0,     0,   243,   669,     0,   669,     0,
       0,     0,     0,     0,     0,   210,     0,     0,     0,     0,
     668,     0,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   665,     0,     0,   243,     0,
     210,     0,     0,     0,     0,     0,     0,     0,   664,   665,
       0,   665,     0,     0,   664,     0,     0,     0,   664,     0,
     664,     0,     0,     0,     0,     0,     0,   668,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,     0,   355,   668,
     668,   666,     0,   668,  1350,     0,     0,     0,  1166,     0,
       0,     0,   243,     0,     0,   666,     0,   666,   330,   331,
     332,     0,   669,     0,     0,     0,     0,   669,     0,   669,
       0,     0,     0,     0,   333,     0,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,     0,   355,   331,
     332,     0,     0,     0,     0,   665,     0,     0,     0,     0,
     665,     0,   665,     0,   333,     0,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   669,   355,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   668,     0,     0,     0,   243,     0,     0,
       0,   666,     0,     0,     0,     0,   666,     0,   666,     0,
       0,     0,     0,   243,     0,   243,     0,     0,   668,     0,
     665,     0,     0,     0,   669,   243,     0,     0,     0,   668,
       0,     0,     0,     0,   668,     0,     0,     0,   243,     0,
       0,   243,   668,     0,     0,     0,   669,   669,     0,     0,
     669,     0,     0,     0,  1353,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   665,     0,     0,
       0,     0,     0,     0,     0,     0,   666,     0,     0,     0,
       0,   668,   717,     0,     0,     0,     0,     0,     0,   665,
     665,     0,     0,   665,     0,   668,     0,     0,     0,     0,
       0,     0,     0,     0,  1454,     0,     0,   243,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1158,
       0,     0,     0,   666,     0,     0,   175,   177,     0,   179,
     180,   181,     0,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   195,   666,   666,   204,   207,   666,
       0,     0,   668,     0,     0,     0,     0,     0,   668,   224,
     669,     0,   668,     0,   668,     0,   232,     0,   235,     0,
       0,   250,     0,   255,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   669,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   669,     0,     0,     0,
     289,   669,     0,   665,     0,     0,     0,     0,     0,   669,
       0,     0,     0,     0,   296,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1430,     0,   665,     0,
     298,     0,     0,     0,     0,     0,     0,     0,     0,   665,
       0,     0,     0,     0,   665,     0,     0,     0,   669,     0,
       0,     0,   665,     0,     0,     0,     0,     0,     0,   666,
       0,     0,   669,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,     0,     0,     0,     0,     0,
    1392,     0,     0,     0,   666,     0,     0,     0,     0,     0,
       0,   665,     0,     0,     0,   666,     0,     0,     0,   396,
     666,     0,     0,     0,     0,   665,     0,     0,   666,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   669,
       0,     0,     0,     0,     0,   669,     0,     0,     0,   669,
       0,   669,     0,     0,     0,     0,     0,     0,     0,   420,
       0,     0,   420,   171,     0,     0,    78,   666,   224,   429,
      81,    82,     0,    83,    84,    85,     0,     0,     0,     0,
       0,   666,   665,     0,     0,     0,     0,     0,   665,     0,
       0,     0,   665,     0,   665,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   298,     0,     0,   243,  1390,     0,   204,     0,     0,
       0,   501,     0,     0,     0,     0,     0,     0,     0,     0,
     243,     0,     0,     0,     0,     0,     0,     0,   666,     0,
       0,     0,     0,   531,   666,     0,     0,     0,   666,     0,
     666,     0,     0,     0,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   546,   547,   548,   550,   551,   552,   553,   554,   555,
     556,   557,   558,   559,   560,   561,   562,   563,   564,   565,
     566,   567,   568,   569,   570,   571,     0,     0,   573,   573,
       0,   576,     0,     0,     0,     0,     0,     0,   591,   593,
     594,   595,   596,   597,   598,   599,   600,   601,   602,   603,
       0,     0,     0,     0,     0,   573,   608,     0,   540,   573,
     611,     0,     0,     0,     0,     0,   591,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   623,     0,   625,   330,
     331,   332,     0,     0,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   636,   333,     0,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,     0,   355,
       0,   674,     0,     0,   677,   680,   681,     0,     0,     0,
     330,   331,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   333,   692,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,     0,     0,     0,     0,     0,     0,     0,     0,   330,
     331,   332,   721,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,   333,     0,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,     0,   355,
     754,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
     762,    31,    32,    33,    34,    35,     0,    36,     0,   289,
     614,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,   771,     0,    45,    46,    47,
      48,    49,    50,    51,     0,    52,    53,    54,    55,    56,
      57,    58,    59,    60,     0,    61,    62,    63,    64,    65,
      66,   803,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,   224,     0,   740,     0,     0,     0,    74,     0,
       0,     0,     0,    75,    76,    77,    78,    79,    80,     0,
      81,    82,     0,    83,    84,    85,    86,     0,    87,     0,
       0,     0,    88,   846,     0,     0,     0,     0,    89,    90,
    1218,    91,     0,    92,    93,    94,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,   110,   111,   878,   112,   113,     0,   114,
     115,     0,     0,     0,     0,     0,     0,   877,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     884,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   236,     0,     0,     0,     0,   893,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   905,     0,     0,   906,
       0,   907,     0,     0,     0,   540,     0,   237,     0,     0,
       0,     0,     0,     0,   540,     0,     0,     0,     0,     0,
       0,     0,   330,   331,   332,     0,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   333,   938,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,     0,   355,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     238,   355,     0,     0,     0,     0,     0,     0,   980,   981,
     982,     0,     0,     0,   677,   984,     0,   171,     0,     0,
      78,     0,   239,     0,    81,    82,     0,    83,    84,    85,
       0,     0,     0,   830,     0,     0,   997,     0,     0,     0,
       0,     0,   240,     0,     0,     0,     0,     0,     0,  1011,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   241,   330,   331,   332,     0,
     540,     0,     0,     0,     0,     0,     0,     0,   540,     0,
     997,     0,   333,     0,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,     0,   355,     0,   224,   330,
     331,   332,     0,     0,     0,     0,     0,  1061,     0,     0,
       0,     0,     0,     0,     0,   333,   775,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,     0,   355,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,  1103,     0,     0,     0,  1104,     0,
    1105,     0,     0,     0,   540,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1119,   540,     0,     0,    11,
      12,    13,     0,     0,   540,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,  1152,    45,    46,    47,    48,    49,    50,    51,
     778,    52,    53,    54,    55,    56,    57,    58,    59,    60,
       0,    61,    62,    63,    64,    65,    66,     0,   540,     0,
       0,    67,    68,    69,    70,    71,    72,    73,     0,     0,
       0,     0,     0,     0,    74,     0,     0,     0,   854,    75,
      76,    77,    78,    79,    80,     0,    81,    82,     0,    83,
      84,    85,    86,     0,    87,     0,     0,     0,    88,     0,
       0,     0,     0,     0,    89,    90,     0,    91,     0,    92,
      93,    94,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,   110,
     111,   999,   112,   113,     0,   114,   115,   540,     5,     6,
       7,     8,     9,     0,     0,     0,   333,    10,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,  1347,
     355,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,    49,    50,    51,     0,    52,    53,
      54,    55,    56,    57,    58,    59,    60,     0,    61,    62,
      63,    64,    65,    66,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,    75,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,    90,     0,    91,    10,    92,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,   110,   111,     0,   112,
     113,     0,   114,   115,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   171,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,   110,   111,   525,   112,   113,     0,
     114,   115,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   171,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,   110,   111,   857,   112,   113,     0,   114,   115,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,   946,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,     0,    65,    66,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     171,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     110,   111,     0,   112,   113,     0,   114,   115,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,  1058,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   171,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,   110,   111,
       0,   112,   113,     0,   114,   115,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,    58,    59,    60,     0,    61,    62,
      63,     0,    65,    66,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   171,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,   110,   111,  1154,   112,
     113,     0,   114,   115,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   171,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,   110,   111,  1348,   112,   113,     0,
     114,   115,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,  1387,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   171,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,   110,   111,     0,   112,   113,     0,   114,   115,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,     0,    65,    66,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     171,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     110,   111,  1417,   112,   113,     0,   114,   115,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   171,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,   110,   111,
    1420,   112,   113,     0,   114,   115,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
    1423,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,    58,    59,    60,     0,    61,    62,
      63,     0,    65,    66,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   171,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,   110,   111,     0,   112,
     113,     0,   114,   115,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   171,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,   110,   111,  1439,   112,   113,     0,
     114,   115,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   171,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,   110,   111,  1492,   112,   113,     0,   114,   115,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,     0,    65,    66,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     171,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     110,   111,  1497,   112,   113,     0,   114,   115,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   171,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,   110,   111,
       0,   112,   113,     0,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   421,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,   167,   168,    60,     0,    61,    62,    63,     0,
       0,     0,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   171,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     0,     0,     0,     0,     0,    89,
       0,     0,     0,     0,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,   110,   111,     0,   112,   113,     0,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   638,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,   167,   168,
      60,     0,    61,    62,    63,     0,     0,     0,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     171,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     110,   111,     0,   112,   113,     0,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   817,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,   167,   168,    60,     0,    61,    62,
      63,     0,     0,     0,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   171,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,   110,   111,     0,   112,
     113,     0,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1210,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     167,   168,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   171,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,   110,   111,     0,   112,   113,     0,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1341,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,   167,   168,    60,     0,
      61,    62,    63,     0,     0,     0,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   171,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,   110,   111,
       0,   112,   113,     0,   114,   115,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,   167,   168,    60,     0,    61,    62,
      63,     0,     0,     0,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   171,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,    93,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,   110,   111,     0,   112,
     113,     0,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,   587,   355,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,     0,   355,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     167,   168,   169,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   170,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   171,    76,    77,    78,   588,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,    94,    95,   247,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,   112,   113,     0,   114,   115,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,  -778,
    -778,  -778,  -778,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,     0,   355,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   167,   168,
     169,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   170,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     171,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,   247,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     248,     0,     0,   112,   113,     0,   114,   115,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   167,   168,   169,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     170,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   171,    76,
      77,    78,   588,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,   112,   113,     0,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   203,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   167,   168,   169,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   170,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   171,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,     0,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,     0,   112,   113,     0,
     114,   115,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     167,   168,   169,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   170,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   171,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,   231,     0,     0,   112,   113,     0,   114,   115,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   167,   168,
     169,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   170,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     171,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     234,     0,     0,   112,   113,     0,   114,   115,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   288,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   167,   168,   169,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     170,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   171,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,   112,   113,     0,   114,   115,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   167,   168,   169,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   170,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   171,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   419,     0,     0,     0,   112,
     113,     0,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   537,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     167,   168,   169,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   170,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   171,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,   112,   113,     0,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   549,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   167,   168,   169,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     170,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   171,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,   112,   113,     0,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   587,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   167,   168,   169,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   170,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   171,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     0,     0,     0,     0,     0,    89,
       0,     0,     0,     0,    92,     0,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,     0,   112,   113,     0,
     114,   115,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     622,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   167,   168,
     169,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   170,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     171,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,     0,   112,   113,     0,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   624,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   167,   168,   169,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   170,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   171,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,     0,   112,
     113,     0,   114,   115,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   892,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     167,   168,   169,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   170,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   171,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,     0,    94,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,   112,   113,     0,   114,   115,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   937,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   167,   168,   169,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     170,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   171,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,     0,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,   112,   113,     0,   114,   115,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   167,   168,   169,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   170,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   171,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,     0,   112,
     113,     0,   114,   115,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,   500,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   167,   168,   169,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   170,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   171,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,  1227,  1228,  1229,  1230,  1231,    89,
    1232,  1233,  1234,  1235,    92,     0,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,     0,     0,     0,   112,   113,     0,
     114,   115,     0,     0,     0,     0,     0,     0,     0,     0,
    1236,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1237,  1238,  1239,  1240,  1241,  1242,  1243,
       0,     0,    33,     0,     0,     0,     0,     0,     0,     0,
       0,  1244,  1245,  1246,  1247,  1248,  1249,  1250,  1251,  1252,
    1253,  1254,  1255,  1256,  1257,  1258,  1259,  1260,  1261,  1262,
    1263,  1264,  1265,  1266,  1267,  1268,  1269,  1270,  1271,  1272,
    1273,  1274,  1275,  1276,  1277,  1278,  1279,  1280,  1281,  1282,
    1283,  1284,     0,     0,  1285,  1286,  1287,  1288,  1289,  1290,
    1291,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1292,  1293,  1294,     0,  1295,     0,     0,    81,
      82,     0,    83,    84,    85,  1296,  1297,  1298,     0,     0,
    1299,     0,     0,     0,     0,     0,     0,  1300,  1301,     0,
    1302,     0,  1303,  1304,  1305,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     330,   331,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   333,     0,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,   330,   331,   332,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   333,     0,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
       0,   355,   330,   331,   332,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   333,     0,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,     0,   355,   330,   331,   332,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   333,
       0,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,     0,   355,     0,     0,     0,   330,   331,   332,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   333,   862,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,     0,   355,   330,   331,
     332,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   333,   908,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,     0,   355,   330,
     331,   332,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   333,   916,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,     0,   355,
       0,     0,   330,   331,   332,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   939,   333,  1062,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,     0,   355,     0,   330,   331,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     333,  1106,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,     0,   355,   330,   331,   332,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   333,  1184,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,     0,   355,   330,   331,   332,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   333,  1185,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,     0,   355,   330,   331,   332,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   333,  1063,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,     0,   355,     0,   330,
     331,   332,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   333,   356,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,     0,   355,
     330,   331,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   333,   433,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,   330,   331,   332,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   333,   435,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
       0,   355,   330,   331,   332,     0,     0,     0,     0,    33,
       0,    35,     0,     0,     0,     0,     0,     0,   333,   447,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,     0,   355,     0,   330,   331,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     333,   471,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   236,   355,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   633,     0,     0,     0,     0,     0,     0,     0,   237,
       0,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,    33,
     579,     0,   112,     0,   236,     0,   850,   851,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -286,     0,     0,     0,
     237,     0,     0,     0,    55,    56,    57,   167,   168,   321,
       0,     0,     0,     0,  1388,     0,     0,     0,     0,     0,
      33,     0,   238,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   171,
       0,     0,    78,   236,   239,     0,    81,    82,     0,    83,
      84,    85,     0,     0,  1081,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   240,   811,     0,     0,     0,   237,
       0,    94,    95,   238,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   241,     0,    33,
     171,     0,     0,    78,     0,   239,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    33,  1067,    35,     0,     0,
       0,     0,     0,     0,     0,   240,     0,     0,     0,     0,
       0,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   241,     0,
       0,     0,   238,     0,     0,   196,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   812,     0,   171,
      33,     0,    78,     0,   239,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,   171,     0,    33,    78,    35,
      80,     0,    81,    82,   240,    83,    84,    85,     0,     0,
       0,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   241,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   197,   656,   657,     0,     0,   112,     0,
       0,     0,     0,   284,     0,     0,     0,    81,    82,     0,
      83,    84,    85,   658,     0,     0,     0,     0,     0,     0,
       0,    31,    32,    33,    81,    82,     0,    83,    84,    85,
       0,    37,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
      95,   285,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,   604,     0,
     112,     0,     0,     0,     0,     0,   659,    69,    70,    71,
      72,    73,     0,   780,   781,     0,     0,     0,   660,     0,
       0,     0,     0,   171,    76,    77,    78,     0,   661,     0,
      81,    82,   782,    83,    84,    85,     0,     0,    87,     0,
     783,   784,    33,     0,     0,     0,     0,     0,   662,     0,
     785,     0,     0,    92,     0,     0,   663,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    27,    28,     0,     0,     0,     0,     0,     0,     0,
      33,     0,    35,     0,     0,   786,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   787,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    81,
      82,     0,    83,    84,    85,     0,     0,     0,     0,     0,
     196,     0,     0,   769,     0,     0,     0,   788,     0,     0,
       0,    33,     0,    35,     0,   789,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     171,     0,     0,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,     0,     0,     0,     0,    88,
       0,   196,     0,     0,     0,     0,     0,     0,     0,     0,
      33,     0,    35,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   400,     0,
       0,   171,     0,   112,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,     0,     0,
     196,     0,     0,     0,     0,     0,     0,     0,     0,    33,
       0,    35,     0,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   197,
     171,     0,     0,    78,   112,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   196,
       0,     0,     0,     0,     0,     0,     0,     0,    33,     0,
      35,   492,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   197,   171,
       0,   476,    78,   112,    80,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   196,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,    35,
     873,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   197,   171,     0,
       0,    78,   112,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,     0,     0,     0,     0,   196,     0,     0,
       0,     0,     0,     0,     0,     0,    33,     0,    35,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   197,   171,     0,     0,
      78,   112,    80,     0,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
      33,     0,    35,     0,     0,     0,     0,     0,     0,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   197,   171,     0,     0,    78,
     112,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,    35,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   209,     0,     0,    81,    82,   112,
      83,    84,    85,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   920,   921,    33,     0,     0,     0,     0,     0,
       0,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,     0,
       0,   630,     0,   112,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,   888,     0,
     112,    81,    82,     0,    83,    84,    85,     0,     0,    33,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,  1311,     0,    81,    82,  1312,    83,    84,    85,
       0,     0,     0,  1170,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,  1171,    81,    82,     0,    83,
      84,    85,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,  1171,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   542,    81,    82,    33,    83,    84,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,     0,     0,     0,     0,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   752,     0,  1159,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1160,
       0,     0,    33,     0,   711,   712,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   171,    33,     0,    78,
       0,  1161,     0,    81,    82,     0,    83,  1162,    85,     0,
       0,   171,     0,     0,    78,     0,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,    95,    33,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,    81,
      82,    33,    83,    84,    85,     0,     0,     0,     0,     0,
     256,     0,     0,     0,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,     0,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,     0,     0,     0,     0,     0,
     318,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   330,
     331,   332,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   333,     0,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,     0,   355,
     330,   331,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   333,     0,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   330,   331,   332,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   333,   405,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,     0,   355,   330,   331,   332,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   333,   511,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,     0,   355,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   330,   331,   332,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   333,   758,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
       0,   355,   330,   331,   332,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   333,   798,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,     0,   355,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,     0,   355,     0,     0,   330,   331,
     332,     0,     0,     0,  1024,     0,     0,     0,     0,     0,
       0,     0,     0,   642,   333,   755,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,     0,   355,   330,
     331,   332,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   333,     0,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   332,   355,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   333,     0,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,     0,   355,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,     0,   355
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1182))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-778))

static const yytype_int16 yycheck[] =
{
       4,   153,   130,   314,     4,    50,    86,     4,     4,   897,
      90,    91,     4,    30,     4,   173,   615,    24,    25,   367,
      28,    28,   399,    40,   236,   237,   392,    44,   311,   241,
       4,   156,  1016,   355,    24,    25,   641,  1112,    53,   507,
     900,     8,    46,   903,   216,    49,   174,     8,   109,   129,
     421,   881,     8,   109,   730,   210,     8,    26,     8,   216,
      75,     8,    66,    78,  1115,     8,  1117,     8,     8,   751,
       8,    41,   163,     8,     8,     4,     8,     8,   387,    41,
       8,     8,    86,     8,   766,    61,    90,    91,   121,    73,
       8,     8,   217,    73,     8,     8,    41,   468,    78,   311,
       8,    61,    73,     8,   413,   144,     8,    78,   121,    73,
      78,    29,   121,   140,   839,    26,    13,   189,   843,   109,
     647,    78,   162,   189,   358,   129,   928,   102,   192,    26,
     932,     8,     0,    31,   189,   162,   197,   748,   189,    90,
     517,     8,    66,    67,   117,   121,    43,  1198,   209,   189,
     384,   189,   191,   192,   388,  1336,   130,    90,   138,   139,
     193,   121,   189,  1023,   322,   192,   842,   138,   139,   137,
      61,    61,    61,    61,    61,    73,   140,   190,    61,   183,
     137,   190,    41,    61,   159,    61,    61,   157,    61,    61,
     197,  1021,    61,   144,    73,   157,   203,    61,  1028,  1029,
     174,   578,   209,  1384,    61,    61,    61,   197,    73,   270,
     194,   144,   157,   203,   270,   192,   192,   197,   222,   209,
     190,   225,   191,   190,   153,   229,   438,   198,   190,   190,
     222,   913,   222,   225,   190,   225,   394,   317,   366,   191,
     192,   256,   246,   193,   191,   190,   121,   189,   191,   140,
     191,   191,   720,   191,   261,   931,   191,   191,   163,   191,
     191,   268,   269,   191,   191,  1125,   191,   638,   275,   284,
     361,   261,   190,   190,   281,   652,   190,   190,   268,   269,
     270,   192,   190,   363,   364,   275,   163,  1338,   190,   900,
     381,   281,   903,   820,  1124,   822,   163,   301,   157,   189,
     189,   192,   192,   192,   192,   192,   310,   519,   520,   192,
     314,   301,   403,   317,   192,   194,   192,   192,    46,   192,
     192,   412,   367,   192,   415,   147,    46,   301,   192,   194,
     140,   190,    93,    94,   933,   192,   192,   192,   160,   400,
     117,   707,   947,   147,    93,    94,   189,   355,   355,    73,
     522,   189,   162,   361,    78,    73,    31,   361,   362,   363,
     364,    31,    66,    67,    73,   522,    25,    31,   189,    78,
     192,   654,   301,   381,   189,    73,   146,   381,   191,   192,
      78,   992,   192,    42,   189,   392,    45,    73,   192,  1000,
    1465,   365,    78,   400,    73,   403,    31,  1073,    73,   403,
     191,   192,   392,    73,   412,  1016,  1481,   415,  1459,    73,
     400,   415,  1023,   137,   138,   139,   793,    26,   146,   137,
     138,   139,   426,   189,  1475,   580,   146,   189,   586,   138,
     139,    31,   193,   161,   426,   189,   426,  1036,    73,  1038,
     138,   139,   654,   655,   193,   144,   817,   769,   620,  1433,
     605,   137,   138,   139,   626,   764,    31,  1133,   473,   138,
     139,   838,   191,   620,   773,   545,   621,    25,  1452,   626,
      98,   475,    73,    73,  1360,  1361,  1460,    78,   106,   107,
     108,   109,   110,   111,    42,   191,   490,    45,  1376,    92,
      93,    94,   496,   497,   106,   107,   108,   109,   110,   111,
     191,   626,    92,    93,    94,  1116,   278,  1356,  1357,   664,
     282,  1122,  1123,   191,  1125,   106,   107,   108,   109,   110,
     111,    61,   191,   735,    45,    46,    47,   875,    49,   741,
     537,   303,  1131,   305,   306,   307,   308,   138,   139,   191,
     191,   545,   106,   107,   108,   173,    61,   537,    61,   192,
     927,    42,    43,    44,    45,    46,    47,   140,    49,   189,
     189,   173,  1450,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   113,   114,   115,    61,   191,
     587,   140,   173,   949,   189,    40,    49,   175,   176,   177,
     144,  1202,   162,   140,   182,   183,   973,   587,   186,   187,
     909,   121,   979,   196,     8,   140,  1205,   140,   917,    59,
      60,   615,   824,   617,   826,   622,   987,   624,   189,   121,
     637,   162,   191,   189,    13,    13,    73,    13,   994,   191,
     634,   191,   622,   190,   624,   642,   162,    79,   191,    13,
     644,   645,    90,   195,   634,   736,   106,   107,   108,   109,
     110,   111,   642,   645,   866,   645,   190,   117,   118,   190,
     634,   189,    98,     8,   189,   189,    82,  1044,   190,     8,
      13,   121,   190,    79,   191,   189,     8,   178,  1049,    73,
      73,    73,   686,   181,   189,   689,    73,   899,   768,   696,
     190,   190,   119,  1064,   154,   191,   686,   189,    61,   120,
     707,   708,   190,   161,  1013,   634,   696,   122,     8,   190,
      13,    61,   686,   173,   190,   719,  1025,   707,   708,   719,
       8,    13,   719,   719,  1033,   196,    41,   719,   736,   719,
     196,   193,   736,     8,   889,   119,   189,   189,   189,   196,
     190,  1340,   196,   122,   190,   719,   190,     8,   191,   189,
     191,   963,   190,   192,   189,   140,   189,   686,   140,   178,
     178,   769,   769,    13,   768,  1136,     8,    73,   192,    13,
      90,   191,    13,  1144,   196,  1086,   780,   781,   782,   192,
     192,    13,  1153,   191,    99,    13,   189,   189,   805,   104,
     719,   106,   107,   108,   109,   110,   111,   112,  1107,   189,
       8,   122,   806,    13,   811,   809,   189,     8,   189,   189,
     122,     8,   190,   190,   806,   191,   806,   189,   191,   809,
     190,   811,  1433,   137,    73,   196,     8,   189,   122,   190,
     875,   191,   189,   148,   149,   809,   151,   841,     8,  1210,
     844,  1452,   189,   192,   190,   189,   122,     8,   196,  1460,
     192,   192,   189,   137,   190,    26,  1068,    68,   173,   163,
     191,   190,    26,   867,   191,   190,   122,  1178,     8,   122,
     193,   190,  1084,   190,     8,   879,    90,  1186,   193,   879,
     809,   190,   879,   879,  1096,   892,   193,   879,   189,   879,
      26,   190,   190,   190,   189,   122,   192,  1109,     8,   190,
    1112,   190,   892,   190,   190,   879,    26,   122,   191,   191,
     190,   150,   191,    24,    25,    69,    70,    28,   192,   189,
     146,   190,    73,   940,   928,    79,   104,    13,   932,   933,
     937,   190,   190,   190,   122,   190,   122,   122,    49,   943,
      13,   192,   949,   950,   191,    73,  1098,   937,    13,   953,
     879,   943,   190,   943,   189,   192,  1327,   190,    13,   949,
     950,    13,  1333,   953,   122,   190,    73,    51,  1096,    13,
    1341,   125,   126,   127,   128,   129,   192,    73,    73,   953,
     189,     8,   136,    13,   191,    73,   102,   994,   142,   143,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,   156,    49,   994,    90,   153,   140,    90,    29,
      13,  1382,   189,   159,   190,   189,   191,   171,  1176,    73,
       9,    10,    11,   155,   953,   190,     8,   190,    73,   191,
     190,    73,  1036,   192,  1038,    13,    25,    13,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    73,
      49,    13,    13,   992,   473,   767,  1442,   722,   765,   362,
     153,  1000,  1456,  1061,   364,  1225,   950,  1452,   363,  1083,
     478,  1309,  1086,  1484,  1473,  1091,   197,  1091,  1321,    38,
     367,   856,   203,  1083,   853,  1197,   683,   882,   209,   457,
     781,   457,   795,   915,   276,   827,    -1,    -1,  1479,  1083,
      -1,    -1,   269,    -1,  1485,    -1,    -1,    -1,    -1,    -1,
      -1,  1095,    -1,    -1,    -1,   236,   237,  1131,    -1,    -1,
     241,    -1,    -1,  1137,    -1,    -1,    -1,  1141,    -1,    -1,
      -1,  1145,    -1,    -1,    -1,    -1,    -1,  1137,    -1,  1141,
     261,  1141,    -1,    -1,  1083,  1145,    -1,   268,   269,    -1,
      -1,  1165,   443,  1137,   275,  1323,    -1,  1171,    -1,  1098,
     281,  1145,    -1,    -1,  1178,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,  1116,    -1,    -1,
      -1,    -1,    -1,  1122,  1123,    -1,    -1,    -1,    -1,   480,
     311,  1205,    -1,   314,  1208,  1209,  1180,   196,  1137,  1213,
      -1,    -1,    -1,    -1,    -1,  1219,  1145,  1209,  1208,  1209,
      -1,    59,    60,  1213,    -1,    -1,    -1,    24,    25,  1219,
      -1,    28,    -1,    -1,  1208,    -1,    -1,    -1,    -1,  1213,
    1320,    -1,    -1,    -1,   355,  1219,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1465,    -1,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,  1481,
      -1,    -1,    -1,  1202,    -1,    -1,    -1,  1429,    -1,  1208,
      -1,   392,    -1,   121,  1213,    -1,    -1,    -1,    -1,   400,
    1219,    -1,  1372,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,  1320,    -1,  1446,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   438,    -1,    -1,
      -1,    -1,   443,    -1,    -1,    -1,  1340,    -1,    -1,    -1,
    1344,    -1,    -1,    -1,    -1,    -1,    -1,  1351,    -1,    59,
      60,    -1,  1356,  1357,  1344,    -1,  1360,  1361,    -1,    -1,
      -1,    -1,   643,   121,    -1,    -1,    -1,    -1,  1372,   480,
    1344,    -1,    -1,  1377,    -1,   656,   657,  1381,    -1,    -1,
      -1,    -1,  1386,    -1,    -1,    -1,    -1,  1377,    -1,    -1,
      -1,  1381,    -1,    -1,    -1,    -1,  1386,    -1,    -1,    -1,
      -1,    -1,    -1,  1377,    -1,    -1,   203,  1381,   519,   520,
      -1,   121,  1386,    -1,    -1,  1344,    -1,  1421,    -1,    -1,
      -1,    -1,    -1,    -1,  1428,    -1,   537,    -1,    -1,    -1,
      -1,  1421,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1445,    -1,    -1,    -1,    -1,    -1,  1421,  1377,    -1,
      -1,    -1,  1381,    -1,    -1,    -1,    -1,  1386,    41,    -1,
      -1,    -1,    -1,    -1,   261,    -1,   747,    -1,    -1,    -1,
      -1,   268,   269,    -1,    -1,    -1,   587,    -1,   275,    -1,
      -1,    -1,  1486,    -1,   281,    -1,    -1,  1491,    -1,    -1,
      -1,    -1,  1421,    -1,    -1,    -1,  1486,    -1,    -1,    -1,
    1429,  1491,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   622,  1486,   624,    -1,    -1,    99,  1491,    49,    -1,
      -1,   104,    -1,   106,   107,   108,   109,   110,   111,   112,
      -1,   642,   643,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   654,   655,   656,   657,   658,    -1,    41,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1486,   355,    -1,
      -1,    -1,  1491,    -1,   845,   148,   149,    -1,   151,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   687,    -1,    -1,    -1,
      -1,    -1,   863,    -1,    -1,   696,    -1,    -1,    -1,    -1,
     173,    -1,    -1,    -1,   705,   392,   707,   708,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    99,    -1,    -1,
     193,   722,   104,    -1,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,   735,    -1,    -1,    -1,    -1,    -1,
     741,    -1,    -1,    -1,    -1,    -1,   747,    -1,    -1,   750,
      -1,    -1,    -1,    -1,    -1,    -1,   443,    -1,    25,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,   769,   151,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   952,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,
      -1,   173,    -1,   480,    -1,   966,    -1,   968,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
     811,   193,    -1,    -1,    -1,   236,   237,    -1,    -1,    -1,
     241,    -1,    -1,   824,    -1,   826,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,   839,    -1,
      -1,    -1,   843,    -1,   845,    -1,    -1,    -1,    -1,    -1,
     537,    -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,
      -1,    -1,   863,    -1,    -1,   866,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,    41,    -1,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,
     311,   892,    -1,   314,    -1,    -1,    -1,    -1,   899,   166,
     587,  1072,    -1,    -1,    -1,    -1,  1077,   174,  1079,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   443,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,   622,   937,   624,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,   949,   950,
      -1,   952,    -1,    -1,    -1,   642,   643,    -1,    -1,    -1,
     480,    -1,   963,    -1,    -1,   966,   443,   968,    -1,   656,
     657,   658,    -1,    -1,    -1,    -1,  1147,    -1,    -1,    -1,
      -1,    -1,   148,   149,    -1,   151,    -1,    -1,    -1,    -1,
     991,    -1,    -1,   994,    -1,    -1,    -1,    -1,    -1,    -1,
     687,    -1,    -1,   480,    -1,    -1,    -1,   173,    -1,   696,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   438,   705,    -1,
     707,   708,   443,  1194,    -1,    -1,    -1,   193,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   722,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1216,  1217,    -1,    -1,  1220,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   480,
     747,    -1,    41,   750,    -1,    -1,    -1,  1068,    -1,    -1,
      -1,  1072,    -1,    -1,    -1,    -1,  1077,    -1,  1079,    -1,
      -1,    -1,   769,  1084,    -1,  1086,  1087,    -1,  1089,    -1,
      -1,    -1,    -1,    -1,    -1,  1096,    -1,    -1,   519,   520,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1109,    -1,
      -1,  1112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      99,    -1,    -1,   643,   811,   104,    -1,   106,   107,   108,
     109,   110,   111,   112,    -1,    -1,   656,   657,   658,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1147,    -1,    -1,    -1,
      -1,    -1,   839,    -1,    -1,    -1,   843,    -1,   845,  1330,
      -1,    -1,    -1,    -1,    -1,    -1,   643,    -1,    -1,   148,
     149,    -1,   151,    -1,    -1,    -1,   863,  1178,    -1,   656,
     657,   658,    -1,    -1,  1355,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1194,   173,  1366,    -1,    -1,    -1,    -1,
    1371,    -1,    -1,    -1,    -1,   892,    -1,    -1,  1379,    -1,
      -1,    -1,    -1,    -1,   193,  1216,  1217,    -1,    -1,  1220,
    1221,    -1,   643,    -1,  1225,    41,    -1,   747,    -1,    -1,
      -1,    -1,    -1,   654,   655,   656,   657,   658,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1418,    -1,    -1,
     937,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1432,   949,   950,    -1,   952,    -1,    -1,    -1,    -1,
     747,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   966,
      -1,   968,    -1,    99,    -1,    -1,    -1,    -1,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   991,    -1,    -1,   994,    -1,    -1,
      -1,    -1,    -1,    -1,   735,    -1,    -1,    -1,  1489,   839,
     741,    -1,    -1,   843,  1495,   845,   747,    -1,  1499,  1330,
    1501,    -1,   148,   149,    -1,   151,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   863,    -1,    -1,    -1,    -1,    -1,    -1,
    1351,    -1,    -1,    -1,  1355,    -1,    -1,   173,    -1,    -1,
      -1,    -1,   839,    -1,    -1,  1366,   843,    -1,   845,    -1,
    1371,    -1,    -1,    -1,    -1,    -1,    -1,   193,  1379,    -1,
      -1,    -1,    -1,    -1,    -1,  1072,   863,    -1,    -1,    -1,
    1077,    -1,  1079,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1087,    -1,  1089,   824,    -1,   826,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1418,   839,    -1,
      -1,    -1,   843,    -1,   845,    -1,    -1,    -1,    -1,    -1,
      -1,  1432,   952,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1441,    -1,   863,    -1,    -1,   866,   966,    -1,   968,    -1,
      -1,    -1,    -1,    -1,    -1,  1456,    -1,    -1,    -1,    -1,
    1147,    -1,    -1,    -1,  1465,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   952,    -1,    -1,   899,    -1,
    1481,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1489,   966,
      -1,   968,    -1,    -1,  1495,    -1,    -1,    -1,  1499,    -1,
    1501,    -1,    -1,    -1,    -1,    -1,    -1,  1194,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,  1216,
    1217,   952,    -1,  1220,  1221,    -1,    -1,    -1,  1225,    -1,
      -1,    -1,   963,    -1,    -1,   966,    -1,   968,     9,    10,
      11,    -1,  1072,    -1,    -1,    -1,    -1,  1077,    -1,  1079,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    10,
      11,    -1,    -1,    -1,    -1,  1072,    -1,    -1,    -1,    -1,
    1077,    -1,  1079,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,  1147,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1330,    -1,    -1,    -1,  1068,    -1,    -1,
      -1,  1072,    -1,    -1,    -1,    -1,  1077,    -1,  1079,    -1,
      -1,    -1,    -1,  1084,    -1,  1086,    -1,    -1,  1355,    -1,
    1147,    -1,    -1,    -1,  1194,  1096,    -1,    -1,    -1,  1366,
      -1,    -1,    -1,    -1,  1371,    -1,    -1,    -1,  1109,    -1,
      -1,  1112,  1379,    -1,    -1,    -1,  1216,  1217,    -1,    -1,
    1220,    -1,    -1,    -1,  1224,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1194,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1147,    -1,    -1,    -1,
      -1,  1418,   193,    -1,    -1,    -1,    -1,    -1,    -1,  1216,
    1217,    -1,    -1,  1220,    -1,  1432,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1441,    -1,    -1,  1178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1456,
      -1,    -1,    -1,  1194,    -1,    -1,     5,     6,    -1,     8,
       9,    10,    -1,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,  1216,  1217,    26,    27,  1220,
      -1,    -1,  1489,    -1,    -1,    -1,    -1,    -1,  1495,    38,
    1330,    -1,  1499,    -1,  1501,    -1,    45,    -1,    47,    -1,
      -1,    50,    -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1355,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1366,    -1,    -1,    -1,
      79,  1371,    -1,  1330,    -1,    -1,    -1,    -1,    -1,  1379,
      -1,    -1,    -1,    -1,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1396,    -1,  1355,    -1,
     109,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1366,
      -1,    -1,    -1,    -1,  1371,    -1,    -1,    -1,  1418,    -1,
      -1,    -1,  1379,    -1,    -1,    -1,    -1,    -1,    -1,  1330,
      -1,    -1,  1432,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,
    1351,    -1,    -1,    -1,  1355,    -1,    -1,    -1,    -1,    -1,
      -1,  1418,    -1,    -1,    -1,  1366,    -1,    -1,    -1,   178,
    1371,    -1,    -1,    -1,    -1,  1432,    -1,    -1,  1379,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1489,
      -1,    -1,    -1,    -1,    -1,  1495,    -1,    -1,    -1,  1499,
      -1,  1501,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   218,
      -1,    -1,   221,   141,    -1,    -1,   144,  1418,   227,   228,
     148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,
      -1,  1432,  1489,    -1,    -1,    -1,    -1,    -1,  1495,    -1,
      -1,    -1,  1499,    -1,  1501,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   270,    -1,    -1,  1465,   193,    -1,   276,    -1,    -1,
      -1,   280,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1481,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1489,    -1,
      -1,    -1,    -1,   302,  1495,    -1,    -1,    -1,  1499,    -1,
    1501,    -1,    -1,    -1,   313,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,    -1,    -1,   357,   358,
      -1,   360,    -1,    -1,    -1,    -1,    -1,    -1,   367,   368,
     369,   370,   371,   372,   373,   374,   375,   376,   377,   378,
      -1,    -1,    -1,    -1,    -1,   384,   385,    -1,   387,   388,
     389,    -1,    -1,    -1,    -1,    -1,   395,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   405,    -1,   407,     9,
      10,    11,    -1,    -1,   413,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   423,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,   450,    -1,    -1,   453,   454,   455,    -1,    -1,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   476,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,   511,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
     549,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
     579,    69,    70,    71,    72,    73,    -1,    75,    -1,   588,
     190,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,   604,    -1,    95,    96,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,   630,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,   641,    -1,   193,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,   672,    -1,    -1,    -1,    -1,   166,   167,
     180,   169,    -1,   171,   172,   173,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,    -1,   191,   192,   193,   194,   195,    -1,   197,
     198,    -1,    -1,    -1,    -1,    -1,    -1,   716,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     729,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    -1,    -1,    -1,   745,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   755,    -1,    -1,   758,
      -1,   760,    -1,    -1,    -1,   764,    -1,    51,    -1,    -1,
      -1,    -1,    -1,    -1,   773,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   798,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     124,    49,    -1,    -1,    -1,    -1,    -1,    -1,   847,   848,
     849,    -1,    -1,    -1,   853,   854,    -1,   141,    -1,    -1,
     144,    -1,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,   157,    -1,    -1,   875,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,   888,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,     9,    10,    11,    -1,
     909,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   917,    -1,
     919,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,   947,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,   956,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   193,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,  1003,    -1,    -1,    -1,  1007,    -1,
    1009,    -1,    -1,    -1,  1013,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1024,  1025,    -1,    -1,    41,
      42,    43,    -1,    -1,  1033,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,  1081,    95,    96,    97,    98,    99,   100,   101,
     193,   103,   104,   105,   106,   107,   108,   109,   110,   111,
      -1,   113,   114,   115,   116,   117,   118,    -1,  1107,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,   188,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,   167,    -1,   169,    -1,   171,
     172,   173,   174,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,    -1,   191,
     192,   193,   194,   195,    -1,   197,   198,  1186,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    25,    12,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,  1218,
      49,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    99,   100,   101,    -1,   103,   104,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,   167,    -1,   169,    12,   171,   172,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,   191,   192,    -1,   194,
     195,    -1,   197,   198,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,   191,   192,   193,   194,   195,    -1,
     197,   198,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,   173,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    -1,   191,   192,   193,   194,   195,    -1,   197,   198,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,   192,    -1,   194,   195,    -1,   197,   198,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    89,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,   191,   192,
      -1,   194,   195,    -1,   197,   198,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,   191,   192,   193,   194,
     195,    -1,   197,   198,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,   191,   192,   193,   194,   195,    -1,
     197,   198,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    87,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,   173,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    -1,   191,   192,    -1,   194,   195,    -1,   197,   198,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,   192,   193,   194,   195,    -1,   197,   198,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,   191,   192,
     193,   194,   195,    -1,   197,   198,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      85,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,   191,   192,    -1,   194,
     195,    -1,   197,   198,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,   191,   192,   193,   194,   195,    -1,
     197,   198,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,   173,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    -1,   191,   192,   193,   194,   195,    -1,   197,   198,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,   192,   193,   194,   195,    -1,   197,   198,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,   171,   172,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,   191,   192,
      -1,   194,   195,    -1,   197,   198,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,    -1,    -1,   171,   172,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,   191,   192,    -1,   194,   195,    -1,
     197,   198,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,
     171,   172,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,   192,    -1,   194,   195,    -1,   197,   198,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,   171,   172,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,   191,   192,    -1,   194,
     195,    -1,   197,   198,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,   106,   107,   108,
     109,   110,   111,    -1,   113,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
      -1,    -1,   171,   172,   173,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    -1,   191,   192,    -1,   194,   195,    -1,   197,   198,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,   191,   192,
      -1,   194,   195,    -1,   197,   198,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,   106,   107,   108,   109,   110,   111,    -1,   113,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,   171,   172,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,   191,   192,    -1,   194,
     195,    -1,   197,   198,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    31,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    -1,    -1,    -1,    -1,   194,   195,    -1,   197,   198,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,    -1,    -1,   194,   195,    -1,   197,   198,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,   171,   172,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,    -1,    -1,
      -1,   194,   195,    -1,   197,   198,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,    -1,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,    -1,    -1,    -1,   194,   195,    -1,
     197,   198,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,    -1,   173,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    -1,   191,    -1,    -1,   194,   195,    -1,   197,   198,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,    -1,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,    -1,    -1,   194,   195,    -1,   197,   198,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,    -1,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,    -1,    -1,
      -1,   194,   195,    -1,   197,   198,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,   171,    -1,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,    -1,    -1,    -1,   194,
     195,    -1,   197,   198,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
      -1,    -1,   171,    -1,   173,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    -1,    -1,    -1,    -1,   194,   195,    -1,   197,   198,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,   171,    -1,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,    -1,    -1,
      -1,   194,   195,    -1,   197,   198,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,    -1,    -1,   171,    -1,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,    -1,    -1,    -1,   194,   195,    -1,
     197,   198,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,
     171,    -1,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      -1,    -1,    -1,   194,   195,    -1,   197,   198,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,   171,    -1,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,    -1,    -1,    -1,   194,
     195,    -1,   197,   198,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
      -1,    -1,   171,    -1,   173,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,    -1,    -1,    -1,    -1,   194,   195,    -1,   197,   198,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,    -1,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,    -1,    -1,
      -1,   194,   195,    -1,   197,   198,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,    -1,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,    -1,    -1,    -1,   194,
     195,    -1,   197,   198,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
       9,    10,    11,    12,   171,    -1,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,    -1,    -1,    -1,   194,   195,    -1,
     197,   198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    62,    63,    64,    65,    66,    67,    68,
      -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   141,   142,   143,    -1,   145,    -1,    -1,   148,
     149,    -1,   151,   152,   153,   154,   155,   156,    -1,    -1,
     159,    -1,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,
     169,    -1,   171,   172,   173,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   193,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   193,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   193,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   193,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   193,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,   193,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   193,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   191,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   191,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   191,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   191,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    71,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    25,   191,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   191,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    25,    49,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   190,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      -1,    -1,   174,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,    -1,    -1,    71,
     192,    -1,   194,    -1,    25,    -1,   184,   185,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      51,    -1,    -1,    -1,   106,   107,   108,   109,   110,   111,
      -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,    -1,    -1,
      71,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,    -1,   144,    25,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   166,    31,    -1,    -1,    -1,    51,
      -1,   173,   174,   124,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,    -1,    71,
     141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    71,   157,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      -1,    -1,   124,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,    -1,   141,
      71,    -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,   141,    -1,    71,   144,    73,
     146,    -1,   148,   149,   166,   151,   152,   153,    -1,    -1,
      -1,    -1,   174,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    42,    43,    -1,    -1,   194,    -1,
      -1,    -1,    -1,   144,    -1,    -1,    -1,   148,   149,    -1,
     151,   152,   153,    61,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    70,    71,   148,   149,    -1,   151,   152,   153,
      -1,    79,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,    -1,    -1,
     174,   192,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,    -1,    -1,    -1,   192,    -1,
     194,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,   127,
     128,   129,    -1,    42,    43,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    61,   151,   152,   153,    -1,    -1,   156,    -1,
      69,    70,    71,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      79,    -1,    -1,   171,    -1,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    73,    -1,    -1,   124,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    63,    -1,    -1,    -1,   166,    -1,    -1,
      -1,    71,    -1,    73,    -1,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,   160,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    73,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      -1,   141,    -1,   194,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      -1,    73,    -1,    -1,   174,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     141,    -1,    -1,   144,   194,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,
      73,   123,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   141,
      -1,   192,   144,   194,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,
     123,    -1,   174,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   141,    -1,
      -1,   144,   194,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,
      -1,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   141,    -1,    -1,
     144,   194,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,
      71,    -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   141,    -1,    -1,   144,
     194,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,    -1,   148,   149,   194,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    70,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,    -1,    -1,
      -1,   192,    -1,   194,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,    -1,    -1,    -1,   192,    -1,
     194,   148,   149,    -1,   151,   152,   153,    -1,    -1,    71,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   146,    -1,   148,   149,   150,   151,   152,   153,
      -1,    -1,    -1,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      -1,    -1,   174,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   148,   149,    71,   151,   152,   153,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,    -1,   112,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    71,    -1,    73,    74,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,    71,    -1,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   174,    71,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   148,
     149,    71,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,
     144,    -1,    -1,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,    -1,    -1,    -1,    -1,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,    -1,   174,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,   122,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   122,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   122,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   122,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,     9,    10,
      11,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    11,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   200,   201,     0,   202,     3,     4,     5,     6,     7,
      12,    41,    42,    43,    48,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    69,    70,    71,    72,    73,    75,    79,    80,    81,
      82,    84,    86,    88,    91,    95,    96,    97,    98,    99,
     100,   101,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   113,   114,   115,   116,   117,   118,   123,   124,   125,
     126,   127,   128,   129,   136,   141,   142,   143,   144,   145,
     146,   148,   149,   151,   152,   153,   154,   156,   160,   166,
     167,   169,   171,   172,   173,   174,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     191,   192,   194,   195,   197,   198,   203,   206,   209,   210,
     211,   212,   213,   214,   217,   232,   233,   237,   242,   248,
     303,   304,   309,   313,   314,   315,   316,   317,   318,   319,
     320,   328,   329,   330,   332,   333,   335,   355,   365,   366,
     367,   372,   375,   393,   398,   400,   401,   402,   403,   404,
     405,   406,   407,   409,   422,   424,   426,   109,   110,   111,
     123,   141,   206,   232,   303,   319,   400,   319,   189,   319,
     319,   319,   391,   392,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   111,   189,   210,   366,
     367,   400,   400,    31,   319,   413,   414,   319,   111,   189,
     210,   366,   367,   368,   399,   405,   410,   411,   189,   310,
     369,   189,   310,   311,   319,   219,   310,   189,   189,   189,
     310,   191,   319,   206,   191,   319,    25,    51,   124,   146,
     166,   189,   206,   213,   427,   437,   438,   175,   191,   316,
     319,   334,   336,   192,   225,   319,   144,   207,   208,   209,
      73,   194,   274,   275,   117,   117,    73,   276,   189,   189,
     189,   189,   206,   246,   428,   189,   189,    73,    78,   137,
     138,   139,   419,   420,   144,   192,   209,   209,    95,   319,
     247,   428,   146,   189,   428,   428,   319,   309,   319,   320,
     400,   215,   192,    78,   370,   419,    78,   419,   419,    26,
     144,   162,   429,   189,     8,   191,    31,   231,   146,   245,
     428,   111,   232,   304,   191,   191,   191,   191,   191,   191,
       9,    10,    11,    25,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    49,   191,    61,    61,   191,
     192,   140,   118,   154,   248,   302,   303,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    59,
      60,   121,   395,   396,    61,   192,   397,   189,    61,   192,
     194,   406,   189,   231,   232,    13,   319,    40,   206,   390,
     189,   309,   400,   140,   400,   122,   196,     8,   377,   309,
     400,   429,   140,   189,   371,   121,   395,   396,   397,   190,
     319,    26,   217,     8,   191,   217,   218,   311,   312,   319,
     206,   260,   221,   191,   191,   191,   438,   438,   162,   189,
      98,   430,   438,    13,   206,    73,   191,   191,   191,   175,
     176,   177,   182,   183,   186,   187,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   349,   350,   351,   226,   102,
     159,   191,   209,     8,   191,    90,   192,   400,     8,   191,
      13,     8,   191,   400,   423,   423,   309,   320,   400,   190,
     162,   240,   123,   400,   412,   413,    61,   121,   137,   420,
      72,   319,   400,    78,   137,   420,   209,   205,   191,   192,
     191,   122,   243,   356,   358,    79,   323,   324,   326,    13,
      90,   425,   190,   190,   190,   193,   216,   217,   233,   237,
     242,   319,   195,   197,   198,   206,   430,    31,   272,   273,
     319,   427,   189,   428,   238,   231,   319,   319,   319,    26,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   368,   319,   408,   408,   319,   415,   416,   192,
     206,   405,   406,   246,   247,   245,   232,    31,   145,   313,
     316,   319,   334,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   192,   206,   405,   408,   319,   272,
     408,   319,   412,   231,   190,   189,   389,     8,   377,   309,
     190,   206,    31,   319,    31,   319,   190,   190,   405,   272,
     192,   206,   405,   190,   215,   264,   319,    82,    26,   217,
     258,   191,    90,    13,     8,   190,    26,   192,   261,   438,
      79,   434,   435,   436,   189,     8,    42,    43,    61,   124,
     136,   146,   166,   174,   210,   211,   213,   331,   366,   372,
     373,   374,   178,    73,   319,    73,    73,   319,   346,   347,
     319,   319,   339,   349,   181,   352,   215,   189,   224,    90,
     208,   206,   319,   275,   373,    73,     8,   190,   190,   190,
     190,   190,   191,   206,   433,   119,   251,   189,     8,   190,
     190,    73,    74,   206,   421,   206,    61,   193,   193,   202,
     204,   319,   120,   250,   161,    46,   146,   161,   360,   122,
       8,   377,   190,   438,   438,    13,   121,   395,   396,   397,
     193,     8,   163,   400,   190,     8,   378,    13,   321,   234,
     119,   249,   189,   428,   319,    26,   196,   196,   122,   193,
       8,   377,   319,   429,   189,   241,   244,   239,   231,    63,
     400,   319,   429,   189,   196,   193,   190,   196,   193,   190,
      42,    43,    61,    69,    70,    79,   124,   136,   166,   174,
     206,   380,   382,   385,   388,   206,   400,   400,   122,   395,
     396,   397,   190,   319,   265,    66,    67,   266,   310,   215,
     312,    31,   123,   255,   400,   373,   206,    26,   217,   259,
     191,   262,   191,   262,     8,   163,   122,     8,   377,   190,
     157,   430,   431,   438,   373,   373,   373,   376,   379,   189,
      78,   140,   189,   189,   140,   192,   319,   178,   178,    13,
     184,   185,   348,     8,   188,   352,    73,   193,   366,   192,
     228,   206,   193,    13,   400,   191,    90,     8,   163,   252,
     366,   192,   412,   123,   400,    13,   196,   319,   193,   202,
     252,   192,   359,    13,   319,   323,   191,   438,   192,   206,
     405,   438,    31,   319,   373,   157,   270,   271,   393,   394,
     189,   366,   250,   322,   235,   319,   319,   319,   193,   189,
     272,   251,   250,   249,   428,   368,   193,   189,   272,    13,
      69,    70,   206,   381,   381,   382,   383,   384,   189,    78,
     137,   189,   189,     8,   377,   190,   389,    31,   319,   193,
      66,    67,   267,   310,   217,   191,    83,   191,   400,   189,
     122,   254,    13,   215,   262,    92,    93,    94,   262,   193,
     438,   438,   434,     8,   190,   190,   122,   196,     8,   377,
     376,   206,   323,   325,   327,   376,   206,   373,   417,   418,
     319,   319,   319,   347,   319,   337,    73,   229,   373,   438,
     206,     8,   277,   190,   189,   313,   316,   319,   196,   193,
     277,   147,   160,   192,   355,   362,   147,   192,   361,   122,
     191,   319,   429,   189,   400,   190,     8,   378,   438,   439,
     270,   192,   270,   189,   122,   189,   272,   190,   192,   192,
     250,   236,   371,   189,   272,   190,   122,   196,     8,   377,
     383,   137,   323,   386,   387,   383,   382,   400,   310,    26,
      68,   217,   191,   312,   412,   255,   190,   373,    89,    92,
     191,   319,    26,   191,   263,   193,   163,   157,    26,   373,
     373,   190,   122,     8,   377,   190,   190,   122,   193,     8,
     377,   179,   190,   215,    90,   366,    99,   104,   112,   148,
     149,   151,   193,   278,   301,   302,   303,   308,   393,   412,
     193,   193,    46,   319,   319,   319,   193,   189,   272,    26,
     432,   157,   394,    31,    73,   190,   277,   190,   270,   319,
     272,   190,   277,   277,   192,   189,   272,   190,   382,   382,
     190,   122,   190,     8,   377,   190,    26,   215,   191,   190,
     190,   222,   191,   191,   263,   215,   438,   122,   373,   323,
     373,   373,   319,   192,   193,   438,   427,   253,   366,   112,
     124,   146,   152,   287,   288,   289,   366,   150,   293,   294,
     115,   189,   206,   295,   296,   279,   232,   438,     8,   191,
     302,   190,   146,   357,   193,   193,   189,   272,   190,   438,
     104,   353,   439,    73,    13,   432,   193,   432,   190,   190,
     193,   193,   277,   270,   190,   122,   382,   323,   215,   220,
      26,   217,   257,   215,   190,   373,   122,   122,   180,   215,
      13,     8,   191,   192,   192,     8,   191,     3,     4,     5,
       6,     7,     9,    10,    11,    12,    49,    62,    63,    64,
      65,    66,    67,    68,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   123,   124,   125,   126,   127,
     128,   129,   141,   142,   143,   145,   154,   155,   156,   159,
     166,   167,   169,   171,   172,   173,   206,   363,   364,     8,
     191,   146,   150,   206,   296,   297,   298,   191,    73,   307,
     231,   280,   427,   232,   272,   190,   189,   192,    31,    73,
      13,   373,   191,   192,   299,   353,   432,   193,   190,   382,
     122,    26,   217,   256,   215,   373,   373,   319,   193,   373,
     366,   283,   290,   372,   288,    13,    26,    43,   291,   294,
       8,    29,   190,    25,    42,    45,    13,     8,   191,   428,
     307,    13,   231,   190,    31,    73,   354,   215,    73,    13,
     373,   215,   192,   299,   432,   382,   215,    87,   181,   227,
     193,   206,   213,   284,   285,   286,     8,   193,   373,   364,
     364,    51,   292,   297,   297,    25,    42,    45,   373,    73,
     189,   191,   373,   428,    73,     8,   378,   193,    13,   373,
     193,   215,   299,    85,   191,    73,   102,   223,   140,    90,
     372,   153,    13,   281,   189,    31,    73,   190,   373,   193,
     191,   189,   159,   230,   206,   302,   303,   373,   157,   268,
     269,   394,   282,    73,   366,   228,   155,   206,   191,   190,
       8,   378,   106,   107,   108,   305,   306,   268,    73,   253,
     191,   432,   157,   394,   439,   190,   190,   191,   191,   192,
     300,   305,    31,    73,   432,   192,   215,   439,    73,    13,
     300,   215,   193,    31,    73,    13,   373,   193,    73,    13,
     373,    13,   373,   373
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
#line 716 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 719 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 726 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 727 "hphp.y"
    { }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 730 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 731 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 732 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 733 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 735 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 738 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 740 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 741 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 742 "hphp.y"
    { _p->onNamespaceStart("");}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 743 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 744 "hphp.y"
    { _p->nns(); (yyval).reset();}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 745 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 750 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 751 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 752 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 753 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 754 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 755 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 756 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 757 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 759 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 761 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 762 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 764 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 765 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 767 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 774 "hphp.y"
    { }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 775 "hphp.y"
    { }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 778 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 779 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 780 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 782 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 793 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 794 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 797 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 804 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 811 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 819 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 822 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 828 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 829 "hphp.y"
    { _p->onStatementListStart((yyval));}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 838 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 842 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 847 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 848 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 850 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 854 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 857 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 861 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 863 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 866 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 868 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 871 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 872 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 873 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 874 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 875 "hphp.y"
    { _p->onReturn((yyval), NULL);}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 876 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 877 "hphp.y"
    { _p->onYieldBreak((yyval));}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 878 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 879 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 880 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 881 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 882 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 883 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 886 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 888 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 892 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 899 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 900 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 903 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 904 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 905 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 906 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 910 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 911 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 912 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 913 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 914 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 915 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 916 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 917 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 918 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 919 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 920 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 928 "hphp.y"
    { _p->onNewLabelScope(false);}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 929 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 938 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 939 "hphp.y"
    { (yyval).reset();}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 943 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 945 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 951 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 952 "hphp.y"
    { (yyval).reset();}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 956 "hphp.y"
    { (yyval) = 1;}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 957 "hphp.y"
    { (yyval).reset();}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 961 "hphp.y"
    { _p->pushFuncLocation();}
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 966 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 972 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 978 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 984 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 990 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 996 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1004 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1007 "hphp.y"
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
#line 1022 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1025 "hphp.y"
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
#line 1039 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1042 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1047 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1050 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1057 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1060 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1068 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1071 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1079 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1080 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1084 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1087 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1090 "hphp.y"
    { (yyval) = T_CLASS;}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1091 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1092 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1096 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1097 "hphp.y"
    { (yyval).reset();}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1100 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1101 "hphp.y"
    { (yyval).reset();}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1104 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1105 "hphp.y"
    { (yyval).reset();}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1108 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1110 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 1113 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 1115 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1119 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1120 "hphp.y"
    { (yyval).reset();}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1124 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1125 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1131 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1136 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1146 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1156 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1157 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1158 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1159 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1164 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1166 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval).reset();}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval).reset();}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval).reset();}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1176 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval).reset();}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1182 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1183 "hphp.y"
    { (yyval).reset();}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1187 "hphp.y"
    { (yyval).reset();}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1190 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1191 "hphp.y"
    { (yyval).reset();}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1198 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1199 "hphp.y"
    { (yyval).reset();}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1200 "hphp.y"
    { (yyval).reset();}
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1206 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1210 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1215 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1220 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1225 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1230 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1236 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1242 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1248 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1251 "hphp.y"
    { (yyval).reset();}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1252 "hphp.y"
    { (yyval).reset();}
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1257 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1260 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1268 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1272 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1276 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);}
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1281 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1286 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);}
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1293 "hphp.y"
    { (yyval).reset();}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1296 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1297 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1299 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1301 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1310 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1311 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1319 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1324 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1325 "hphp.y"
    { (yyval).reset();}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1329 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1332 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1333 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1335 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1339 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1345 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1358 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1363 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1365 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1367 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1368 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1371 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1374 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1375 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1376 "hphp.y"
    { (yyval).reset(); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1382 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1389 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1397 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1402 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1405 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1412 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1414 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1418 "hphp.y"
    { (yyval) = 4;}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1419 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1425 "hphp.y"
    { (yyval) = 6;}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1427 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1431 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1433 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1437 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1438 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1442 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1443 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1447 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1450 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1455 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1460 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1461 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1463 "hphp.y"
    { (yyval) = 0;}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1467 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1468 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1469 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1470 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1475 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1476 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1477 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1478 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1480 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1482 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1489 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1490 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1494 "hphp.y"
    { (yyval).reset();}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1495 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1499 "hphp.y"
    { (yyval).reset();}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1500 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1503 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1504 "hphp.y"
    { (yyval).reset();}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1507 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1508 "hphp.y"
    { (yyval).reset();}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1511 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1513 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1516 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1517 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1518 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1519 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1520 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1521 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1522 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1526 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1527 "hphp.y"
    { (yyval).reset();}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1530 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1538 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1549 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1552 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1553 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1554 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1557 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1561 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1562 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1566 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1567 "hphp.y"
    { (yyval).reset();}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1572 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1581 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1585 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1594 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1599 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1604 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1606 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1610 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1611 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1612 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1613 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1614 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1615 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1616 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1617 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1618 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1619 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1620 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1621 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1622 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1623 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1624 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1628 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1633 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1634 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1635 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1638 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1643 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1644 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1645 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1647 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1648 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1649 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1650 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1652 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1653 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1656 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1657 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1658 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1659 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1661 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1662 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1663 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1664 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1665 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1666 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1667 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1668 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1669 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1674 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1676 "hphp.y"
    { Token t; 
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1682 "hphp.y"
    { Token u; u.reset();
                                         _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         _p->onClosure((yyval),0,u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1688 "hphp.y"
    { Token t; 
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1694 "hphp.y"
    { Token u; u.reset();
                                         _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         _p->onClosure((yyval),&(yyvsp[(1) - (12)]),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1703 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1710 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1713 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1723 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1728 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1729 "hphp.y"
    { (yyval).reset(); }
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 1735 "hphp.y"
    { (yyval).reset(); }
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1739 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 1743 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1744 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1749 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1756 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1763 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1765 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1771 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1775 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1783 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1788 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); }
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1790 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1792 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); }
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1794 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1798 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1800 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1813 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1817 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1821 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1826 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1831 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1839 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1840 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); }
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1845 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1859 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval).reset();}
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1877 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1878 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1880 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1887 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1890 "hphp.y"
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

  case 458:

/* Line 1806 of yacc.c  */
#line 1901 "hphp.y"
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

  case 459:

/* Line 1806 of yacc.c  */
#line 1912 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1913 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1919 "hphp.y"
    { (yyval).reset();}
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval).reset();}
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1926 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1930 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1933 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 1936 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       }
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 1943 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 1944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 1948 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1950 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1952 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 1956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 1957 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 1958 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 1959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 1967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 1971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 1972 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 1975 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 1977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 1978 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 1979 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 1981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 1982 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 1983 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 1984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 1985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 1986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 1987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 1988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 1989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 1990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 1991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 1992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 1993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 1994 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 1995 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 1997 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 1998 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 1999 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2000 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2002 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2003 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2016 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2021 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2022 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2030 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2039 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2043 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2044 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2047 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2049 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2053 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2054 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2055 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2059 "hphp.y"
    { (yyval).reset();}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2060 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2061 "hphp.y"
    { (yyval).reset();}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval).reset();}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2066 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval).reset();}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2076 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2077 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2078 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2081 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2083 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2084 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2087 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2088 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2099 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2100 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2104 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2105 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2107 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2115 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2117 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2127 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2129 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval).reset();}
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval).reset();}
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval).reset();}
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2143 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval).reset();}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2150 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2167 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2171 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2172 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2176 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2177 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2178 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2185 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval).reset();}
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2198 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2201 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2205 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval).reset(); }
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2217 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2226 "hphp.y"
    { (yyval).reset();}
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2230 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2242 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2244 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2247 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval).reset();}
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2273 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2278 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2280 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 663:

/* Line 1806 of yacc.c  */
#line 2284 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2289 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2291 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2294 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2296 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2297 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2308 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2310 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2311 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2316 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2317 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2352 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2360 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2362 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2367 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2368 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2373 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval).reset();}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = 1;}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval)++;}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2388 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2390 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2393 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2402 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2403 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2407 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2410 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2413 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2419 "hphp.y"
    { (yyval).reset();}
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2423 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2424 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2425 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2426 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2429 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2431 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2432 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2439 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2444 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2445 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2446 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2451 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2452 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2457 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2459 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2462 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2466 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 744:

/* Line 1806 of yacc.c  */
#line 2468 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 745:

/* Line 1806 of yacc.c  */
#line 2469 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2471 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2476 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2478 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2480 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2482 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2484 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2485 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2488 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2490 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2494 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2495 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2496 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2497 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2498 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 761:

/* Line 1806 of yacc.c  */
#line 2499 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 762:

/* Line 1806 of yacc.c  */
#line 2500 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 763:

/* Line 1806 of yacc.c  */
#line 2501 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 764:

/* Line 1806 of yacc.c  */
#line 2502 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 765:

/* Line 1806 of yacc.c  */
#line 2506 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 766:

/* Line 1806 of yacc.c  */
#line 2507 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 767:

/* Line 1806 of yacc.c  */
#line 2512 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 770:

/* Line 1806 of yacc.c  */
#line 2526 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 771:

/* Line 1806 of yacc.c  */
#line 2529 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 772:

/* Line 1806 of yacc.c  */
#line 2534 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 773:

/* Line 1806 of yacc.c  */
#line 2535 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 774:

/* Line 1806 of yacc.c  */
#line 2541 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 775:

/* Line 1806 of yacc.c  */
#line 2545 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 776:

/* Line 1806 of yacc.c  */
#line 2551 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 777:

/* Line 1806 of yacc.c  */
#line 2552 "hphp.y"
    { (yyval).reset(); }
    break;

  case 778:

/* Line 1806 of yacc.c  */
#line 2556 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 779:

/* Line 1806 of yacc.c  */
#line 2559 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 780:

/* Line 1806 of yacc.c  */
#line 2564 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 781:

/* Line 1806 of yacc.c  */
#line 2565 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 782:

/* Line 1806 of yacc.c  */
#line 2566 "hphp.y"
    { (yyval).reset(); }
    break;

  case 783:

/* Line 1806 of yacc.c  */
#line 2567 "hphp.y"
    { (yyval).reset(); }
    break;

  case 784:

/* Line 1806 of yacc.c  */
#line 2571 "hphp.y"
    { (yyval).reset(); }
    break;

  case 785:

/* Line 1806 of yacc.c  */
#line 2572 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 786:

/* Line 1806 of yacc.c  */
#line 2577 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 787:

/* Line 1806 of yacc.c  */
#line 2578 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 788:

/* Line 1806 of yacc.c  */
#line 2580 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 789:

/* Line 1806 of yacc.c  */
#line 2581 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 790:

/* Line 1806 of yacc.c  */
#line 2587 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 793:

/* Line 1806 of yacc.c  */
#line 2598 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 794:

/* Line 1806 of yacc.c  */
#line 2600 "hphp.y"
    {}
    break;

  case 795:

/* Line 1806 of yacc.c  */
#line 2604 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 796:

/* Line 1806 of yacc.c  */
#line 2611 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 797:

/* Line 1806 of yacc.c  */
#line 2614 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 798:

/* Line 1806 of yacc.c  */
#line 2617 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 799:

/* Line 1806 of yacc.c  */
#line 2618 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 800:

/* Line 1806 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 801:

/* Line 1806 of yacc.c  */
#line 2623 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 802:

/* Line 1806 of yacc.c  */
#line 2626 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 803:

/* Line 1806 of yacc.c  */
#line 2629 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 804:

/* Line 1806 of yacc.c  */
#line 2635 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 805:

/* Line 1806 of yacc.c  */
#line 2639 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 806:

/* Line 1806 of yacc.c  */
#line 2647 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 807:

/* Line 1806 of yacc.c  */
#line 2648 "hphp.y"
    { (yyval).reset(); }
    break;



/* Line 1806 of yacc.c  */
#line 11793 "hphp.tab.cpp"
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
#line 2651 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

