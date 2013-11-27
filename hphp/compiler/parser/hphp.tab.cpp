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
     T_TUPLE = 410
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
#line 845 "hphp.tab.cpp"

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
#define YYLAST   11142

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  185
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  222
/* YYNRULES -- Number of rules.  */
#define YYNRULES  760
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1427

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   410

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   183,     2,   180,    47,    31,   184,
     175,   176,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   177,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   182,    30,     2,   181,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   178,    29,   179,    50,     2,     2,     2,
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
     174
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    26,    30,    31,    38,    39,    45,    49,    52,
      54,    56,    58,    60,    62,    64,    66,    70,    72,    74,
      77,    81,    86,    88,    92,    94,    98,   101,   103,   106,
     109,   115,   120,   123,   124,   126,   128,   130,   132,   136,
     142,   151,   152,   157,   158,   165,   166,   177,   178,   183,
     186,   190,   193,   197,   200,   204,   208,   212,   216,   220,
     226,   228,   230,   231,   241,   247,   248,   262,   263,   269,
     273,   277,   280,   283,   286,   289,   292,   295,   299,   302,
     305,   306,   311,   321,   322,   323,   328,   331,   332,   334,
     335,   337,   338,   348,   349,   360,   361,   373,   374,   383,
     384,   394,   395,   403,   404,   413,   414,   422,   423,   432,
     434,   436,   438,   440,   442,   445,   448,   451,   452,   455,
     456,   459,   460,   462,   466,   468,   472,   475,   476,   478,
     481,   486,   488,   493,   495,   500,   502,   507,   509,   514,
     518,   524,   528,   533,   538,   544,   550,   555,   556,   558,
     560,   565,   566,   572,   573,   576,   577,   581,   582,   586,
     589,   591,   592,   597,   603,   611,   618,   625,   633,   643,
     652,   656,   659,   661,   662,   666,   671,   678,   684,   690,
     697,   706,   714,   717,   718,   720,   723,   727,   732,   736,
     738,   740,   743,   748,   752,   758,   760,   764,   767,   768,
     769,   774,   775,   781,   784,   785,   796,   797,   809,   813,
     817,   821,   825,   831,   834,   837,   838,   845,   851,   856,
     860,   862,   864,   868,   873,   875,   877,   879,   881,   886,
     888,   892,   895,   896,   899,   900,   902,   906,   908,   910,
     912,   914,   918,   923,   928,   933,   935,   937,   940,   943,
     946,   950,   954,   956,   958,   960,   962,   966,   968,   972,
     974,   976,   978,   979,   981,   984,   986,   988,   990,   992,
     994,   996,   998,  1000,  1001,  1003,  1005,  1007,  1011,  1017,
    1019,  1023,  1029,  1034,  1038,  1042,  1045,  1047,  1049,  1053,
    1057,  1059,  1061,  1062,  1065,  1070,  1074,  1081,  1084,  1088,
    1095,  1097,  1099,  1101,  1108,  1112,  1117,  1124,  1128,  1132,
    1136,  1140,  1144,  1148,  1152,  1156,  1160,  1164,  1168,  1171,
    1174,  1177,  1180,  1184,  1188,  1192,  1196,  1200,  1204,  1208,
    1212,  1216,  1220,  1224,  1228,  1232,  1236,  1240,  1244,  1247,
    1250,  1253,  1256,  1260,  1264,  1268,  1272,  1276,  1280,  1284,
    1288,  1292,  1296,  1302,  1307,  1309,  1312,  1315,  1318,  1321,
    1324,  1327,  1330,  1333,  1336,  1338,  1340,  1342,  1346,  1349,
    1350,  1362,  1363,  1376,  1378,  1380,  1386,  1390,  1396,  1400,
    1403,  1404,  1407,  1408,  1413,  1418,  1422,  1427,  1432,  1437,
    1442,  1444,  1446,  1450,  1456,  1457,  1461,  1466,  1468,  1471,
    1476,  1479,  1486,  1487,  1489,  1494,  1495,  1498,  1499,  1501,
    1503,  1507,  1509,  1513,  1515,  1517,  1521,  1525,  1527,  1529,
    1531,  1533,  1535,  1537,  1539,  1541,  1543,  1545,  1547,  1549,
    1551,  1553,  1555,  1557,  1559,  1561,  1563,  1565,  1567,  1569,
    1571,  1573,  1575,  1577,  1579,  1581,  1583,  1585,  1587,  1589,
    1591,  1593,  1595,  1597,  1599,  1601,  1603,  1605,  1607,  1609,
    1611,  1613,  1615,  1617,  1619,  1621,  1623,  1625,  1627,  1629,
    1631,  1633,  1635,  1637,  1639,  1641,  1643,  1645,  1647,  1649,
    1651,  1653,  1655,  1657,  1659,  1661,  1663,  1665,  1667,  1669,
    1671,  1673,  1675,  1677,  1679,  1681,  1683,  1685,  1690,  1692,
    1694,  1696,  1698,  1700,  1702,  1704,  1706,  1709,  1711,  1712,
    1713,  1715,  1717,  1721,  1722,  1724,  1726,  1728,  1730,  1732,
    1734,  1736,  1738,  1740,  1742,  1744,  1746,  1750,  1753,  1755,
    1757,  1760,  1763,  1768,  1773,  1777,  1782,  1784,  1786,  1790,
    1794,  1796,  1798,  1800,  1802,  1806,  1810,  1814,  1817,  1818,
    1820,  1821,  1823,  1824,  1830,  1834,  1838,  1840,  1842,  1844,
    1846,  1850,  1853,  1855,  1857,  1859,  1861,  1863,  1866,  1869,
    1874,  1879,  1883,  1888,  1891,  1892,  1898,  1902,  1906,  1908,
    1912,  1914,  1917,  1918,  1924,  1928,  1931,  1932,  1936,  1937,
    1942,  1945,  1946,  1950,  1954,  1956,  1957,  1959,  1962,  1965,
    1970,  1974,  1978,  1981,  1986,  1989,  1994,  1996,  1998,  2000,
    2002,  2004,  2007,  2012,  2016,  2021,  2025,  2027,  2029,  2031,
    2033,  2036,  2041,  2046,  2050,  2052,  2054,  2058,  2066,  2073,
    2082,  2092,  2101,  2112,  2120,  2127,  2136,  2138,  2141,  2146,
    2151,  2153,  2155,  2160,  2162,  2163,  2165,  2168,  2170,  2172,
    2175,  2180,  2184,  2188,  2189,  2191,  2194,  2199,  2203,  2206,
    2210,  2217,  2218,  2220,  2225,  2228,  2229,  2235,  2239,  2243,
    2245,  2252,  2257,  2262,  2265,  2268,  2269,  2275,  2279,  2283,
    2285,  2288,  2289,  2295,  2299,  2303,  2305,  2308,  2311,  2313,
    2316,  2318,  2323,  2327,  2331,  2338,  2342,  2344,  2346,  2348,
    2353,  2358,  2363,  2368,  2371,  2374,  2379,  2382,  2385,  2387,
    2391,  2395,  2396,  2399,  2405,  2412,  2414,  2417,  2419,  2424,
    2428,  2429,  2431,  2435,  2439,  2441,  2443,  2444,  2445,  2448,
    2452,  2454,  2460,  2464,  2468,  2472,  2474,  2477,  2478,  2483,
    2486,  2489,  2491,  2493,  2495,  2500,  2507,  2509,  2518,  2524,
    2526
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     186,     0,    -1,    -1,   187,   188,    -1,   188,   189,    -1,
      -1,   203,    -1,   219,    -1,   223,    -1,   228,    -1,   393,
      -1,   116,   175,   176,   177,    -1,   141,   195,   177,    -1,
      -1,   141,   195,   178,   190,   188,   179,    -1,    -1,   141,
     178,   191,   188,   179,    -1,   104,   193,   177,    -1,   200,
     177,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   174,    -1,   193,     8,   194,    -1,
     194,    -1,   195,    -1,   144,   195,    -1,   195,    90,   192,
      -1,   144,   195,    90,   192,    -1,   192,    -1,   195,   144,
     192,    -1,   195,    -1,   141,   144,   195,    -1,   144,   195,
      -1,   196,    -1,   196,   396,    -1,   196,   396,    -1,   200,
       8,   394,    13,   340,    -1,    99,   394,    13,   340,    -1,
     201,   202,    -1,    -1,   203,    -1,   219,    -1,   223,    -1,
     228,    -1,   178,   201,   179,    -1,    65,   296,   203,   250,
     252,    -1,    65,   296,    26,   201,   251,   253,    68,   177,
      -1,    -1,    82,   296,   204,   244,    -1,    -1,    81,   205,
     203,    82,   296,   177,    -1,    -1,    84,   175,   298,   177,
     298,   177,   298,   176,   206,   242,    -1,    -1,    91,   296,
     207,   247,    -1,    95,   177,    -1,    95,   305,   177,    -1,
      97,   177,    -1,    97,   305,   177,    -1,   100,   177,    -1,
     100,   305,   177,    -1,   145,    95,   177,    -1,   105,   260,
     177,    -1,   111,   262,   177,    -1,    80,   297,   177,    -1,
     113,   175,   390,   176,   177,    -1,   177,    -1,    75,    -1,
      -1,    86,   175,   305,    90,   241,   240,   176,   208,   243,
      -1,    88,   175,   246,   176,   245,    -1,    -1,   101,   211,
     102,   175,   333,    73,   176,   178,   201,   179,   213,   209,
     216,    -1,    -1,   101,   211,   159,   210,   214,    -1,   103,
     305,   177,    -1,    96,   192,   177,    -1,   305,   177,    -1,
     299,   177,    -1,   300,   177,    -1,   301,   177,    -1,   302,
     177,    -1,   303,   177,    -1,   100,   302,   177,    -1,   304,
     177,    -1,   192,    26,    -1,    -1,   178,   212,   201,   179,
      -1,   213,   102,   175,   333,    73,   176,   178,   201,   179,
      -1,    -1,    -1,   178,   215,   201,   179,    -1,   159,   214,
      -1,    -1,    31,    -1,    -1,    98,    -1,    -1,   218,   217,
     395,   220,   175,   256,   176,   399,   285,    -1,    -1,   289,
     218,   217,   395,   221,   175,   256,   176,   399,   285,    -1,
      -1,   360,   288,   218,   217,   395,   222,   175,   256,   176,
     399,   285,    -1,    -1,   234,   231,   224,   235,   236,   178,
     263,   179,    -1,    -1,   360,   234,   231,   225,   235,   236,
     178,   263,   179,    -1,    -1,   118,   232,   226,   237,   178,
     263,   179,    -1,    -1,   360,   118,   232,   227,   237,   178,
     263,   179,    -1,    -1,   154,   233,   229,   236,   178,   263,
     179,    -1,    -1,   360,   154,   233,   230,   236,   178,   263,
     179,    -1,   395,    -1,   146,    -1,   395,    -1,   395,    -1,
     117,    -1,   110,   117,    -1,   109,   117,    -1,   119,   333,
      -1,    -1,   120,   238,    -1,    -1,   119,   238,    -1,    -1,
     333,    -1,   238,     8,   333,    -1,   333,    -1,   239,     8,
     333,    -1,   122,   241,    -1,    -1,   367,    -1,    31,   367,
      -1,   123,   175,   379,   176,    -1,   203,    -1,    26,   201,
      85,   177,    -1,   203,    -1,    26,   201,    87,   177,    -1,
     203,    -1,    26,   201,    83,   177,    -1,   203,    -1,    26,
     201,    89,   177,    -1,   192,    13,   340,    -1,   246,     8,
     192,    13,   340,    -1,   178,   248,   179,    -1,   178,   177,
     248,   179,    -1,    26,   248,    92,   177,    -1,    26,   177,
     248,    92,   177,    -1,   248,    93,   305,   249,   201,    -1,
     248,    94,   249,   201,    -1,    -1,    26,    -1,   177,    -1,
     250,    66,   296,   203,    -1,    -1,   251,    66,   296,    26,
     201,    -1,    -1,    67,   203,    -1,    -1,    67,    26,   201,
      -1,    -1,   255,     8,   157,    -1,   255,   345,    -1,   157,
      -1,    -1,   361,   291,   406,    73,    -1,   361,   291,   406,
      31,    73,    -1,   361,   291,   406,    31,    73,    13,   340,
      -1,   361,   291,   406,    73,    13,   340,    -1,   255,     8,
     361,   291,   406,    73,    -1,   255,     8,   361,   291,   406,
      31,    73,    -1,   255,     8,   361,   291,   406,    31,    73,
      13,   340,    -1,   255,     8,   361,   291,   406,    73,    13,
     340,    -1,   257,     8,   157,    -1,   257,   345,    -1,   157,
      -1,    -1,   361,   406,    73,    -1,   361,   406,    31,    73,
      -1,   361,   406,    31,    73,    13,   340,    -1,   361,   406,
      73,    13,   340,    -1,   257,     8,   361,   406,    73,    -1,
     257,     8,   361,   406,    31,    73,    -1,   257,     8,   361,
     406,    31,    73,    13,   340,    -1,   257,     8,   361,   406,
      73,    13,   340,    -1,   259,   345,    -1,    -1,   305,    -1,
      31,   367,    -1,   259,     8,   305,    -1,   259,     8,    31,
     367,    -1,   260,     8,   261,    -1,   261,    -1,    73,    -1,
     180,   367,    -1,   180,   178,   305,   179,    -1,   262,     8,
      73,    -1,   262,     8,    73,    13,   340,    -1,    73,    -1,
      73,    13,   340,    -1,   263,   264,    -1,    -1,    -1,   287,
     265,   293,   177,    -1,    -1,   289,   405,   266,   293,   177,
      -1,   294,   177,    -1,    -1,   288,   218,   217,   395,   175,
     267,   254,   176,   399,   286,    -1,    -1,   360,   288,   218,
     217,   395,   175,   268,   254,   176,   399,   286,    -1,   148,
     273,   177,    -1,   149,   279,   177,    -1,   151,   281,   177,
      -1,   104,   239,   177,    -1,   104,   239,   178,   269,   179,
      -1,   269,   270,    -1,   269,   271,    -1,    -1,   199,   140,
     192,   155,   239,   177,    -1,   272,    90,   288,   192,   177,
      -1,   272,    90,   289,   177,    -1,   199,   140,   192,    -1,
     192,    -1,   274,    -1,   273,     8,   274,    -1,   275,   330,
     277,   278,    -1,   146,    -1,   124,    -1,   333,    -1,   112,
      -1,   152,   178,   276,   179,    -1,   339,    -1,   276,     8,
     339,    -1,    13,   340,    -1,    -1,    51,   153,    -1,    -1,
     280,    -1,   279,     8,   280,    -1,   150,    -1,   282,    -1,
     192,    -1,   115,    -1,   175,   283,   176,    -1,   175,   283,
     176,    45,    -1,   175,   283,   176,    25,    -1,   175,   283,
     176,    42,    -1,   282,    -1,   284,    -1,   284,    45,    -1,
     284,    25,    -1,   284,    42,    -1,   283,     8,   283,    -1,
     283,    29,   283,    -1,   192,    -1,   146,    -1,   150,    -1,
     177,    -1,   178,   201,   179,    -1,   177,    -1,   178,   201,
     179,    -1,   289,    -1,   112,    -1,   289,    -1,    -1,   290,
      -1,   289,   290,    -1,   106,    -1,   107,    -1,   108,    -1,
     111,    -1,   110,    -1,   109,    -1,   173,    -1,   292,    -1,
      -1,   106,    -1,   107,    -1,   108,    -1,   293,     8,    73,
      -1,   293,     8,    73,    13,   340,    -1,    73,    -1,    73,
      13,   340,    -1,   294,     8,   394,    13,   340,    -1,    99,
     394,    13,   340,    -1,   175,   295,   176,    -1,    63,   335,
     338,    -1,    62,   305,    -1,   322,    -1,   316,    -1,   175,
     305,   176,    -1,   297,     8,   305,    -1,   305,    -1,   297,
      -1,    -1,   145,   305,    -1,   145,   305,   122,   305,    -1,
     367,    13,   299,    -1,   123,   175,   379,   176,    13,   299,
      -1,   172,   305,    -1,   367,    13,   302,    -1,   123,   175,
     379,   176,    13,   302,    -1,   306,    -1,   367,    -1,   295,
      -1,   123,   175,   379,   176,    13,   305,    -1,   367,    13,
     305,    -1,   367,    13,    31,   367,    -1,   367,    13,    31,
      63,   335,   338,    -1,   367,    24,   305,    -1,   367,    23,
     305,    -1,   367,    22,   305,    -1,   367,    21,   305,    -1,
     367,    20,   305,    -1,   367,    19,   305,    -1,   367,    18,
     305,    -1,   367,    17,   305,    -1,   367,    16,   305,    -1,
     367,    15,   305,    -1,   367,    14,   305,    -1,   367,    60,
      -1,    60,   367,    -1,   367,    59,    -1,    59,   367,    -1,
     305,    27,   305,    -1,   305,    28,   305,    -1,   305,     9,
     305,    -1,   305,    11,   305,    -1,   305,    10,   305,    -1,
     305,    29,   305,    -1,   305,    31,   305,    -1,   305,    30,
     305,    -1,   305,    44,   305,    -1,   305,    42,   305,    -1,
     305,    43,   305,    -1,   305,    45,   305,    -1,   305,    46,
     305,    -1,   305,    47,   305,    -1,   305,    41,   305,    -1,
     305,    40,   305,    -1,    42,   305,    -1,    43,   305,    -1,
      48,   305,    -1,    50,   305,    -1,   305,    33,   305,    -1,
     305,    32,   305,    -1,   305,    35,   305,    -1,   305,    34,
     305,    -1,   305,    36,   305,    -1,   305,    39,   305,    -1,
     305,    37,   305,    -1,   305,    38,   305,    -1,   305,    49,
     335,    -1,   175,   306,   176,    -1,   305,    25,   305,    26,
     305,    -1,   305,    25,    26,   305,    -1,   389,    -1,    58,
     305,    -1,    57,   305,    -1,    56,   305,    -1,    55,   305,
      -1,    54,   305,    -1,    53,   305,    -1,    52,   305,    -1,
      64,   336,    -1,    51,   305,    -1,   342,    -1,   315,    -1,
     314,    -1,   181,   337,   181,    -1,    12,   305,    -1,    -1,
     218,   217,   175,   307,   256,   176,   399,   320,   178,   201,
     179,    -1,    -1,   289,   218,   217,   175,   308,   256,   176,
     399,   320,   178,   201,   179,    -1,   318,    -1,    79,    -1,
     310,     8,   309,   122,   305,    -1,   309,   122,   305,    -1,
     311,     8,   309,   122,   340,    -1,   309,   122,   340,    -1,
     310,   344,    -1,    -1,   311,   344,    -1,    -1,   166,   175,
     312,   176,    -1,   124,   175,   380,   176,    -1,    61,   380,
     182,    -1,   333,   178,   382,   179,    -1,   333,   178,   384,
     179,    -1,   318,    61,   375,   182,    -1,   319,    61,   375,
     182,    -1,   315,    -1,   391,    -1,   175,   306,   176,    -1,
     104,   175,   321,   345,   176,    -1,    -1,   321,     8,    73,
      -1,   321,     8,    31,    73,    -1,    73,    -1,    31,    73,
      -1,   160,   146,   323,   161,    -1,   325,    46,    -1,   325,
     161,   326,   160,    46,   324,    -1,    -1,   146,    -1,   325,
     327,    13,   328,    -1,    -1,   326,   329,    -1,    -1,   146,
      -1,   147,    -1,   178,   305,   179,    -1,   147,    -1,   178,
     305,   179,    -1,   322,    -1,   331,    -1,   330,    26,   331,
      -1,   330,    43,   331,    -1,   192,    -1,    64,    -1,    98,
      -1,    99,    -1,   100,    -1,   145,    -1,   172,    -1,   101,
      -1,   102,    -1,   159,    -1,   103,    -1,    65,    -1,    66,
      -1,    68,    -1,    67,    -1,    82,    -1,    83,    -1,    81,
      -1,    84,    -1,    85,    -1,    86,    -1,    87,    -1,    88,
      -1,    89,    -1,    49,    -1,    90,    -1,    91,    -1,    92,
      -1,    93,    -1,    94,    -1,    95,    -1,    97,    -1,    96,
      -1,    80,    -1,    12,    -1,   117,    -1,   118,    -1,   119,
      -1,   120,    -1,    63,    -1,    62,    -1,   112,    -1,     5,
      -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,   141,
      -1,   104,    -1,   105,    -1,   114,    -1,   115,    -1,   116,
      -1,   111,    -1,   110,    -1,   109,    -1,   108,    -1,   107,
      -1,   106,    -1,   173,    -1,   113,    -1,   123,    -1,   124,
      -1,     9,    -1,    11,    -1,    10,    -1,   125,    -1,   127,
      -1,   126,    -1,   128,    -1,   129,    -1,   143,    -1,   142,
      -1,   171,    -1,   154,    -1,   156,    -1,   155,    -1,   167,
      -1,   169,    -1,   166,    -1,   198,   175,   258,   176,    -1,
     199,    -1,   146,    -1,   333,    -1,   111,    -1,   373,    -1,
     333,    -1,   111,    -1,   377,    -1,   175,   176,    -1,   296,
      -1,    -1,    -1,    78,    -1,   386,    -1,   175,   258,   176,
      -1,    -1,    69,    -1,    70,    -1,    79,    -1,   128,    -1,
     129,    -1,   143,    -1,   125,    -1,   156,    -1,   126,    -1,
     127,    -1,   142,    -1,   171,    -1,   136,    78,   137,    -1,
     136,   137,    -1,   339,    -1,   197,    -1,    42,   340,    -1,
      43,   340,    -1,   124,   175,   343,   176,    -1,   174,   175,
     343,   176,    -1,    61,   343,   182,    -1,   166,   175,   313,
     176,    -1,   341,    -1,   317,    -1,   199,   140,   192,    -1,
     146,   140,   192,    -1,   197,    -1,    72,    -1,   391,    -1,
     339,    -1,   183,   386,   183,    -1,   184,   386,   184,    -1,
     136,   386,   137,    -1,   346,   344,    -1,    -1,     8,    -1,
      -1,     8,    -1,    -1,   346,     8,   340,   122,   340,    -1,
     346,     8,   340,    -1,   340,   122,   340,    -1,   340,    -1,
      69,    -1,    70,    -1,    79,    -1,   136,    78,   137,    -1,
     136,   137,    -1,    69,    -1,    70,    -1,   192,    -1,   347,
      -1,   192,    -1,    42,   348,    -1,    43,   348,    -1,   124,
     175,   350,   176,    -1,   174,   175,   350,   176,    -1,    61,
     350,   182,    -1,   166,   175,   353,   176,    -1,   351,   344,
      -1,    -1,   351,     8,   349,   122,   349,    -1,   351,     8,
     349,    -1,   349,   122,   349,    -1,   349,    -1,   352,     8,
     349,    -1,   349,    -1,   354,   344,    -1,    -1,   354,     8,
     309,   122,   349,    -1,   309,   122,   349,    -1,   352,   344,
      -1,    -1,   175,   355,   176,    -1,    -1,   357,     8,   192,
     356,    -1,   192,   356,    -1,    -1,   359,   357,   344,    -1,
      41,   358,    40,    -1,   360,    -1,    -1,   363,    -1,   121,
     372,    -1,   121,   192,    -1,   121,   178,   305,   179,    -1,
      61,   375,   182,    -1,   178,   305,   179,    -1,   368,   364,
      -1,   175,   295,   176,   364,    -1,   378,   364,    -1,   175,
     295,   176,   364,    -1,   372,    -1,   332,    -1,   370,    -1,
     371,    -1,   365,    -1,   367,   362,    -1,   175,   295,   176,
     362,    -1,   334,   140,   372,    -1,   369,   175,   258,   176,
      -1,   175,   367,   176,    -1,   332,    -1,   370,    -1,   371,
      -1,   365,    -1,   367,   363,    -1,   175,   295,   176,   363,
      -1,   369,   175,   258,   176,    -1,   175,   367,   176,    -1,
     372,    -1,   365,    -1,   175,   367,   176,    -1,   367,   121,
     192,   396,   175,   258,   176,    -1,   367,   121,   372,   175,
     258,   176,    -1,   367,   121,   178,   305,   179,   175,   258,
     176,    -1,   175,   295,   176,   121,   192,   396,   175,   258,
     176,    -1,   175,   295,   176,   121,   372,   175,   258,   176,
      -1,   175,   295,   176,   121,   178,   305,   179,   175,   258,
     176,    -1,   334,   140,   192,   396,   175,   258,   176,    -1,
     334,   140,   372,   175,   258,   176,    -1,   334,   140,   178,
     305,   179,   175,   258,   176,    -1,   373,    -1,   376,   373,
      -1,   373,    61,   375,   182,    -1,   373,   178,   305,   179,
      -1,   374,    -1,    73,    -1,   180,   178,   305,   179,    -1,
     305,    -1,    -1,   180,    -1,   376,   180,    -1,   372,    -1,
     366,    -1,   377,   362,    -1,   175,   295,   176,   362,    -1,
     334,   140,   372,    -1,   175,   367,   176,    -1,    -1,   366,
      -1,   377,   363,    -1,   175,   295,   176,   363,    -1,   175,
     367,   176,    -1,   379,     8,    -1,   379,     8,   367,    -1,
     379,     8,   123,   175,   379,   176,    -1,    -1,   367,    -1,
     123,   175,   379,   176,    -1,   381,   344,    -1,    -1,   381,
       8,   305,   122,   305,    -1,   381,     8,   305,    -1,   305,
     122,   305,    -1,   305,    -1,   381,     8,   305,   122,    31,
     367,    -1,   381,     8,    31,   367,    -1,   305,   122,    31,
     367,    -1,    31,   367,    -1,   383,   344,    -1,    -1,   383,
       8,   305,   122,   305,    -1,   383,     8,   305,    -1,   305,
     122,   305,    -1,   305,    -1,   385,   344,    -1,    -1,   385,
       8,   340,   122,   340,    -1,   385,     8,   340,    -1,   340,
     122,   340,    -1,   340,    -1,   386,   387,    -1,   386,    78,
      -1,   387,    -1,    78,   387,    -1,    73,    -1,    73,    61,
     388,   182,    -1,    73,   121,   192,    -1,   138,   305,   179,
      -1,   138,    72,    61,   305,   182,   179,    -1,   139,   367,
     179,    -1,   192,    -1,    74,    -1,    73,    -1,   114,   175,
     390,   176,    -1,   115,   175,   367,   176,    -1,   115,   175,
     306,   176,    -1,   115,   175,   295,   176,    -1,     7,   305,
      -1,     6,   305,    -1,     5,   175,   305,   176,    -1,     4,
     305,    -1,     3,   305,    -1,   367,    -1,   390,     8,   367,
      -1,   334,   140,   192,    -1,    -1,    90,   405,    -1,   167,
     395,    13,   405,   177,    -1,   169,   395,   392,    13,   405,
     177,    -1,   192,    -1,   405,   192,    -1,   192,    -1,   192,
     162,   400,   163,    -1,   162,   397,   163,    -1,    -1,   405,
      -1,   397,     8,   405,    -1,   397,     8,   157,    -1,   397,
      -1,   157,    -1,    -1,    -1,    26,   405,    -1,   400,     8,
     192,    -1,   192,    -1,   400,     8,   192,    90,   405,    -1,
     192,    90,   405,    -1,    79,   122,   405,    -1,   402,     8,
     401,    -1,   401,    -1,   402,   344,    -1,    -1,   166,   175,
     403,   176,    -1,    25,   405,    -1,    51,   405,    -1,   199,
      -1,   124,    -1,   404,    -1,   124,   162,   405,   163,    -1,
     124,   162,   405,     8,   405,   163,    -1,   146,    -1,   175,
      98,   175,   398,   176,    26,   405,   176,    -1,   175,   397,
       8,   405,   176,    -1,   405,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   701,   701,   701,   710,   712,   715,   716,   717,   718,
     719,   720,   723,   725,   725,   727,   727,   729,   730,   735,
     736,   737,   738,   739,   740,   741,   745,   747,   750,   751,
     752,   753,   758,   759,   763,   764,   766,   769,   775,   782,
     789,   793,   799,   801,   804,   805,   806,   807,   810,   811,
     815,   820,   820,   826,   826,   833,   832,   838,   838,   843,
     844,   845,   846,   847,   848,   849,   850,   851,   852,   853,
     854,   855,   858,   856,   863,   871,   865,   875,   873,   877,
     878,   882,   883,   884,   885,   886,   887,   888,   889,   890,
     898,   898,   903,   909,   913,   913,   921,   922,   926,   927,
     931,   936,   935,   948,   946,   960,   958,   974,   973,   992,
     990,  1009,  1008,  1017,  1015,  1027,  1026,  1038,  1036,  1049,
    1050,  1054,  1057,  1060,  1061,  1062,  1065,  1067,  1070,  1071,
    1074,  1075,  1078,  1079,  1083,  1084,  1089,  1090,  1093,  1094,
    1095,  1099,  1100,  1104,  1105,  1109,  1110,  1114,  1115,  1120,
    1121,  1126,  1127,  1128,  1129,  1132,  1135,  1137,  1140,  1141,
    1145,  1147,  1150,  1153,  1156,  1157,  1160,  1161,  1165,  1167,
    1169,  1170,  1174,  1178,  1182,  1187,  1192,  1197,  1202,  1208,
    1217,  1219,  1221,  1222,  1226,  1229,  1232,  1236,  1240,  1244,
    1248,  1253,  1261,  1263,  1266,  1267,  1268,  1270,  1275,  1276,
    1279,  1280,  1281,  1285,  1286,  1288,  1289,  1293,  1295,  1298,
    1298,  1302,  1301,  1305,  1309,  1307,  1322,  1319,  1332,  1334,
    1336,  1338,  1340,  1344,  1345,  1346,  1349,  1355,  1358,  1364,
    1367,  1372,  1374,  1379,  1384,  1388,  1389,  1395,  1396,  1401,
    1402,  1407,  1408,  1412,  1413,  1417,  1419,  1425,  1430,  1431,
    1433,  1437,  1438,  1439,  1440,  1444,  1445,  1446,  1447,  1448,
    1449,  1451,  1456,  1459,  1460,  1464,  1465,  1469,  1470,  1473,
    1474,  1477,  1478,  1481,  1482,  1486,  1487,  1488,  1489,  1490,
    1491,  1492,  1496,  1497,  1500,  1501,  1502,  1505,  1507,  1509,
    1510,  1513,  1515,  1519,  1520,  1522,  1523,  1524,  1527,  1531,
    1532,  1536,  1537,  1541,  1542,  1546,  1550,  1555,  1559,  1563,
    1568,  1569,  1570,  1573,  1575,  1576,  1577,  1580,  1581,  1582,
    1583,  1584,  1585,  1586,  1587,  1588,  1589,  1590,  1591,  1592,
    1593,  1594,  1595,  1596,  1597,  1598,  1599,  1600,  1601,  1602,
    1603,  1604,  1605,  1606,  1607,  1608,  1609,  1610,  1611,  1612,
    1613,  1614,  1615,  1616,  1617,  1618,  1619,  1620,  1622,  1623,
    1625,  1627,  1628,  1629,  1630,  1631,  1632,  1633,  1634,  1635,
    1636,  1637,  1638,  1639,  1640,  1641,  1642,  1643,  1644,  1646,
    1645,  1658,  1657,  1669,  1673,  1677,  1681,  1687,  1691,  1697,
    1699,  1703,  1705,  1709,  1713,  1714,  1718,  1725,  1732,  1734,
    1739,  1740,  1741,  1745,  1747,  1751,  1752,  1753,  1754,  1758,
    1764,  1773,  1786,  1787,  1790,  1793,  1796,  1797,  1800,  1804,
    1807,  1810,  1817,  1818,  1822,  1823,  1825,  1829,  1830,  1831,
    1832,  1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,  1841,
    1842,  1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,  1851,
    1852,  1853,  1854,  1855,  1856,  1857,  1858,  1859,  1860,  1861,
    1862,  1863,  1864,  1865,  1866,  1867,  1868,  1869,  1870,  1871,
    1872,  1873,  1874,  1875,  1876,  1877,  1878,  1879,  1880,  1881,
    1882,  1883,  1884,  1885,  1886,  1887,  1888,  1889,  1890,  1891,
    1892,  1893,  1894,  1895,  1896,  1897,  1898,  1899,  1900,  1901,
    1902,  1903,  1904,  1905,  1906,  1907,  1908,  1912,  1917,  1918,
    1921,  1922,  1923,  1927,  1928,  1929,  1933,  1934,  1935,  1939,
    1940,  1941,  1944,  1946,  1950,  1951,  1952,  1954,  1955,  1956,
    1957,  1958,  1959,  1960,  1961,  1962,  1963,  1966,  1971,  1972,
    1973,  1974,  1975,  1977,  1979,  1980,  1982,  1983,  1987,  1990,
    1996,  1997,  1998,  1999,  2000,  2001,  2002,  2007,  2009,  2013,
    2014,  2017,  2018,  2022,  2025,  2027,  2029,  2033,  2034,  2035,
    2037,  2040,  2044,  2045,  2046,  2049,  2050,  2051,  2052,  2053,
    2055,  2057,  2058,  2063,  2065,  2068,  2071,  2073,  2075,  2078,
    2080,  2084,  2086,  2089,  2092,  2098,  2100,  2103,  2104,  2109,
    2112,  2116,  2116,  2121,  2124,  2125,  2129,  2130,  2135,  2136,
    2140,  2141,  2145,  2146,  2151,  2153,  2158,  2159,  2160,  2161,
    2162,  2163,  2164,  2166,  2169,  2171,  2175,  2176,  2177,  2178,
    2179,  2181,  2183,  2185,  2189,  2190,  2191,  2195,  2198,  2201,
    2204,  2208,  2212,  2219,  2223,  2227,  2234,  2235,  2240,  2242,
    2243,  2246,  2247,  2250,  2251,  2255,  2256,  2260,  2261,  2262,
    2263,  2265,  2268,  2271,  2272,  2273,  2275,  2277,  2281,  2282,
    2283,  2285,  2286,  2287,  2291,  2293,  2296,  2298,  2299,  2300,
    2301,  2304,  2306,  2307,  2311,  2313,  2316,  2318,  2319,  2320,
    2324,  2326,  2329,  2332,  2334,  2336,  2340,  2341,  2343,  2344,
    2350,  2351,  2353,  2355,  2357,  2359,  2362,  2363,  2364,  2368,
    2369,  2370,  2371,  2372,  2373,  2374,  2375,  2376,  2380,  2381,
    2385,  2393,  2395,  2399,  2402,  2408,  2409,  2415,  2416,  2423,
    2426,  2430,  2433,  2438,  2439,  2440,  2441,  2445,  2446,  2450,
    2452,  2453,  2455,  2459,  2465,  2467,  2471,  2474,  2477,  2485,
    2488,  2491,  2492,  2495,  2496,  2499,  2503,  2507,  2513,  2521,
    2522
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
  "T_COMPILER_HALT_OFFSET", "T_AWAIT", "T_ASYNC", "T_TUPLE", "'('", "')'",
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
  "expr_no_variable", "$@26", "$@27", "shape_keyname",
  "non_empty_shape_pair_list", "non_empty_static_shape_pair_list",
  "shape_pair_list", "static_shape_pair_list", "shape_literal",
  "array_literal", "collection_literal", "static_collection_literal",
  "dim_expr", "dim_expr_base", "lexical_vars", "lexical_var_list",
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
     406,   407,   408,   409,   410,    40,    41,    59,   123,   125,
      36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   185,   187,   186,   188,   188,   189,   189,   189,   189,
     189,   189,   189,   190,   189,   191,   189,   189,   189,   192,
     192,   192,   192,   192,   192,   192,   193,   193,   194,   194,
     194,   194,   195,   195,   196,   196,   196,   197,   198,   199,
     200,   200,   201,   201,   202,   202,   202,   202,   203,   203,
     203,   204,   203,   205,   203,   206,   203,   207,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   208,   203,   203,   209,   203,   210,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     212,   211,   213,   213,   215,   214,   216,   216,   217,   217,
     218,   220,   219,   221,   219,   222,   219,   224,   223,   225,
     223,   226,   223,   227,   223,   229,   228,   230,   228,   231,
     231,   232,   233,   234,   234,   234,   235,   235,   236,   236,
     237,   237,   238,   238,   239,   239,   240,   240,   241,   241,
     241,   242,   242,   243,   243,   244,   244,   245,   245,   246,
     246,   247,   247,   247,   247,   248,   248,   248,   249,   249,
     250,   250,   251,   251,   252,   252,   253,   253,   254,   254,
     254,   254,   255,   255,   255,   255,   255,   255,   255,   255,
     256,   256,   256,   256,   257,   257,   257,   257,   257,   257,
     257,   257,   258,   258,   259,   259,   259,   259,   260,   260,
     261,   261,   261,   262,   262,   262,   262,   263,   263,   265,
     264,   266,   264,   264,   267,   264,   268,   264,   264,   264,
     264,   264,   264,   269,   269,   269,   270,   271,   271,   272,
     272,   273,   273,   274,   274,   275,   275,   275,   275,   276,
     276,   277,   277,   278,   278,   279,   279,   280,   281,   281,
     281,   282,   282,   282,   282,   283,   283,   283,   283,   283,
     283,   283,   284,   284,   284,   285,   285,   286,   286,   287,
     287,   288,   288,   289,   289,   290,   290,   290,   290,   290,
     290,   290,   291,   291,   292,   292,   292,   293,   293,   293,
     293,   294,   294,   295,   295,   295,   295,   295,   296,   297,
     297,   298,   298,   299,   299,   300,   301,   302,   303,   304,
     305,   305,   305,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   306,   306,   306,   306,   306,   306,   306,
     306,   306,   306,   306,   306,   306,   306,   306,   306,   307,
     306,   308,   306,   306,   309,   310,   310,   311,   311,   312,
     312,   313,   313,   314,   315,   315,   316,   317,   318,   318,
     319,   319,   319,   320,   320,   321,   321,   321,   321,   322,
     323,   323,   324,   324,   325,   325,   326,   326,   327,   328,
     328,   329,   329,   329,   330,   330,   330,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   331,   331,   331,
     331,   331,   331,   331,   331,   331,   331,   332,   333,   333,
     334,   334,   334,   335,   335,   335,   336,   336,   336,   337,
     337,   337,   338,   338,   339,   339,   339,   339,   339,   339,
     339,   339,   339,   339,   339,   339,   339,   339,   340,   340,
     340,   340,   340,   340,   340,   340,   340,   340,   341,   341,
     342,   342,   342,   342,   342,   342,   342,   343,   343,   344,
     344,   345,   345,   346,   346,   346,   346,   347,   347,   347,
     347,   347,   348,   348,   348,   349,   349,   349,   349,   349,
     349,   349,   349,   350,   350,   351,   351,   351,   351,   352,
     352,   353,   353,   354,   354,   355,   355,   356,   356,   357,
     357,   359,   358,   360,   361,   361,   362,   362,   363,   363,
     364,   364,   365,   365,   366,   366,   367,   367,   367,   367,
     367,   367,   367,   367,   367,   367,   368,   368,   368,   368,
     368,   368,   368,   368,   369,   369,   369,   370,   370,   370,
     370,   370,   370,   371,   371,   371,   372,   372,   373,   373,
     373,   374,   374,   375,   375,   376,   376,   377,   377,   377,
     377,   377,   377,   378,   378,   378,   378,   378,   379,   379,
     379,   379,   379,   379,   380,   380,   381,   381,   381,   381,
     381,   381,   381,   381,   382,   382,   383,   383,   383,   383,
     384,   384,   385,   385,   385,   385,   386,   386,   386,   386,
     387,   387,   387,   387,   387,   387,   388,   388,   388,   389,
     389,   389,   389,   389,   389,   389,   389,   389,   390,   390,
     391,   392,   392,   393,   393,   394,   394,   395,   395,   396,
     396,   397,   397,   398,   398,   398,   398,   399,   399,   400,
     400,   400,   400,   401,   402,   402,   403,   403,   404,   405,
     405,   405,   405,   405,   405,   405,   405,   405,   405,   406,
     406
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     2,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     1,     2,
       3,     4,     1,     3,     1,     3,     2,     1,     2,     2,
       5,     4,     2,     0,     1,     1,     1,     1,     3,     5,
       8,     0,     4,     0,     6,     0,    10,     0,     4,     2,
       3,     2,     3,     2,     3,     3,     3,     3,     3,     5,
       1,     1,     0,     9,     5,     0,    13,     0,     5,     3,
       3,     2,     2,     2,     2,     2,     2,     3,     2,     2,
       0,     4,     9,     0,     0,     4,     2,     0,     1,     0,
       1,     0,     9,     0,    10,     0,    11,     0,     8,     0,
       9,     0,     7,     0,     8,     0,     7,     0,     8,     1,
       1,     1,     1,     1,     2,     2,     2,     0,     2,     0,
       2,     0,     1,     3,     1,     3,     2,     0,     1,     2,
       4,     1,     4,     1,     4,     1,     4,     1,     4,     3,
       5,     3,     4,     4,     5,     5,     4,     0,     1,     1,
       4,     0,     5,     0,     2,     0,     3,     0,     3,     2,
       1,     0,     4,     5,     7,     6,     6,     7,     9,     8,
       3,     2,     1,     0,     3,     4,     6,     5,     5,     6,
       8,     7,     2,     0,     1,     2,     3,     4,     3,     1,
       1,     2,     4,     3,     5,     1,     3,     2,     0,     0,
       4,     0,     5,     2,     0,    10,     0,    11,     3,     3,
       3,     3,     5,     2,     2,     0,     6,     5,     4,     3,
       1,     1,     3,     4,     1,     1,     1,     1,     4,     1,
       3,     2,     0,     2,     0,     1,     3,     1,     1,     1,
       1,     3,     4,     4,     4,     1,     1,     2,     2,     2,
       3,     3,     1,     1,     1,     1,     3,     1,     3,     1,
       1,     1,     0,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     1,     1,     1,     3,     5,     1,
       3,     5,     4,     3,     3,     2,     1,     1,     3,     3,
       1,     1,     0,     2,     4,     3,     6,     2,     3,     6,
       1,     1,     1,     6,     3,     4,     6,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     4,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     1,     1,     1,     3,     2,     0,
      11,     0,    12,     1,     1,     5,     3,     5,     3,     2,
       0,     2,     0,     4,     4,     3,     4,     4,     4,     4,
       1,     1,     3,     5,     0,     3,     4,     1,     2,     4,
       2,     6,     0,     1,     4,     0,     2,     0,     1,     1,
       3,     1,     3,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       2,     2,     4,     4,     3,     4,     1,     1,     3,     3,
       1,     1,     1,     1,     3,     3,     3,     2,     0,     1,
       0,     1,     0,     5,     3,     3,     1,     1,     1,     1,
       3,     2,     1,     1,     1,     1,     1,     2,     2,     4,
       4,     3,     4,     2,     0,     5,     3,     3,     1,     3,
       1,     2,     0,     5,     3,     2,     0,     3,     0,     4,
       2,     0,     3,     3,     1,     0,     1,     2,     2,     4,
       3,     3,     2,     4,     2,     4,     1,     1,     1,     1,
       1,     2,     4,     3,     4,     3,     1,     1,     1,     1,
       2,     4,     4,     3,     1,     1,     3,     7,     6,     8,
       9,     8,    10,     7,     6,     8,     1,     2,     4,     4,
       1,     1,     4,     1,     0,     1,     2,     1,     1,     2,
       4,     3,     3,     0,     1,     2,     4,     3,     2,     3,
       6,     0,     1,     4,     2,     0,     5,     3,     3,     1,
       6,     4,     4,     2,     2,     0,     5,     3,     3,     1,
       2,     0,     5,     3,     3,     1,     2,     2,     1,     2,
       1,     4,     3,     3,     6,     3,     1,     1,     1,     4,
       4,     4,     4,     2,     2,     4,     2,     2,     1,     3,
       3,     0,     2,     5,     6,     1,     2,     1,     4,     3,
       0,     1,     3,     3,     1,     1,     0,     0,     2,     3,
       1,     5,     3,     3,     3,     1,     2,     0,     4,     2,
       2,     1,     1,     1,     4,     6,     1,     8,     5,     1,
       0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   601,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   675,     0,   663,   518,
       0,   524,   525,    19,   551,   651,    71,   526,     0,    53,
       0,     0,     0,     0,     0,     0,     0,     0,   100,     0,
       0,     0,     0,     0,     0,   275,   276,   277,   280,   279,
     278,     0,     0,     0,     0,   123,     0,     0,     0,   530,
     532,   533,   527,   528,     0,     0,   534,   529,     0,     0,
     509,    20,    21,    22,    24,    23,     0,   531,     0,     0,
       0,     0,   535,     0,   281,    25,     0,    70,    43,   655,
     519,     0,     0,     4,    32,    34,    37,   550,     0,   508,
       0,     6,    99,     7,     8,     9,     0,     0,   273,   312,
       0,     0,     0,     0,     0,     0,     0,   310,   376,   375,
     297,   383,     0,   296,   617,   510,     0,   553,   374,   272,
     620,   311,     0,     0,   618,   619,   616,   646,   650,     0,
     364,   552,    10,   280,   279,   278,     0,     0,    32,    99,
       0,   717,   311,   716,     0,   714,   713,   378,     0,     0,
     348,   349,   350,   351,   373,   371,   370,   369,   368,   367,
     366,   365,   511,     0,   730,   510,     0,   331,   329,     0,
     679,     0,   560,   295,   514,     0,   730,   513,     0,   523,
     658,   657,   515,     0,     0,   517,   372,     0,     0,     0,
     300,     0,    51,   302,     0,     0,    57,    59,     0,     0,
      61,     0,     0,     0,   752,   756,     0,     0,    32,   751,
       0,   753,     0,    63,     0,     0,    90,     0,     0,     0,
       0,    27,    28,   200,     0,     0,   199,   125,   124,   205,
       0,     0,     0,     0,     0,   727,   111,   121,   671,   675,
     700,     0,   537,     0,     0,     0,   698,     0,    15,     0,
      36,     0,   303,   115,   122,   415,   390,     0,   721,   307,
     312,     0,   310,   311,     0,     0,   520,     0,   521,     0,
       0,     0,    89,     0,     0,    39,   193,     0,    18,    98,
       0,   120,   107,   119,   278,    99,   274,    82,    83,    84,
      85,    86,    88,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   663,    81,
     654,   654,   685,     0,     0,     0,     0,     0,   271,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   330,   328,     0,   621,   606,   654,     0,   612,   193,
     654,     0,   656,   647,   671,     0,    99,     0,     0,   603,
     598,   560,     0,     0,     0,     0,   683,     0,   395,   559,
     674,     0,     0,    39,     0,   193,   294,     0,   659,   606,
     614,   516,     0,    43,   161,     0,    68,     0,     0,   301,
       0,     0,     0,     0,     0,    60,    80,    62,   749,   750,
       0,   747,     0,     0,   731,     0,   726,    87,    64,    43,
       0,    77,    79,    29,     0,    17,     0,     0,   201,     0,
      66,     0,     0,    67,   718,     0,     0,   312,   310,   311,
       0,     0,   131,     0,   672,     0,     0,     0,     0,   536,
     699,   551,     0,     0,   697,   556,   696,    35,     5,    12,
      13,    65,     0,   129,     0,     0,   384,     0,   560,     0,
       0,     0,     0,   293,   361,   625,    48,    42,    44,    45,
      46,    47,     0,   377,   554,   555,    33,     0,     0,     0,
     562,   194,     0,   379,   101,   127,     0,   334,   336,   335,
       0,     0,   332,   333,   337,   339,   338,   353,   352,   355,
     354,   356,   358,   359,   357,   347,   346,   341,   342,   340,
     343,   344,   345,   360,   653,     0,     0,   689,     0,   560,
       0,   720,   623,   646,   113,   117,   109,    99,     0,     0,
     305,   308,   314,   327,   326,   325,   324,   323,   322,   321,
     320,   319,   318,   317,     0,   608,   607,     0,     0,     0,
       0,     0,     0,     0,   715,   596,   600,   559,   602,     0,
       0,   730,     0,   678,     0,   677,     0,   662,   661,     0,
       0,   608,   607,   298,   163,   165,   299,     0,    43,   145,
      52,   302,     0,     0,     0,     0,   157,   157,    58,     0,
       0,   745,   560,     0,   736,     0,     0,     0,   558,     0,
       0,   509,     0,    25,    37,   539,   508,   547,     0,   538,
      41,   546,     0,     0,     0,     0,    26,    30,     0,   198,
     206,   203,     0,     0,   709,   712,   711,   710,    11,   740,
       0,     0,     0,   671,   668,     0,   394,   708,   707,   706,
       0,   702,     0,   703,   705,     0,     5,   304,     0,     0,
     409,   410,   418,   417,     0,     0,   559,   389,   393,     0,
     722,     0,     0,   622,   606,   613,   652,     0,   729,   195,
     507,   561,   192,     0,   605,     0,     0,   129,   381,   103,
     363,     0,   398,   399,     0,   396,   559,   684,     0,     0,
     193,   131,   129,   127,     0,   663,   315,     0,     0,   193,
     610,   611,   624,   648,   649,     0,     0,     0,   584,   567,
     568,   569,     0,     0,     0,    25,   576,   575,   590,   560,
       0,   598,   682,   681,     0,   660,   606,   615,   522,     0,
     167,     0,     0,    49,     0,     0,     0,     0,     0,   137,
     138,   149,     0,    43,   147,    74,   157,     0,   157,     0,
       0,   754,     0,   559,   746,   748,   735,   734,     0,   732,
     540,   541,   566,     0,   560,   558,     0,     0,   392,   558,
       0,   691,    91,     0,    94,    78,    31,   202,     0,   719,
      69,     0,     0,   728,   130,   132,   208,     0,     0,   669,
       0,   701,     0,    16,     0,   128,   208,     0,     0,   386,
       0,   723,     0,     0,   608,   607,   732,     0,   196,    40,
     182,     0,   562,   604,   760,   605,   126,     0,   605,     0,
     362,   688,   687,     0,   193,     0,     0,     0,   129,   105,
     523,   609,   193,     0,     0,   572,   573,   574,   577,   578,
     588,     0,   560,   584,     0,   571,   592,   584,   559,   595,
     597,   599,     0,   676,   609,     0,     0,     0,     0,   164,
      54,     0,   302,   139,   671,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   151,     0,   743,   744,     0,     0,
     758,     0,   544,   559,   557,     0,   549,     0,   560,     0,
       0,   548,   695,     0,   560,     0,    43,   204,   742,   739,
       0,   272,   673,   671,   306,   309,   313,     0,    14,   272,
     421,     0,     0,   423,   416,   419,     0,   414,     0,   724,
       0,     0,   193,   197,   737,   605,   181,   759,     0,     0,
     208,     0,   605,     0,   193,     0,   644,   208,   208,     0,
       0,   316,   193,     0,   638,     0,   581,   559,   583,     0,
     570,     0,     0,   560,     0,   589,   680,     0,    43,     0,
     160,   146,     0,     0,   136,    72,   150,     0,     0,   153,
       0,   158,   159,    43,   152,   755,   733,     0,   565,   564,
     542,     0,   559,   391,   545,   543,     0,   397,   559,   690,
       0,     0,     0,   133,     0,     0,   270,     0,     0,     0,
     112,   207,   209,     0,   269,     0,   272,     0,   704,   116,
     412,     0,     0,   385,   609,   193,     0,     0,   404,   180,
     760,     0,   184,   737,   272,   737,     0,   686,     0,   643,
     272,   272,   208,   605,     0,   637,   587,   586,   579,     0,
     582,   559,   591,   580,    43,   166,    50,    55,   140,     0,
     148,   154,    43,   156,     0,     0,   388,     0,   694,   693,
      43,    95,   741,     0,     0,   134,   237,   235,   509,    24,
       0,   231,     0,   236,   247,     0,   245,   250,     0,   249,
       0,   248,     0,    99,   211,     0,   213,     0,   670,   413,
     411,   422,   420,   193,     0,   641,   738,     0,     0,     0,
     185,     0,     0,   108,   404,   737,   645,   114,   118,   272,
       0,   639,     0,   594,     0,   162,     0,    43,   143,    73,
     155,   757,   563,     0,     0,     0,     0,     0,   221,   225,
       0,     0,   218,   473,   472,   469,   471,   470,   490,   492,
     491,   461,   451,   467,   466,   428,   438,   439,   441,   440,
     460,   444,   442,   443,   445,   446,   447,   448,   449,   450,
     452,   453,   454,   455,   456,   457,   459,   458,   429,   430,
     431,   434,   435,   437,   475,   476,   485,   484,   483,   482,
     481,   480,   468,   487,   477,   478,   479,   462,   463,   464,
     465,   488,   489,   493,   495,   494,   496,   497,   474,   499,
     498,   432,   501,   503,   502,   436,   506,   504,   505,   500,
     433,   486,   427,   242,   424,     0,   219,   263,   264,   262,
     255,     0,   256,   220,   289,     0,     0,     0,     0,    99,
       0,   640,     0,    43,     0,   188,     0,   187,   265,    43,
     102,     0,     0,   110,   737,   585,     0,    43,   141,    56,
       0,   387,   692,    93,   292,   135,     0,     0,   239,   232,
       0,     0,     0,   244,   246,     0,     0,   251,   258,   259,
     257,     0,     0,   210,     0,     0,     0,     0,   642,     0,
     407,   562,     0,   189,     0,   186,     0,    43,   104,     0,
     593,     0,     0,    75,   222,    32,     0,   223,   224,     0,
       0,   238,   241,   425,   426,     0,   233,   260,   261,   253,
     254,   252,   290,   287,   214,   212,   291,     0,   408,   561,
       0,   380,     0,   191,   266,     0,   106,     0,   144,     0,
      97,     0,   272,   240,   243,     0,   605,   216,     0,   405,
     403,   190,   382,   142,     0,     0,    76,   229,     0,   271,
     288,   170,     0,   562,   283,   605,   406,     0,    96,     0,
       0,   228,   737,   605,   169,   284,   285,   286,   760,   282,
       0,     0,     0,   227,     0,   168,   283,     0,   737,     0,
     226,   267,    43,   215,   760,     0,   172,     0,    43,     0,
       0,   173,     0,   217,     0,   268,     0,   176,     0,   175,
      92,   177,     0,   174,     0,   179,   178
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   103,   666,   468,   158,   240,   241,
     105,   106,   107,   108,   109,   110,   284,   487,   488,   408,
     211,  1136,   414,  1069,  1350,   634,   237,   429,  1313,   795,
     916,  1366,   300,   159,   489,   695,   839,   960,   490,   505,
     713,   452,   711,   491,   473,   712,   302,   256,   273,   116,
     697,   669,   652,   804,  1084,   886,   759,  1269,  1139,   600,
     765,   413,   608,   767,   993,   595,   750,   753,   877,  1372,
    1373,   831,   832,   499,   500,   245,   246,   250,   921,  1021,
    1102,  1247,  1356,  1375,  1276,  1317,  1318,  1319,  1090,  1091,
    1092,  1277,  1283,  1326,  1095,  1096,  1100,  1240,  1241,  1242,
    1260,  1403,  1022,  1023,   160,   118,  1388,  1389,  1245,  1025,
     119,   205,   409,   410,   120,   121,   122,   123,   124,   125,
     126,   127,   694,   838,   477,   478,   908,   479,   909,   128,
     129,   130,   627,   131,   132,  1118,  1301,   133,   474,  1110,
     475,   817,   674,   937,   934,  1233,  1234,   134,   135,   136,
     199,   206,   287,   396,   137,   782,   631,   138,   783,   390,
     692,   784,   737,   858,   860,   861,   862,   739,   972,   973,
     740,   576,   381,   168,   169,   139,   834,   364,   365,   685,
     140,   200,   162,   142,   143,   144,   145,   146,   147,   148,
     535,   149,   202,   203,   455,   191,   192,   538,   539,   913,
     914,   265,   266,   660,   150,   445,   151,   482,   152,   230,
     257,   295,   423,   778,  1038,   650,   611,   612,   613,   231,
     232,   948
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1154
static const yytype_int16 yypact[] =
{
   -1154,   124, -1154, -1154,  3735,  9567,  9567,   -37,  9567,  9567,
    9567, -1154,  9567,  9567,  9567,  9567,  9567,  9567,  9567,  9567,
    9567,  9567,  9567,  9567,  2967,  2967,  7647,  9567,  3011,    13,
     142, -1154, -1154, -1154, -1154, -1154, -1154, -1154,  9567, -1154,
     142,   149,   152,   158,   142,  7805,   786,  7963, -1154,  1477,
    7331,   -28,  9567,   835,    -3, -1154, -1154, -1154,    55,    61,
      73,   163,   168,   177,   185, -1154,   786,   187,   197, -1154,
   -1154, -1154, -1154, -1154,   373,   507, -1154, -1154,   786,  8121,
   -1154, -1154, -1154, -1154, -1154, -1154,   786, -1154,    51,   204,
     786,   786, -1154,  9567, -1154, -1154,  9567, -1154, -1154,   121,
     475,   552,   552, -1154,   159,   279,   522, -1154,   255, -1154,
      39, -1154,   410, -1154, -1154, -1154,  1208,   247, -1154, -1154,
     304,   314,   326,   328,   352,   353, 10332, -1154, -1154,   416,
   -1154,   437,   438, -1154,    36,   354,   395, -1154, -1154,  1087,
      20,  2792,    99,   365,   103,   105,   367,    30, -1154,   219,
   -1154,   488, -1154, -1154, -1154,   411,   379,   412, -1154,   410,
     247, 11007,  3278, 11007,  9567, 11007, 11007, 11070,   526,   786,
   -1154, -1154,   527, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154,  2162,   413, -1154,   453,   474,   474,  2967,
   10606,   420,   590, -1154,   411,  2162,   413,   463,   466,   434,
     112, -1154,   490,    99,  8279, -1154, -1154,  9567,  6129,    49,
   11007,  7015, -1154,  9567,  9567,   786, -1154, -1154, 10373,   459,
   -1154, 10414,  1477,  1477,   465, -1154,   464,   433,   627, -1154,
     630, -1154,   786, -1154,   489, 10455, -1154,   223, 10496,   786,
      50, -1154,   222, -1154,  2578,    53, -1154, -1154, -1154,   640,
      63,  2967,  2967,  9567,   500,   515, -1154, -1154,  2006,  7647,
      37,   335, -1154,  9725,  2967,   532, -1154,   786, -1154,   -36,
     279,   505, 10708, -1154, -1154, -1154,   604,   673,   598, 11007,
     513, 11007,   529,   503,  3893,  9567,   237,   521,   560,   237,
     300,   322, -1154,   786,  1477,   531,  8437,  1477, -1154, -1154,
     879, -1154, -1154, -1154, -1154,   410, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154,  9567,  9567,  9567,  8619,  9567,  9567,  9567,
    9567,  9567,  9567,  9567,  9567,  9567,  9567,  9567,  9567,  9567,
    9567,  9567,  9567,  9567,  9567,  9567,  9567,  9567,  3011, -1154,
    9567,  9567,  9567,   316,   786,   786,  1208,   609,     7,  7173,
    9567,  9567,  9567,  9567,  9567,  9567,  9567,  9567,  9567,  9567,
    9567, -1154, -1154,   470, -1154,   123,  9567,  9567, -1154,  8437,
    9567,  9567,   121,   126,  2006,   534,   410,  8777,  1694, -1154,
     536,   707,  2162,   540,   -21,   316,   474,  8935, -1154,  9093,
   -1154,   541,   -14, -1154,   232,  8437, -1154,   695, -1154,   129,
   -1154, -1154, 10537, -1154, -1154,  9567, -1154,   636,  6311,   711,
     548, 10900,   714,    57,   100, -1154, -1154, -1154, -1154, -1154,
    1477,   656,   561,   730, -1154, 10550, -1154, -1154, -1154, -1154,
     564, -1154, -1154,   236,   835, -1154,   786,  9567,   474,    -3,
   -1154, 10550,   667, -1154,   474,    60,    77,   565,   567,   908,
     570,   786,   625,   581,   474,    84,   583,  1178,   786, -1154,
   -1154,   696,  2060,   -20, -1154, -1154, -1154,   279, -1154, -1154,
   -1154, -1154,  9567,   643,   599,   283, -1154,   642,   761,   595,
    1477,  1477,   765,   135,   709,   122, -1154, -1154, -1154, -1154,
   -1154, -1154,  2961, -1154, -1154, -1154, -1154,    40,  2967,   605,
     776, 11007,   774, -1154, -1154,   672,  1038, 11047,  3200, 11070,
    9567, 10966,  3372,  2443,  7223,  7381,  7537,  7695,  7695,  7695,
    7695,  1957,  1957,  1957,  1957,   757,   757,   542,   542,   542,
     527,   527,   527, -1154, 11007,   611,   612, 10749,   616,   788,
    9567,   -35,   622,   126, -1154, -1154, -1154,   410,  2179,  9567,
   -1154, -1154, 11070, 11070, 11070, 11070, 11070, 11070, 11070, 11070,
   11070, 11070, 11070, 11070,  9567,   -35,   634,   623,  3241,   635,
     631,  3332,    91,   637, -1154,   911, -1154,   786, -1154,   513,
     135,   413,  2967, 11007,  2967, 10804,   170,   131, -1154,   639,
    9567, -1154, -1154, -1154,  5947,   269, 11007,   142, -1154, -1154,
   -1154,  9567,  1516, 10550,   786,  6493,   641,   644, -1154,    46,
     694, -1154,   812,   649,  1298,  1477, 10550, 10550, 10550,   651,
      32,   687,   654,   655,   308, -1154,   691, -1154,   660, -1154,
   -1154, -1154,  4051,   973,   661,   786, -1154, -1154, 10004, -1154,
   -1154,   820,  2967,   663, -1154, -1154, -1154, -1154, -1154,   752,
      87,   973,   671,  2006,  2794,   832, -1154, -1154, -1154, -1154,
     669, -1154,  9567, -1154, -1154,  3419, -1154, 11007,   973,   674,
   -1154, -1154, -1154, -1154,   840,  9567,   604, -1154, -1154,   677,
   -1154,  1477,   845, -1154,   141, -1154, -1154,  1477, -1154,   474,
   -1154,  9251, -1154, 10550,    25,   680,   973,   643, -1154, -1154,
   11093,  9567, -1154, -1154,  9567, -1154,  9567, -1154, 10045,   681,
    8437,   625,   643,   672,   786,  3011,   474, 10086,   683,  8437,
   -1154, -1154,   145, -1154, -1154,   847,   719,   719,   911, -1154,
   -1154, -1154,   686,    44,   690,   702, -1154, -1154, -1154,   854,
     703,   536,   474,   474,  9409, -1154,   224, -1154, -1154, 10127,
     301,   142,  7015, -1154,   689,  4209,   701,  2967,   705,   759,
     474, -1154,   870, -1154, -1154, -1154, -1154,   452, -1154,   228,
    1477, -1154,  1477,   656, -1154, -1154, -1154,   877,   710,   716,
   -1154, -1154,   768,   715,   891, 10550,   750,   786,   604, 10550,
     786, 10550, -1154,   827, -1154, -1154, -1154, -1154, 10550,   474,
   -1154,  1477,   786, -1154,   894, -1154, -1154,    94,   728,   474,
    7489, -1154,  1191, -1154,  3577,   894, -1154,   319,    -2, 11007,
     782, -1154,   732,  9567,   -35,   736, -1154,  2967, 11007, -1154,
   -1154,   737,   904, -1154,  1477,    25, -1154,   739,    25,   744,
   11093, 11007, 10845,   745,  8437,   738,   758,   762,   643, -1154,
     434,   769,  8437,   771,  9567, -1154, -1154, -1154, -1154, -1154,
     826,   767,   948,   911,   821, -1154,   604,   911,   911, -1154,
   -1154, -1154,  2967, 11007, -1154,   142,   931,   896,  7015, -1154,
   -1154,   785,  9567,   474,  2006,  1516,   783, 10550,  4367,   476,
     792,  9567,   102,   238, -1154,   802, -1154, -1154,  1442,   944,
   -1154, 10550, -1154, 10550, -1154,   809, -1154,   867,   984,   819,
     829, -1154,   880,   822,   998,   836, -1154, -1154, -1154,   918,
     973,  1279, -1154,  2006, -1154, -1154, 11070,   834, -1154,  1673,
   -1154,    48,  9567, -1154, -1154, -1154,  9567, -1154,  9567, -1154,
   10168,   841,  8437,   474,   985,    34, -1154, -1154,    80,   842,
   -1154,   844,    25,  9567,  8437,   846, -1154, -1154, -1154,   839,
     849, -1154,  8437,   858, -1154,   911, -1154,   911, -1154,   869,
   -1154,   921,   873,  1043,   876, -1154,   474,  1020, -1154,   881,
   -1154, -1154,   895,    95, -1154, -1154, -1154,   890,   893, -1154,
   10291, -1154, -1154, -1154, -1154, -1154, -1154,  1477, -1154,   935,
   -1154, 10550,   604, -1154, -1154, -1154, 10550, -1154, 10550, -1154,
     900,  4525,  1477, -1154,  1477,   973, -1154,  1745,   924,   385,
   -1154, -1154, -1154,   609,  2275,    64,     7,    97, -1154, -1154,
     934, 10209, 10250, 11007,   906,  8437,   910,  1477,   978, -1154,
    1477,  1016,  1077,   985,  1839,   985,   915, 11007,   916, -1154,
    2396,  2592, -1154,    25,   923, -1154, -1154,   971, -1154,   911,
   -1154,   604, -1154, -1154, -1154,  5947, -1154, -1154, -1154,  6675,
   -1154, -1154, -1154,  5947,   926, 10550, -1154,   975, -1154,   982,
   -1154, -1154, -1154,  1092,    45, -1154, -1154, -1154,    65,   933,
      66, -1154,  9853, -1154, -1154,    68, -1154, -1154,   825, -1154,
     936, -1154,  1035,   410, -1154,  1477, -1154,   609, -1154, -1154,
   -1154, -1154, -1154,  8437,   939, -1154, -1154,   941,   940,    98,
    1107, 10550,   -45, -1154,   978,   985, -1154, -1154, -1154,  2912,
     947, -1154,   911, -1154,  1005,  5947,  6857, -1154, -1154, -1154,
    5947, -1154, -1154, 10550, 10550,  4683, 10550,   973, -1154, -1154,
    1334,  1745, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154,   321, -1154,   924, -1154, -1154, -1154, -1154,
   -1154,    75,   417, -1154,  1116,    70,   786,  1035,  1117,   410,
     957, -1154,   106, -1154,  1061,  1122, 10550, -1154, -1154, -1154,
   -1154,   959,   -45, -1154,   985, -1154,   911, -1154, -1154, -1154,
    4841, -1154, -1154, -1154, -1154, -1154,   601,    38, -1154, -1154,
   10550,  9853,  9853,  1091, -1154,   825,   825,   450, -1154, -1154,
   -1154, 10550,  1076, -1154,   977,    71, 10550,   786, -1154,  1080,
   -1154,  1146,  4999,  1143, 10550, -1154,  5157, -1154, -1154,   -45,
   -1154,  5315,   981,  1057, -1154,  1070,  1024, -1154, -1154,  1075,
    1334, -1154, -1154, -1154, -1154,  1017, -1154,  1142, -1154, -1154,
   -1154, -1154, -1154,  1159, -1154, -1154, -1154,   999, -1154,   315,
     997, -1154, 10550, -1154, -1154,  5473, -1154,  1000, -1154,  1001,
    1019,   786,     7, -1154, -1154, 10550,    43, -1154,  1106, -1154,
   -1154, -1154, -1154, -1154,   973,   661, -1154,  1025,   786,   930,
   -1154, -1154,  1006,  1176,   539,    43, -1154,  1112, -1154,   973,
    1011, -1154,   985,   127, -1154, -1154, -1154, -1154,  1477, -1154,
    1027,  1030,    72, -1154,   213, -1154,   539,   330,   985,  1014,
   -1154, -1154, -1154, -1154,  1477,  1134,  1195,   213, -1154,  5631,
     414,  1196, 10550, -1154,  5789, -1154,  1137,  1226, 10550, -1154,
   -1154,  1230, 10550, -1154, 10550, -1154, -1154
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1154, -1154, -1154,  -427, -1154, -1154, -1154,    -4, -1154,   810,
     -15,   917,  1775, -1154,  1350, -1154,  -352, -1154,    27, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,  -120,
   -1154, -1154,  -151,     3,     0, -1154, -1154, -1154,     8, -1154,
   -1154, -1154, -1154,     9, -1154, -1154,   901,   902,   903,  1111,
     543,  -576,   546,   586,  -121, -1154,   374, -1154, -1154, -1154,
   -1154, -1154, -1154,  -511,   271, -1154, -1154, -1154, -1154,  -112,
   -1154,  -795, -1154,  -305, -1154, -1154,   828, -1154,  -783, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154, -1154,   113,
   -1154, -1154, -1154, -1154, -1154,    33, -1154,   246,  -797, -1154,
   -1153,  -138, -1154,  -137,    22,  -116,  -126, -1154,    24, -1154,
     -64,   -25,  1236,  -571,  -325, -1154, -1154,   -34, -1154, -1154,
    2504,   -41, -1154, -1154,  -637, -1154, -1154, -1154, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154,   151, -1154,   468, -1154, -1154,
   -1154, -1154, -1154, -1154, -1154, -1154,  -767, -1154,  1259,   119,
    -317, -1154, -1154,   426,   865,  1841, -1154, -1154,  -655,  -353,
    -823, -1154, -1154,   559,  -552,  -728, -1154, -1154, -1154, -1154,
   -1154,   547, -1154, -1154, -1154,  -601,  -920,  -153,  -146,  -115,
   -1154, -1154,    10, -1154, -1154, -1154, -1154,    31,   -99, -1154,
    -218, -1154, -1154, -1154,  -371,  1036, -1154, -1154, -1154, -1154,
   -1154,   594,   443, -1154, -1154,  1037, -1154, -1154, -1154,  -277,
     -80,  -144,  -265, -1154,  -976, -1154,   523, -1154, -1154, -1154,
    -205, -1003
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -731
static const yytype_int16 yytable[] =
{
     104,   306,   347,   572,   113,   208,   274,   112,   375,   946,
     277,   278,   114,   115,   141,   212,   234,   418,   419,   216,
     502,   533,   424,   738,   550,  1040,   117,   368,   578,   497,
     756,   111,   280,   929,   187,   188,   303,  1119,   242,   820,
     949,   665,   219,   951,   306,   228,  1320,   297,   687,   398,
     373,   594,   393,  1147,   770,   282,   399,   405,   434,   201,
     269,   439,   255,   270,   569,   604,    11,  1122,   642,  1124,
     243,   442,  1105,  -234,  1151,    11,  1235,   632,  1292,  1292,
    1147,  -629,   255,  1285,    11,   642,   255,   255,   400,   424,
     589,   370,   654,   833,  1030,   802,   769,  -626,   457,   654,
     363,   363,   654,   654,  1286,   654,   283,   363,   293,  1308,
     786,  1041,   255,    55,    56,    57,   153,   154,   304,   383,
     305,   837,   864,   536,     3,   677,   606,   294,   991,  1254,
     905,   391,  1258,  1259,   910,   969,   847,  1299,   164,   974,
    -730,   469,   470,   186,   186,   935,   249,   198,   567,  1262,
     236,   907,   570,  1042,   506,   485,  1346,  1046,   458,   664,
     366,   348,   587,   376,  -627,   380,  -628,  1044,    11,   262,
    -512,  1255,   247,  -664,  1050,  1051,   936,   244,   248,  1300,
      94,   865,   830,  -633,  -630,   292,   707,   370,   204,   447,
    -665,  1039,  -667,   384,   275,  -635,   366,   275,  -629,   386,
    1371,  -183,  -631,   688,   104,   392,  -632,   104,   371,   771,
    -561,   412,   448,  -511,  -626,   609,   298,  1321,   141,  -171,
     504,   141,  1148,  1149,   433,   573,   406,   435,   426,   971,
     440,   366,   306,   605,   833,   404,   643,   833,   407,   814,
     443,  1106,  -234,  1152,   543,  1236,   755,  1293,  1335,  1400,
     803,  1287,   467,   644,   438,   889,   682,   893,  1130,   774,
     655,   444,   444,   449,   543,   274,   303,   725,   454,  1129,
     922,  1068,   959,  1108,   463,   679,   680,   367,   607,   992,
     104,  -627,   807,  -628,  1395,  -666,   543,   112,  1309,   496,
    -664,   397,    35,   228,   141,   543,   255,  -636,   543,   285,
    -633,  -630,   186,  -561,   371,    35,   117,  -665,   186,  -667,
     260,   982,   436,   367,   186,   551,   975,   207,   579,  -631,
    1026,   891,   892,  -632,   213,   430,   635,   214,  1026,   671,
     683,   891,   892,   215,  1280,   751,   752,   684,   251,   541,
     255,   255,   255,   252,   833,    48,  1358,  1281,   367,   777,
     547,   833,   253,    55,    56,    57,   153,   154,   304,   565,
     254,  1405,   258,   186,  1282,  1077,   293,   875,   876,   201,
     186,   186,   259,   260,   542,   263,   264,   186,   464,   276,
     293,   581,   431,   186,   454,  1397,   869,    33,  1359,    35,
    1401,  1402,   384,   591,   566,   260,   714,   709,   850,   372,
     464,  1410,  -666,  1406,   104,   845,  1394,   894,   260,   424,
     779,   888,    99,  1056,   853,  1057,   542,   994,   141,   242,
      94,   718,  1407,   293,  1134,   588,   699,   683,   592,   672,
     296,   904,   637,   745,   684,   599,  1374,   709,   263,   264,
     746,   299,  1288,  1026,   673,  1416,   260,   649,  -730,  1026,
    1026,   261,   833,   659,   661,  1374,    33,   198,   222,  1289,
     263,   264,  1290,  1396,    81,    82,   930,    83,    84,    85,
     294,   747,   459,   263,   264,  1329,   822,  -400,  1340,   931,
     393,   307,   826,   494,   223,   924,  -730,  1417,  1327,  1328,
      95,   308,  1330,   186,   540,  1331,    99,   932,   340,   341,
    1097,   186,   255,   309,    33,   310,   495,  1133,   689,   968,
     262,   263,   264,   983,  1323,  1324,   377,   350,   351,   352,
     353,   354,   355,   356,   357,   358,   359,   360,  1026,   311,
     312,   422,   342,    81,    82,   343,    83,    84,    85,   955,
     369,    33,  -634,    35,   890,   891,   892,   963,   260,  -401,
    1384,  -511,  1027,   286,   374,  1003,   267,   224,   716,    95,
    1098,  1009,   361,   362,  1011,   895,   379,   896,   988,   891,
     892,   736,   754,   741,   157,   294,   338,    78,    33,   225,
    1265,    81,    82,   543,    83,    84,    85,   335,   336,   337,
     104,   338,   742,   385,   743,   363,   918,   112,   389,   226,
     762,   104,   388,  -510,   141,   260,   394,    95,   227,   395,
     464,   397,   760,   263,   264,   141,   117,   186,    81,    82,
    1062,    83,    84,    85,   363,   260,  1065,   420,   104,   947,
     289,   796,   764,   260,   849,   112,   416,  1036,   464,   421,
    -725,  1073,   141,   425,    95,  1385,  1386,  1387,   564,  1048,
      99,   267,   799,   441,   117,    81,    82,  1054,    83,    84,
      85,   104,  -730,   454,   809,   113,   427,   186,   112,   465,
     263,   264,    33,   114,   115,   141,   450,   451,   824,   485,
     941,    95,   471,   476,   294,   268,   480,   117,   481,   483,
     263,   264,   111,   826,   288,   290,   291,  -730,   263,   264,
    -730,   186,   493,   186,   460,   484,   -38,    48,   466,   503,
     255,   575,  1135,   825,  1310,   577,   580,   586,   597,   405,
    1140,   186,   857,   857,   736,   601,   878,   603,  1145,   460,
    1114,   466,   460,   466,   466,   610,   614,  1083,   615,   633,
     641,   645,   157,   646,   651,    78,   201,   648,   104,    81,
      82,   104,    83,    84,    85,   833,   653,   662,   112,   656,
     670,   186,   141,   668,   675,   141,    33,   883,    35,   676,
    -402,   678,   186,   186,   833,    95,   925,   117,   681,   879,
    1314,   690,   833,   906,   691,  1270,   911,   693,   855,   856,
      33,   696,  1074,   702,   703,   705,   706,   710,   919,   332,
     333,   334,   335,   336,   337,   720,   338,  1082,  1250,   719,
     104,   722,   698,   723,   113,   748,   772,   112,   766,  1104,
     773,   768,   114,   115,   141,   775,   785,   787,  1248,   788,
     789,   790,  1116,   798,   198,   947,   117,   943,   791,   794,
     800,   111,   801,    81,    82,   810,    83,    84,    85,   806,
     977,   811,   816,   818,   821,   835,   844,    33,   852,   736,
     854,   863,   868,   736,   736,   866,   880,    81,    82,    95,
      83,    84,    85,   590,   104,    99,   186,   867,   882,   870,
     884,   885,   976,   887,   104,   898,   899,   459,   141,  1107,
     901,   112,   900,    95,   454,   760,    33,   902,   141,   903,
     915,  1302,   920,   923,   938,   980,    33,  1306,   306,   939,
     117,   942,   945,   944,   956,  1311,    33,   950,    35,   952,
     954,   377,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,   454,    81,    82,   957,    83,    84,    85,
     958,   184,   184,  1024,   962,   196,   186,   964,   965,   966,
      33,  1024,  1246,   726,   727,  1345,   967,   978,   970,   985,
      95,   736,   981,   736,   979,   995,   196,   361,   362,   989,
     997,  1237,   728,    81,    82,  1238,    83,    84,    85,   239,
     729,   730,    33,    81,    82,  1000,    83,    84,    85,  1001,
     731,   186,  1002,    81,    82,  1004,    83,    84,    85,    95,
    1098,  1007,  1006,   186,   186,  1005,  1008,   104,  1012,    95,
     228,  1037,  1010,  1028,   112,  1099,  1035,  1052,  1043,    95,
    1045,   141,  1049,   823,  1053,    99,  1103,    81,    82,   363,
      83,    84,    85,   117,  1055,   732,    55,    56,    57,   153,
     154,   304,   186,  1059,    33,  1058,  1064,   733,   348,  1060,
    1409,  1061,  1063,    95,   503,   736,  1414,  1075,  1066,    81,
      82,   104,    83,    84,    85,   104,  1024,  1070,   112,   104,
    1071,  1067,  1024,  1024,  1094,   141,   112,   734,  1080,   141,
    1109,  1113,  1117,   141,   647,   735,  1115,   117,  1232,  1120,
    1121,  1125,  1126,  1132,  1239,   117,  1138,  1143,  1297,  1131,
     184,   228,  1141,    94,  1144,  1146,   184,  1381,  1244,    33,
    1249,  1150,   184,  1243,   157,  1251,  1252,    78,  1253,    80,
    1256,    81,    82,  1264,    83,    84,    85,  1266,   736,  1291,
    1296,   104,   104,  1298,  1303,  1304,   104,  1307,   112,   196,
     196,   104,  1325,   112,   196,   141,   141,    95,   112,  1333,
     141,  1024,  1334,  1338,  1339,   141,  1342,   117,  1348,  1349,
    -230,   184,   117,  1268,  1351,  1352,  1294,   117,   184,   184,
    1354,  1286,  1355,  1360,  1357,   184,  1364,  1363,  1365,  1376,
    1379,   184,  1382,   947,  1383,  1391,    81,    82,  1393,    83,
      84,    85,  1408,    55,    56,    57,    58,    59,   304,   947,
     313,   314,   315,  1398,    65,   344,  1399,  1411,  1412,  1418,
    1421,   196,    95,   698,   196,  1368,   316,  1337,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,  1422,
     338,   345,   255,  1424,   636,  1378,   544,   546,   545,    33,
     346,   657,   658,   306,   815,   196,   848,   846,  1392,   984,
      94,  1072,   736,  1390,  1279,  1101,   104,   639,  1284,  1413,
    1404,  1295,  1315,   112,   209,  1261,   961,  1232,  1232,    33,
     141,  1239,  1239,   185,   185,   933,   859,   197,   871,   446,
     629,   184,   117,   255,     0,   456,   897,     0,   104,   184,
       0,     0,   104,     0,     0,   112,   629,   104,     0,   112,
       0,     0,   141,     0,   112,     0,   141,     0,     0,     0,
      11,   141,     0,   222,   117,     0,    81,    82,   117,    83,
      84,    85,     0,   117,     0,     0,     0,   196,     0,     0,
       0,   104,   624,     0,     0,     0,     0,  1367,   112,   223,
       0,     0,    95,     0,   301,   141,    81,    82,   624,    83,
      84,    85,     0,     0,  1380,     0,     0,   117,     0,    33,
       0,     0,     0,   927,  1369,     0,     0,     0,  1014,     0,
       0,     0,    95,  1015,     0,    55,    56,    57,   153,   154,
     304,  1016,     0,     0,     0,     0,     0,   196,   196,   229,
       0,     0,     0,    31,    32,   104,     0,     0,     0,     0,
     104,     0,   112,    37,     0,   184,     0,   112,     0,   141,
       0,     0,   224,     0,   141,     0,     0,  1017,  1018,     0,
    1019,   117,     0,     0,     0,     0,   117,     0,     0,   157,
       0,     0,    78,     0,   225,     0,    81,    82,   185,    83,
      84,    85,    94,     0,     0,   776,     0,     0,  1020,    69,
      70,    71,    72,    73,   226,   184,     0,   222,   629,     0,
     620,     0,    95,   227,     0,     0,    76,    77,     0,     0,
       0,   629,   629,   629,     0,     0,     0,     0,     0,     0,
      87,     0,     0,   223,     0,     0,     0,     0,     0,   184,
       0,   184,   222,   185,     0,    92,     0,     0,     0,     0,
     185,   185,     0,    33,     0,     0,     0,   185,     0,   184,
     624,     0,     0,   185,     0,     0,     0,     0,   223,     0,
       0,   196,   196,   624,   624,   624,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   757,    33,     0,
     196,     0,     0,     0,     0,     0,     0,     0,   629,   184,
       0,     0,     0,     0,     0,     0,   224,     0,   196,     0,
     184,   184,   229,   229,     0,     0,     0,   229,     0,     0,
       0,     0,     0,   157,     0,   196,    78,    33,   225,    35,
      81,    82,     0,    83,    84,    85,     0,   197,   196,   996,
       0,   224,     0,     0,   196,     0,     0,     0,   226,     0,
     624,     0,     0,   196,     0,     0,    95,   227,   157,     0,
       0,    78,     0,   225,     0,    81,    82,   182,    83,    84,
      85,     0,   196,   185,     0,     0,     0,     0,     0,   758,
       0,     0,     0,   226,   229,     0,     0,   229,     0,     0,
     629,    95,   227,     0,   629,     0,   629,   157,     0,     0,
      78,     0,    80,   629,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,   184,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   628,     0,     0,   196,     0,   196,
      95,   183,     0,     0,     0,     0,    99,     0,     0,     0,
     628,     0,   624,   313,   314,   315,   624,     0,   624,     0,
       0,     0,     0,     0,    11,   624,     0,     0,   196,   316,
       0,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,     0,   338,   184,     0,     0,     0,     0,     0,
       0,   196,   629,     0,     0,     0,     0,   185,     0,     0,
       0,     0,     0,     0,     0,     0,   629,     0,   629,     0,
     229,     0,  1014,     0,     0,   626,     0,  1015,     0,    55,
      56,    57,   153,   154,   304,  1016,     0,     0,     0,   184,
       0,   626,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   184,   184,     0,   624,     0,     0,   185,     0,     0,
       0,     0,     0,     0,     0,   196,    33,     0,   624,     0,
     624,  1017,  1018,     0,  1019,     0,     0,     0,     0,     0,
     229,   229,     0,     0,     0,     0,     0,   196,     0,     0,
     184,   185,     0,   185,     0,     0,    94,     0,     0,     0,
       0,     0,  1029,     0,     0,     0,     0,  1086,     0,     0,
       0,   185,   628,     0,     0,     0,   629,     0,     0,  1087,
     574,   629,     0,   629,     0,   628,   628,   628,     0,     0,
      11,     0,     0,     0,     0,     0,   157,     0,     0,    78,
       0,  1088,   793,    81,    82,     0,    83,  1089,    85,     0,
       0,   185,     0,     0,     0,     0,     0,     0,     0,     0,
     805,     0,   185,   185,   196,     0,     0,     0,   624,    95,
       0,     0,     0,   624,     0,   624,     0,   805,     0,   196,
       0,   196,   196,     0,   196,     0,     0,     0,  1014,     0,
     629,   196,     0,  1015,     0,    55,    56,    57,   153,   154,
     304,  1016,   628,   626,   196,   836,     0,   196,     0,     0,
       0,     0,     0,     0,   229,   229,   626,   626,   626,     0,
       0,     0,     0,     0,   197,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   629,  1017,  1018,     0,
    1019,     0,   624,  -731,  -731,  -731,  -731,   330,   331,   332,
     333,   334,   335,   336,   337,     0,   338,     0,   629,   629,
       0,   629,    94,     0,     0,  1278,   185,     0,  1123,     0,
       0,     0,   196,     0,     0,     0,     0,     0,     0,     0,
       0,   229,     0,     0,     0,     0,     0,   229,   624,     0,
       0,     0,     0,   626,   628,     0,     0,     0,   628,     0,
     628,     0,     0,     0,     0,     0,     0,   628,     0,     0,
     624,   624,     0,   624,   196,     0,     0,     0,   196,   313,
     314,   315,     0,     0,     0,     0,     0,    33,     0,    35,
       0,     0,     0,     0,     0,   316,   185,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,     0,   338,
       0,     0,     0,     0,     0,     0,     0,   182,     0,     0,
     229,   629,   229,     0,     0,     0,     0,     0,     0,   453,
       0,   185,     0,     0,     0,   626,     0,     0,     0,   626,
       0,   626,     0,   185,   185,   629,   628,   157,   626,     0,
      78,   229,    80,     0,    81,    82,   629,    83,    84,    85,
     628,   629,   628,     0,     0,     0,     0,     0,     0,   629,
       0,     0,     0,   624,     0,     0,     0,     0,     0,  1013,
      95,   183,   185,     0,   229,  1353,    99,     0,     0,     0,
       0,     0,     0,   196,     0,     0,     0,   624,     0,     0,
     625,     0,     0,     0,     0,     0,     0,   629,   624,     0,
       0,     0,     0,   624,     0,     0,   625,     0,     0,     0,
     629,   624,     0,     0,    27,    28,     0,     0,     0,     0,
       0,     0,     0,    33,     0,    35,     0,   626,     0,   663,
       0,     0,   715,     0,     0,     0,     0,     0,   229,     0,
      33,   626,    35,   626,     0,     0,     0,     0,     0,   624,
     628,     0,     0,     0,     0,   628,   630,   628,     0,     0,
       0,     0,   624,   182,  1085,     0,  1093,   629,     0,     0,
       0,   196,   640,   629,     0,     0,     0,   629,     0,   629,
     182,     0,     0,     0,     0,     0,   196,     0,     0,     0,
     222,     0,     0,   157,     0,   196,    78,     0,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,     0,     0,
     157,   196,    88,    78,     0,    80,   223,    81,    82,   624,
      83,    84,    85,     0,   628,   624,    95,   382,     0,   624,
       0,   624,    99,     0,     0,     0,    33,   229,     0,     0,
       0,   626,     0,    95,   183,     0,   626,     0,   626,    99,
       0,     0,   229,     0,   229,     0,     0,     0,     0,     0,
       0,     0,     0,  -271,   229,     0,     0,     0,   625,     0,
     628,    55,    56,    57,   153,   154,   304,   229,     0,     0,
     229,   625,   625,   625,     0,     0,     0,     0,     0,   224,
       0,     0,   628,   628,     0,   628,  1275,     0,     0,     0,
    1093,     0,     0,     0,     0,     0,   157,     0,     0,    78,
       0,   225,     0,    81,    82,   626,    83,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,    11,     0,     0,
       0,   226,     0,     0,   761,     0,     0,     0,    94,    95,
     227,     0,     0,     0,     0,   229,     0,   780,   781,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   625,     0,
       0,   626,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,     0,   338,   626,   626,  1014,   626,     0,     0,     0,
    1015,     0,    55,    56,    57,   153,   154,   304,  1016,   161,
     163,     0,   165,   166,   167,   628,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
     190,   193,     0,     0,   829,     0,     0,     0,     0,   628,
       0,     0,   210,     0,  1017,  1018,     0,  1019,     0,   218,
     628,   221,     0,     0,   235,   628,   238,     0,     0,     0,
     625,     0,     0,   628,   625,     0,   625,     0,     0,    94,
       0,     0,     0,   625,     0,  1127,     0,     0,     0,     0,
       0,     0,     0,   272,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   279,     0,     0,
     281,   628,     0,     0,     0,     0,   626,     0,     0,     0,
       0,     0,     0,     0,   628,     0,     0,     0,     0,     0,
       0,     0,     0,  1377,     0,     0,  1316,     0,     0,     0,
     626,     0,   912,    11,     0,     0,     0,     0,  1085,   917,
       0,   626,     0,     0,     0,     0,   626,     0,     0,    33,
       0,    35,     0,     0,   626,     0,     0,     0,     0,     0,
       0,     0,   625,     0,     0,     0,     0,     0,   378,     0,
       0,   628,     0,     0,     0,     0,   625,   628,   625,     0,
       0,   628,     0,   628,     0,     0,     0,     0,     0,   182,
       0,  1014,   626,     0,     0,     0,  1015,     0,    55,    56,
      57,   153,   154,   304,  1016,   626,     0,     0,   402,     0,
       0,   402,     0,     0,     0,     0,     0,   210,   411,   157,
       0,     0,    78,     0,    80,     0,    81,    82,   986,    83,
      84,    85,     0,     0,     0,     0,     0,     0,   229,     0,
    1017,  1018,   998,  1019,   999,     0,     0,     0,     0,     0,
       0,     0,    95,   183,   229,     0,   437,   281,    99,     0,
       0,     0,   626,   190,     0,    94,     0,   462,   626,     0,
       0,  1128,   626,     0,   626,     0,   625,     0,     0,     0,
       0,   625,     0,   625,     0,     0,     0,     0,     0,   492,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     501,     0,     0,     0,     0,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   507,   508,   509,
     511,   512,   513,   514,   515,   516,   517,   518,   519,   520,
     521,   522,   523,   524,   525,   526,   527,   528,   529,   530,
     531,   532,  1076,     0,   534,   534,   537,  1078,     0,  1079,
     625,   361,   362,   552,   553,   554,   555,   556,   557,   558,
     559,   560,   561,   562,   563,    33,     0,    35,     0,     0,
     534,   568,     0,   501,   534,   571,     0,     0,     0,     0,
       0,   552,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   583,     0,   585,     0,     0,   625,     0,     0,   501,
       0,     0,     0,     0,     0,   182,     0,     0,     0,   596,
       0,     0,     0,   363,     0,     0,  1142,   808,   625,   625,
       0,   625,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   157,     0,     0,    78,     0,
      80,   638,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,    11,     0,     0,     0,     0,     0,     0,
       0,     0,  1257,     0,     0,     0,     0,     0,    95,   183,
     313,   314,   315,     0,    99,     0,   667,     0,     0,     0,
       0,     0,     0,     0,  1271,  1272,   316,  1274,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,     0,
     338,  1014,     0,     0,   700,     0,  1015,     0,    55,    56,
      57,   153,   154,   304,  1016,     0,     0,     0,     0,     0,
       0,   625,     0,     0,     0,     0,     0,     0,    33,     0,
      35,     0,     0,     0,   708,     0,     0,     0,     0,     0,
       0,     0,     0,   272,     0,   625,     0,     0,     0,     0,
    1017,  1018,     0,  1019,     0,     0,   625,     0,   717,     0,
       0,   625,     0,     0,     0,     0,     0,     0,   182,   625,
       0,     0,    33,     0,    35,    94,     0,     0,     0,     0,
       0,  1263,     0,     0,   749,     0,     0,  1305,     0,     0,
       0,     0,     0,     0,     0,   210,     0,     0,   157,     0,
       0,    78,     0,    80,     0,    81,    82,   625,    83,    84,
      85,  1322,   194,     0,     0,     0,     0,     0,     0,     0,
     625,     0,  1332,     0,     0,     0,     0,  1336,     0,     0,
     686,    95,   183,     0,     0,  1343,     0,    99,     0,     0,
       0,     0,   157,     0,     0,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,   812,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   819,
       0,     0,     0,  1361,     0,    95,   195,   625,     0,     0,
       0,    99,     0,   625,     0,   828,  1370,   625,     0,   625,
       0,     0,     0,     0,     0,   840,     0,     0,   841,     0,
     842,   315,     0,     0,   501,     0,     0,     0,     0,     0,
       0,     0,     0,   501,     0,   316,     0,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   873,   338,
     313,   314,   315,  1419,     0,     0,     0,     0,     0,  1423,
       0,     0,     0,  1425,     0,  1426,   316,     0,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,     0,
     338,   377,   350,   351,   352,   353,   354,   355,   356,   357,
     358,   359,   360,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   926,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   940,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   361,   362,     0,
       0,   313,   314,   315,     0,     0,     0,     0,   501,     0,
       0,     0,     0,     0,     0,     0,   501,   316,   926,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
       0,   338,     0,     0,     0,     0,   210,     0,     0,     0,
       0,     0,     0,     0,     0,   990,     0,     0,     0,   363,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     721,   338,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,  1031,     0,     0,     0,
    1032,     0,  1033,     0,     0,     0,   501,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1047,   501,     0,
      11,    12,    13,     0,     0,     0,   501,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,   724,     0,     0,    45,    46,    47,    48,    49,    50,
      51,     0,    52,    53,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,    64,    65,    66,     0,   501,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
      75,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,    90,     0,    91,    10,
      92,    93,    94,    95,    96,     0,    97,    98,   813,    99,
     100,     0,   101,   102,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   501,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,    49,    50,    51,     0,
      52,    53,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,    64,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,    75,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,    90,     0,    91,    10,    92,    93,
      94,    95,    96,     0,    97,    98,   928,    99,   100,     0,
     101,   102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
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
      96,     0,    97,    98,     0,    99,   100,     0,   101,   102,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   157,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,    96,     0,
      97,    98,   486,    99,   100,     0,   101,   102,     0,     0,
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
       0,     0,   157,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,    94,    95,    96,     0,    97,    98,
     792,    99,   100,     0,   101,   102,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,   881,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,     0,    65,    66,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     157,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,    96,     0,    97,    98,     0,    99,
     100,     0,   101,   102,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,   987,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   157,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
      94,    95,    96,     0,    97,    98,     0,    99,   100,     0,
     101,   102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,    58,    59,    60,     0,    61,    62,
      63,     0,    65,    66,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   157,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,    94,    95,
      96,     0,    97,    98,  1081,    99,   100,     0,   101,   102,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   157,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,    96,     0,
      97,    98,  1273,    99,   100,     0,   101,   102,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,  1312,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
      58,    59,    60,     0,    61,    62,    63,     0,    65,    66,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   157,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,    94,    95,    96,     0,    97,    98,
       0,    99,   100,     0,   101,   102,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     157,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,    96,     0,    97,    98,  1341,    99,
     100,     0,   101,   102,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   157,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
      94,    95,    96,     0,    97,    98,  1344,    99,   100,     0,
     101,   102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
    1347,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,    58,    59,    60,     0,    61,    62,
      63,     0,    65,    66,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   157,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,    86,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,    94,    95,
      96,     0,    97,    98,     0,    99,   100,     0,   101,   102,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   157,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,    96,     0,
      97,    98,  1362,    99,   100,     0,   101,   102,     0,     0,
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
       0,     0,   157,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,    86,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,    94,    95,    96,     0,    97,    98,
    1415,    99,   100,     0,   101,   102,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
     157,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,    96,     0,    97,    98,  1420,    99,
     100,     0,   101,   102,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,     0,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   157,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,    93,
      94,    95,    96,     0,    97,    98,     0,    99,   100,     0,
     101,   102,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   403,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,    55,    56,    57,   153,   154,
      60,     0,    61,    62,    63,     0,     0,     0,     0,     0,
       0,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     157,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,    93,    94,    95,    96,     0,    97,    98,     0,    99,
     100,     0,   101,   102,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   598,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,    55,    56,    57,
     153,   154,    60,     0,    61,    62,    63,     0,     0,     0,
       0,     0,     0,     0,    67,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   157,    76,    77,    78,    79,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     0,     0,     0,     0,     0,    89,     0,     0,
       0,     0,    92,    93,    94,    95,    96,     0,    97,    98,
       0,    99,   100,     0,   101,   102,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   763,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,   153,   154,    60,     0,    61,    62,    63,     0,
       0,     0,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   157,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     0,     0,     0,     0,     0,    89,
       0,     0,     0,     0,    92,    93,    94,    95,    96,     0,
      97,    98,     0,    99,   100,     0,   101,   102,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1137,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,   153,   154,    60,     0,    61,    62,
      63,     0,     0,     0,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   157,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,    93,    94,    95,
      96,     0,    97,    98,     0,    99,   100,     0,   101,   102,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1267,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,    55,    56,    57,   153,   154,    60,     0,
      61,    62,    63,     0,     0,     0,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   157,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,    93,
      94,    95,    96,     0,    97,    98,     0,    99,   100,     0,
     101,   102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,    55,    56,    57,   153,   154,    60,     0,    61,    62,
      63,     0,     0,     0,     0,     0,     0,     0,    67,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   157,    76,    77,    78,
      79,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,    93,    94,    95,
      96,     0,    97,    98,     0,    99,   100,     0,   101,   102,
       0,     0,     0,     0,   548,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,    48,   338,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   153,   154,   155,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   156,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   157,    76,    77,    78,   549,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,    96,     0,
       0,     0,     0,    99,   100,     0,   101,   102,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,    48,
     338,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     153,   154,   155,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   156,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   157,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,    93,    94,    95,    96,     0,   233,     0,
       0,    99,   100,     0,   101,   102,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,     0,   338,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   153,   154,
     155,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   156,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     157,    76,    77,    78,   549,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,    96,     0,     0,     0,     0,    99,
     100,     0,   101,   102,     0,     0,     0,     0,   189,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,  -731,  -731,  -731,
    -731,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,     0,   338,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   153,   154,   155,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     156,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   157,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,     0,
      94,    95,    96,     0,     0,     0,     0,    99,   100,     0,
     101,   102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   153,   154,   155,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   156,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   157,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,     0,    94,    95,
      96,     0,   217,     0,     0,    99,   100,     0,   101,   102,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   153,   154,   155,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   156,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   157,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,     0,    94,    95,    96,     0,
     220,     0,     0,    99,   100,     0,   101,   102,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   271,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     153,   154,   155,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   156,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   157,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,     0,    94,    95,    96,     0,     0,     0,
       0,    99,   100,     0,   101,   102,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   153,   154,
     155,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   156,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     157,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,     0,    94,    95,    96,   401,     0,     0,     0,    99,
     100,     0,   101,   102,     0,     0,     0,     0,   498,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   153,   154,   155,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     156,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   157,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,     0,
      94,    95,    96,     0,     0,     0,     0,    99,   100,     0,
     101,   102,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   510,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   153,   154,
     155,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   156,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     157,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,     0,    94,    95,    96,     0,     0,     0,     0,    99,
     100,     0,   101,   102,     0,     0,     0,     0,   548,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   153,   154,   155,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     156,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   157,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,     0,
      94,    95,    96,     0,     0,     0,     0,    99,   100,     0,
     101,   102,     0,     0,     0,     0,   582,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   153,   154,   155,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   156,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   157,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,     0,    94,    95,
      96,     0,     0,     0,     0,    99,   100,     0,   101,   102,
       0,     0,     0,     0,   584,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,   153,   154,   155,     0,     0,    62,    63,     0,
       0,     0,     0,     0,     0,     0,   156,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   157,    76,    77,    78,     0,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,     0,    94,    95,    96,     0,
       0,     0,     0,    99,   100,     0,   101,   102,     0,     0,
       0,     0,   827,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
     153,   154,   155,     0,     0,    62,    63,     0,     0,     0,
       0,     0,     0,     0,   156,    68,    69,    70,    71,    72,
      73,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,   157,    76,    77,    78,     0,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,    87,     0,     0,
       0,    88,     5,     6,     7,     8,     9,    89,     0,     0,
       0,    10,    92,     0,    94,    95,    96,     0,     0,     0,
       0,    99,   100,     0,   101,   102,     0,     0,     0,     0,
     872,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   153,   154,
     155,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   156,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     157,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,     0,    94,    95,    96,     0,     0,     0,     0,    99,
     100,     0,   101,   102,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,   153,   154,   155,     0,
       0,    62,    63,     0,     0,     0,     0,     0,     0,     0,
     156,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,   157,    76,
      77,    78,     0,    80,     0,    81,    82,     0,    83,    84,
      85,     0,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,     0,     0,     0,    10,    92,     0,
      94,    95,    96,     0,     0,     0,     0,    99,   100,     0,
     101,   102,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,   461,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   153,   154,   155,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   156,    68,
      69,    70,    71,    72,    73,     0,  1153,  1154,  1155,  1156,
    1157,    74,  1158,  1159,  1160,  1161,   157,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,     0,     0,     0,     0,
       0,    89,     0,     0,     0,     0,    92,     0,    94,    95,
      96,     0,  1162,     0,     0,    99,   100,     0,   101,   102,
       0,     0,     0,     0,     0,  1163,  1164,  1165,  1166,  1167,
    1168,  1169,     0,     0,    33,     0,     0,     0,     0,     0,
       0,     0,     0,  1170,  1171,  1172,  1173,  1174,  1175,  1176,
    1177,  1178,  1179,  1180,  1181,  1182,  1183,  1184,  1185,  1186,
    1187,  1188,  1189,  1190,  1191,  1192,  1193,  1194,  1195,  1196,
    1197,  1198,  1199,  1200,  1201,  1202,  1203,  1204,  1205,  1206,
    1207,  1208,  1209,  1210,     0,     0,  1211,  1212,  1213,  1214,
    1215,  1216,  1217,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1218,  1219,  1220,     0,  1221,     0,
       0,    81,    82,     0,    83,    84,    85,  1222,  1223,  1224,
       0,     0,  1225,   313,   314,   315,     0,     0,     0,  1226,
    1227,     0,  1228,     0,  1229,  1230,  1231,    95,     0,   316,
       0,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,     0,   338,   313,   314,   315,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     316,     0,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,     0,   338,   313,   314,   315,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   316,     0,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,     0,   338,   313,   314,   315,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   316,     0,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,     0,   338,   313,   314,   315,
       0,     0,     0,   797,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   316,     0,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,     0,   338,   313,   314,
     315,     0,     0,     0,   843,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   316,     0,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   337,     0,   338,   313,
     314,   315,     0,     0,     0,   851,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   316,     0,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,     0,   338,
     313,   314,   315,     0,     0,     0,   874,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   316,   991,   317,   318,
     319,   320,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,     0,
     338,   313,   314,   315,     0,     0,     0,  1034,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   316,     0,   317,
     318,   319,   320,   321,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
       0,   338,   313,   314,   315,     0,     0,     0,  1111,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   316,     0,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,     0,   338,   313,   314,   315,     0,     0,     0,  1112,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   316,
       0,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,     0,   338,   313,   314,   315,     0,   992,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     316,     0,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,     0,   338,   313,   314,   315,     0,   339,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   316,     0,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,     0,   338,   313,   314,   315,     0,
     415,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   316,     0,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,     0,   338,     0,     0,     0,
       0,   417,   616,   617,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   618,     0,     0,     0,   313,   314,   315,     0,    31,
      32,    33,     0,     0,     0,     0,     0,     0,     0,    37,
       0,   316,   428,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,     0,   338,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   432,   619,    69,    70,    71,    72,    73,
       0,     0,     0,     0,     0,     0,   620,     0,     0,     0,
       0,   157,    76,    77,    78,     0,   621,     0,    81,    82,
       0,    83,    84,    85,     0,     0,    87,     0,     0,     0,
       0,     0,     0,   593,     0,     0,   622,   313,   314,   315,
       0,    92,     0,     0,   623,     0,     0,     0,   387,     0,
       0,     0,     0,   316,     0,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,     0,   338,   313,   314,
     315,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   316,     0,   317,   318,   319,   320,
     321,   322,   323,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   337,     0,   338,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   313,   314,   315,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   316,
     472,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,     0,   338,   313,   314,   315,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     316,   704,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,     0,   338,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   313,
     314,   315,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   316,   744,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,     0,   338,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   953,     0,     0,
       0,     0,     0,     0,     0,   313,   314,   315,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     602,   316,   701,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,     0,   338,   313,   314,   315,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   316,     0,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,     0,   338,   314,   315,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   316,     0,   317,   318,   319,   320,   321,   322,
     323,   324,   325,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   316,   338,   317,   318,   319,
     320,   321,   322,   323,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,     0,   338,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,     0,   338
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1154))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-731))

static const yytype_int16 yycheck[] =
{
       4,   117,   139,   374,     4,    30,    86,     4,   159,   832,
      90,    91,     4,     4,     4,    40,    50,   222,   223,    44,
     297,   338,   227,   575,   349,   945,     4,   142,   381,   294,
     601,     4,    96,   816,    24,    25,   116,  1040,    53,   676,
     835,   468,    46,   838,   160,    49,     8,     8,     8,   202,
     149,   403,   196,     8,     8,    96,   202,     8,     8,    28,
      75,     8,    66,    78,   369,     8,    41,  1043,     8,  1045,
      73,     8,     8,     8,     8,    41,     8,   429,     8,     8,
       8,    61,    86,     8,    41,     8,    90,    91,   203,   294,
     395,    61,     8,   694,    46,     8,   607,    61,    61,     8,
     121,   121,     8,     8,    29,     8,    96,   121,   144,  1262,
      78,    31,   116,   106,   107,   108,   109,   110,   111,   183,
     117,   697,    78,   341,     0,   478,    26,   162,    26,    31,
     785,   195,   177,   178,   789,   863,   712,    31,   175,   867,
     175,   177,   178,    24,    25,   147,    73,    28,   366,  1125,
     178,   788,   370,    73,   305,   176,  1309,   952,   121,   179,
      61,   139,   176,   160,    61,   169,    61,   950,    41,   137,
     140,    73,   117,    61,   957,   958,   178,   180,   117,    73,
     173,   137,   157,    61,    61,    26,   539,    61,   175,   253,
      61,   157,    61,   183,   146,   175,    61,   146,   178,   189,
     157,   176,    61,   163,   208,   195,    61,   211,   178,   163,
     176,   215,   253,   140,   178,   420,   177,   179,   208,   176,
     300,   211,   177,   178,   239,   376,   177,   177,   232,   866,
     177,    61,   348,   176,   835,   208,   176,   838,   211,   666,
     177,   177,   177,   177,   343,   177,   598,   177,   177,   177,
     163,   176,   267,   176,   244,   766,   121,   768,  1053,   612,
     176,   251,   252,   253,   363,   345,   346,   176,   258,  1052,
     176,   176,   848,   176,   264,   480,   481,   178,   178,   177,
     284,   178,   653,   178,   157,    61,   385,   284,  1264,   293,
     178,   121,    73,   297,   284,   394,   300,   175,   397,   178,
     178,   178,   183,   176,   178,    73,   284,   178,   189,   178,
      73,   882,    90,   178,   195,   349,   868,   175,   382,   178,
     921,    93,    94,   178,   175,   102,    90,   175,   929,    46,
     483,    93,    94,   175,    13,    66,    67,   483,   175,   343,
     344,   345,   346,   175,   945,    98,    31,    26,   178,   614,
     347,   952,   175,   106,   107,   108,   109,   110,   111,   363,
     175,    31,   175,   244,    43,  1002,   144,    66,    67,   338,
     251,   252,   175,    73,   343,   138,   139,   258,    78,   175,
     144,   385,   159,   264,   374,  1388,   739,    71,    73,    73,
     177,   178,   382,   397,   363,    73,   547,   541,   715,   180,
      78,  1404,   178,    73,   408,   710,  1382,   179,    73,   614,
     615,   763,   180,   965,   719,   967,   385,   179,   408,   434,
     173,   565,  1398,   144,  1061,   394,   506,   580,   397,   146,
     175,   784,   436,   586,   580,   408,  1356,   581,   138,   139,
     586,    31,    25,  1044,   161,    31,    73,   451,   140,  1050,
    1051,    78,  1053,   457,   458,  1375,    71,   338,    25,    42,
     138,   139,    45,  1383,   148,   149,   147,   151,   152,   153,
     162,   586,   137,   138,   139,    25,   681,    61,  1301,   160,
     624,   177,   687,   183,    51,   810,   178,    73,  1285,  1286,
     174,   177,    42,   374,   178,    45,   180,   178,    61,    61,
     115,   382,   506,   177,    71,   177,   184,  1059,   498,   862,
     137,   138,   139,   884,  1281,  1282,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,  1129,   177,
     177,    98,   178,   148,   149,   140,   151,   152,   153,   844,
     175,    71,   175,    73,    92,    93,    94,   852,    73,    61,
    1373,   140,   923,    78,   175,   908,   144,   124,   548,   174,
     175,   914,    59,    60,   916,   770,    40,   772,    92,    93,
      94,   575,   597,   577,   141,   162,    49,   144,    71,   146,
    1132,   148,   149,   682,   151,   152,   153,    45,    46,    47,
     594,    49,   582,   140,   584,   121,   801,   594,     8,   166,
     604,   605,   182,   140,   594,    73,   140,   174,   175,   175,
      78,   121,   602,   138,   139,   605,   594,   498,   148,   149,
     973,   151,   152,   153,   121,    73,   978,   162,   632,   834,
      78,   635,   605,    73,   714,   632,   177,   942,    78,   175,
      13,   993,   632,    13,   174,   106,   107,   108,   178,   954,
     180,   144,   642,    13,   632,   148,   149,   962,   151,   152,
     153,   665,   140,   653,   654,   665,   177,   548,   665,   137,
     138,   139,    71,   665,   665,   665,   176,   162,   682,   176,
     824,   174,   177,    79,   162,   178,    13,   665,    90,   176,
     138,   139,   665,   898,   100,   101,   102,   175,   138,   139,
     178,   582,   181,   584,   261,   176,   175,    98,   265,   175,
     714,   175,  1064,   682,  1266,     8,   176,   176,    82,     8,
    1072,   602,   726,   727,   728,   177,   751,    13,  1080,   286,
    1035,   288,   289,   290,   291,    79,   175,  1014,     8,   175,
      73,   176,   141,   176,   119,   144,   715,   177,   752,   148,
     149,   755,   151,   152,   153,  1356,   175,    61,   755,   176,
     161,   642,   752,   120,   122,   755,    71,   757,    73,     8,
      61,   176,   653,   654,  1375,   174,   810,   755,    13,   752,
     179,   176,  1383,   787,     8,  1137,   790,    13,    69,    70,
      71,   119,   997,   182,   182,   179,     8,   175,   802,    42,
      43,    44,    45,    46,    47,   182,    49,  1012,  1113,   175,
     814,   176,   175,   182,   814,   176,   122,   814,   177,  1024,
       8,   177,   814,   814,   814,   176,   175,   140,  1105,   175,
     175,   140,  1037,    13,   715,  1040,   814,   827,   178,   178,
     177,   814,    90,   148,   149,    13,   151,   152,   153,   178,
     875,   182,   178,    13,   177,   175,   175,    71,   175,   863,
      13,   175,     8,   867,   868,   175,   177,   148,   149,   174,
     151,   152,   153,   178,   878,   180,   757,   175,   177,   176,
     175,   122,   872,    13,   888,     8,   176,   137,   878,  1026,
     122,   888,   176,   174,   884,   885,    71,   182,   888,     8,
      73,  1253,     8,   175,   122,   878,    71,  1259,  1024,   177,
     888,   175,     8,   176,   176,  1267,    71,   178,    73,   175,
     175,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,   923,   148,   149,   178,   151,   152,   153,
     178,    24,    25,   921,   175,    28,   827,   176,   122,   182,
      71,   929,  1103,    42,    43,  1307,     8,    26,   137,   176,
     174,   965,   177,   967,    68,   163,    49,    59,    60,   177,
      26,   146,    61,   148,   149,   150,   151,   152,   153,   144,
      69,    70,    71,   148,   149,   176,   151,   152,   153,   122,
      79,   872,     8,   148,   149,   176,   151,   152,   153,   174,
     175,   179,   122,   884,   885,   176,     8,  1011,    90,   174,
    1014,    26,   176,   179,  1011,  1019,   175,   178,   176,   174,
     176,  1011,   176,   178,   175,   180,  1023,   148,   149,   121,
     151,   152,   153,  1011,   176,   124,   106,   107,   108,   109,
     110,   111,   923,   122,    71,   176,    26,   136,  1026,   176,
    1402,     8,   176,   174,   175,  1059,  1408,   122,   177,   148,
     149,  1065,   151,   152,   153,  1069,  1044,   177,  1065,  1073,
     177,   176,  1050,  1051,   150,  1065,  1073,   166,   178,  1069,
     146,   175,   104,  1073,   176,   174,   176,  1065,  1092,    73,
      13,   176,   176,   122,  1098,  1073,  1069,   122,  1249,   176,
     183,  1105,   176,   173,   122,    13,   189,   177,    73,    71,
    1107,   178,   195,   177,   141,   176,   175,   144,   178,   146,
      13,   148,   149,   176,   151,   152,   153,   122,  1132,    13,
      13,  1135,  1136,   176,    73,    13,  1140,   178,  1135,   222,
     223,  1145,    51,  1140,   227,  1135,  1136,   174,  1145,    73,
    1140,  1129,   175,    73,     8,  1145,    13,  1135,   177,   102,
      90,   244,  1140,  1136,   140,    90,  1246,  1145,   251,   252,
     153,    29,    13,   176,   175,   258,   175,   177,   159,    73,
     155,   264,   176,  1388,     8,    73,   148,   149,   177,   151,
     152,   153,   178,   106,   107,   108,   109,   110,   111,  1404,
       9,    10,    11,   176,   117,   118,   176,    73,    13,    13,
      73,   294,   174,   175,   297,  1352,    25,  1297,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    13,
      49,   154,  1246,    13,   434,  1365,   344,   346,   345,    71,
     139,    73,    74,  1369,   668,   338,   713,   711,  1379,   885,
     173,   990,  1266,  1375,  1151,  1019,  1270,   439,  1235,  1407,
    1396,  1247,  1276,  1270,    38,  1124,   850,  1281,  1282,    71,
    1270,  1285,  1286,    24,    25,   817,   727,    28,   741,   252,
     425,   374,  1270,  1297,    -1,   259,   773,    -1,  1302,   382,
      -1,    -1,  1306,    -1,    -1,  1302,   441,  1311,    -1,  1306,
      -1,    -1,  1302,    -1,  1311,    -1,  1306,    -1,    -1,    -1,
      41,  1311,    -1,    25,  1302,    -1,   148,   149,  1306,   151,
     152,   153,    -1,  1311,    -1,    -1,    -1,   420,    -1,    -1,
      -1,  1345,   425,    -1,    -1,    -1,    -1,  1351,  1345,    51,
      -1,    -1,   174,    -1,   146,  1345,   148,   149,   441,   151,
     152,   153,    -1,    -1,  1368,    -1,    -1,  1345,    -1,    71,
      -1,    -1,    -1,   182,  1352,    -1,    -1,    -1,    99,    -1,
      -1,    -1,   174,   104,    -1,   106,   107,   108,   109,   110,
     111,   112,    -1,    -1,    -1,    -1,    -1,   480,   481,    49,
      -1,    -1,    -1,    69,    70,  1409,    -1,    -1,    -1,    -1,
    1414,    -1,  1409,    79,    -1,   498,    -1,  1414,    -1,  1409,
      -1,    -1,   124,    -1,  1414,    -1,    -1,   148,   149,    -1,
     151,  1409,    -1,    -1,    -1,    -1,  1414,    -1,    -1,   141,
      -1,    -1,   144,    -1,   146,    -1,   148,   149,   189,   151,
     152,   153,   173,    -1,    -1,   157,    -1,    -1,   179,   125,
     126,   127,   128,   129,   166,   548,    -1,    25,   603,    -1,
     136,    -1,   174,   175,    -1,    -1,   142,   143,    -1,    -1,
      -1,   616,   617,   618,    -1,    -1,    -1,    -1,    -1,    -1,
     156,    -1,    -1,    51,    -1,    -1,    -1,    -1,    -1,   582,
      -1,   584,    25,   244,    -1,   171,    -1,    -1,    -1,    -1,
     251,   252,    -1,    71,    -1,    -1,    -1,   258,    -1,   602,
     603,    -1,    -1,   264,    -1,    -1,    -1,    -1,    51,    -1,
      -1,   614,   615,   616,   617,   618,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    71,    -1,
     633,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   693,   642,
      -1,    -1,    -1,    -1,    -1,    -1,   124,    -1,   651,    -1,
     653,   654,   222,   223,    -1,    -1,    -1,   227,    -1,    -1,
      -1,    -1,    -1,   141,    -1,   668,   144,    71,   146,    73,
     148,   149,    -1,   151,   152,   153,    -1,   338,   681,   157,
      -1,   124,    -1,    -1,   687,    -1,    -1,    -1,   166,    -1,
     693,    -1,    -1,   696,    -1,    -1,   174,   175,   141,    -1,
      -1,   144,    -1,   146,    -1,   148,   149,   111,   151,   152,
     153,    -1,   715,   374,    -1,    -1,    -1,    -1,    -1,   123,
      -1,    -1,    -1,   166,   294,    -1,    -1,   297,    -1,    -1,
     785,   174,   175,    -1,   789,    -1,   791,   141,    -1,    -1,
     144,    -1,   146,   798,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,    -1,   757,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   425,    -1,    -1,   770,    -1,   772,
     174,   175,    -1,    -1,    -1,    -1,   180,    -1,    -1,    -1,
     441,    -1,   785,     9,    10,    11,   789,    -1,   791,    -1,
      -1,    -1,    -1,    -1,    41,   798,    -1,    -1,   801,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,   827,    -1,    -1,    -1,    -1,    -1,
      -1,   834,   887,    -1,    -1,    -1,    -1,   498,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   901,    -1,   903,    -1,
     420,    -1,    99,    -1,    -1,   425,    -1,   104,    -1,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,   872,
      -1,   441,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   884,   885,    -1,   887,    -1,    -1,   548,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   898,    71,    -1,   901,    -1,
     903,   148,   149,    -1,   151,    -1,    -1,    -1,    -1,    -1,
     480,   481,    -1,    -1,    -1,    -1,    -1,   920,    -1,    -1,
     923,   582,    -1,   584,    -1,    -1,   173,    -1,    -1,    -1,
      -1,    -1,   179,    -1,    -1,    -1,    -1,   112,    -1,    -1,
      -1,   602,   603,    -1,    -1,    -1,  1001,    -1,    -1,   124,
     176,  1006,    -1,  1008,    -1,   616,   617,   618,    -1,    -1,
      41,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,   144,
      -1,   146,   633,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   642,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     651,    -1,   653,   654,   997,    -1,    -1,    -1,  1001,   174,
      -1,    -1,    -1,  1006,    -1,  1008,    -1,   668,    -1,  1012,
      -1,  1014,  1015,    -1,  1017,    -1,    -1,    -1,    99,    -1,
    1075,  1024,    -1,   104,    -1,   106,   107,   108,   109,   110,
     111,   112,   693,   603,  1037,   696,    -1,  1040,    -1,    -1,
      -1,    -1,    -1,    -1,   614,   615,   616,   617,   618,    -1,
      -1,    -1,    -1,    -1,   715,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1121,   148,   149,    -1,
     151,    -1,  1075,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,  1143,  1144,
      -1,  1146,   173,    -1,    -1,  1150,   757,    -1,   179,    -1,
      -1,    -1,  1105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   681,    -1,    -1,    -1,    -1,    -1,   687,  1121,    -1,
      -1,    -1,    -1,   693,   785,    -1,    -1,    -1,   789,    -1,
     791,    -1,    -1,    -1,    -1,    -1,    -1,   798,    -1,    -1,
    1143,  1144,    -1,  1146,  1147,    -1,    -1,    -1,  1151,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,
      -1,    -1,    -1,    -1,    -1,    25,   827,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
     770,  1256,   772,    -1,    -1,    -1,    -1,    -1,    -1,   123,
      -1,   872,    -1,    -1,    -1,   785,    -1,    -1,    -1,   789,
      -1,   791,    -1,   884,   885,  1280,   887,   141,   798,    -1,
     144,   801,   146,    -1,   148,   149,  1291,   151,   152,   153,
     901,  1296,   903,    -1,    -1,    -1,    -1,    -1,    -1,  1304,
      -1,    -1,    -1,  1256,    -1,    -1,    -1,    -1,    -1,   920,
     174,   175,   923,    -1,   834,  1320,   180,    -1,    -1,    -1,
      -1,    -1,    -1,  1276,    -1,    -1,    -1,  1280,    -1,    -1,
     425,    -1,    -1,    -1,    -1,    -1,    -1,  1342,  1291,    -1,
      -1,    -1,    -1,  1296,    -1,    -1,   441,    -1,    -1,    -1,
    1355,  1304,    -1,    -1,    62,    63,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    73,    -1,   887,    -1,   179,
      -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,   898,    -1,
      71,   901,    73,   903,    -1,    -1,    -1,    -1,    -1,  1342,
    1001,    -1,    -1,    -1,    -1,  1006,   425,  1008,    -1,    -1,
      -1,    -1,  1355,   111,  1015,    -1,  1017,  1412,    -1,    -1,
      -1,  1364,   441,  1418,    -1,    -1,    -1,  1422,    -1,  1424,
     111,    -1,    -1,    -1,    -1,    -1,  1379,    -1,    -1,    -1,
      25,    -1,    -1,   141,    -1,  1388,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,
     141,  1404,   160,   144,    -1,   146,    51,   148,   149,  1412,
     151,   152,   153,    -1,  1075,  1418,   174,   175,    -1,  1422,
      -1,  1424,   180,    -1,    -1,    -1,    71,   997,    -1,    -1,
      -1,  1001,    -1,   174,   175,    -1,  1006,    -1,  1008,   180,
      -1,    -1,  1012,    -1,  1014,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,  1024,    -1,    -1,    -1,   603,    -1,
    1121,   106,   107,   108,   109,   110,   111,  1037,    -1,    -1,
    1040,   616,   617,   618,    -1,    -1,    -1,    -1,    -1,   124,
      -1,    -1,  1143,  1144,    -1,  1146,  1147,    -1,    -1,    -1,
    1151,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,   144,
      -1,   146,    -1,   148,   149,  1075,   151,   152,   153,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,    -1,
      -1,   166,    -1,    -1,   603,    -1,    -1,    -1,   173,   174,
     175,    -1,    -1,    -1,    -1,  1105,    -1,   616,   617,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   693,    -1,
      -1,  1121,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,  1143,  1144,    99,  1146,    -1,    -1,    -1,
     104,    -1,   106,   107,   108,   109,   110,   111,   112,     5,
       6,    -1,     8,     9,    10,  1256,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    -1,    -1,   693,    -1,    -1,    -1,    -1,  1280,
      -1,    -1,    38,    -1,   148,   149,    -1,   151,    -1,    45,
    1291,    47,    -1,    -1,    50,  1296,    52,    -1,    -1,    -1,
     785,    -1,    -1,  1304,   789,    -1,   791,    -1,    -1,   173,
      -1,    -1,    -1,   798,    -1,   179,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    93,    -1,    -1,
      96,  1342,    -1,    -1,    -1,    -1,  1256,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1355,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1364,    -1,    -1,  1276,    -1,    -1,    -1,
    1280,    -1,   791,    41,    -1,    -1,    -1,    -1,  1379,   798,
      -1,  1291,    -1,    -1,    -1,    -1,  1296,    -1,    -1,    71,
      -1,    73,    -1,    -1,  1304,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   887,    -1,    -1,    -1,    -1,    -1,   164,    -1,
      -1,  1412,    -1,    -1,    -1,    -1,   901,  1418,   903,    -1,
      -1,  1422,    -1,  1424,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    99,  1342,    -1,    -1,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,  1355,    -1,    -1,   204,    -1,
      -1,   207,    -1,    -1,    -1,    -1,    -1,   213,   214,   141,
      -1,    -1,   144,    -1,   146,    -1,   148,   149,   887,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,  1388,    -1,
     148,   149,   901,   151,   903,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   174,   175,  1404,    -1,   178,   253,   180,    -1,
      -1,    -1,  1412,   259,    -1,   173,    -1,   263,  1418,    -1,
      -1,   179,  1422,    -1,  1424,    -1,  1001,    -1,    -1,    -1,
      -1,  1006,    -1,  1008,    -1,    -1,    -1,    -1,    -1,   285,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     296,    -1,    -1,    -1,    -1,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,  1001,    -1,   340,   341,   342,  1006,    -1,  1008,
    1075,    59,    60,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,    71,    -1,    73,    -1,    -1,
     366,   367,    -1,   369,   370,   371,    -1,    -1,    -1,    -1,
      -1,   377,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   387,    -1,   389,    -1,    -1,  1121,    -1,    -1,   395,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,   405,
      -1,    -1,    -1,   121,    -1,    -1,  1075,   123,  1143,  1144,
      -1,  1146,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,   144,    -1,
     146,   437,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      -1,    -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1121,    -1,    -1,    -1,    -1,    -1,   174,   175,
       9,    10,    11,    -1,   180,    -1,   472,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1143,  1144,    25,  1146,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    99,    -1,    -1,   510,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,
      -1,  1256,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,
      73,    -1,    -1,    -1,   540,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   549,    -1,  1280,    -1,    -1,    -1,    -1,
     148,   149,    -1,   151,    -1,    -1,  1291,    -1,   564,    -1,
      -1,  1296,    -1,    -1,    -1,    -1,    -1,    -1,   111,  1304,
      -1,    -1,    71,    -1,    73,   173,    -1,    -1,    -1,    -1,
      -1,   179,    -1,    -1,   590,    -1,    -1,  1256,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   601,    -1,    -1,   141,    -1,
      -1,   144,    -1,   146,    -1,   148,   149,  1342,   151,   152,
     153,  1280,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1355,    -1,  1291,    -1,    -1,    -1,    -1,  1296,    -1,    -1,
     179,   174,   175,    -1,    -1,  1304,    -1,   180,    -1,    -1,
      -1,    -1,   141,    -1,    -1,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,   662,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   675,
      -1,    -1,    -1,  1342,    -1,   174,   175,  1412,    -1,    -1,
      -1,   180,    -1,  1418,    -1,   691,  1355,  1422,    -1,  1424,
      -1,    -1,    -1,    -1,    -1,   701,    -1,    -1,   704,    -1,
     706,    11,    -1,    -1,   710,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   719,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   744,    49,
       9,    10,    11,  1412,    -1,    -1,    -1,    -1,    -1,  1418,
      -1,    -1,    -1,  1422,    -1,  1424,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   810,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   823,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,   844,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   852,    25,   854,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    -1,    -1,   882,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   891,    -1,    -1,    -1,   121,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     179,    49,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,   932,    -1,    -1,    -1,
     936,    -1,   938,    -1,    -1,    -1,   942,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   953,   954,    -1,
      41,    42,    43,    -1,    -1,    -1,   962,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,   179,    -1,    -1,    95,    96,    97,    98,    99,   100,
     101,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,   116,   117,   118,    -1,  1035,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,   167,    -1,   169,    12,
     171,   172,   173,   174,   175,    -1,   177,   178,   179,   180,
     181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1113,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,   167,    -1,   169,    12,   171,   172,
     173,   174,   175,    -1,   177,   178,   179,   180,   181,    -1,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
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
     175,    -1,   177,   178,    -1,   180,   181,    -1,   183,   184,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,   172,   173,   174,   175,    -1,
     177,   178,   179,   180,   181,    -1,   183,   184,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
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
      -1,    12,   171,   172,   173,   174,   175,    -1,   177,   178,
     179,   180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     171,   172,   173,   174,   175,    -1,   177,   178,    -1,   180,
     181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
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
     173,   174,   175,    -1,   177,   178,    -1,   180,   181,    -1,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
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
     175,    -1,   177,   178,   179,   180,   181,    -1,   183,   184,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,   172,   173,   174,   175,    -1,
     177,   178,   179,   180,   181,    -1,   183,   184,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
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
      -1,    12,   171,   172,   173,   174,   175,    -1,   177,   178,
      -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     171,   172,   173,   174,   175,    -1,   177,   178,   179,   180,
     181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
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
     173,   174,   175,    -1,   177,   178,   179,   180,   181,    -1,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
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
     175,    -1,   177,   178,    -1,   180,   181,    -1,   183,   184,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,   172,   173,   174,   175,    -1,
     177,   178,   179,   180,   181,    -1,   183,   184,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
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
      -1,    12,   171,   172,   173,   174,   175,    -1,   177,   178,
     179,   180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     171,   172,   173,   174,   175,    -1,   177,   178,   179,   180,
     181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
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
     173,   174,   175,    -1,   177,   178,    -1,   180,   181,    -1,
     183,   184,     3,     4,     5,     6,     7,    -1,    -1,    -1,
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
     171,   172,   173,   174,   175,    -1,   177,   178,    -1,   180,
     181,    -1,   183,   184,     3,     4,     5,     6,     7,    -1,
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
      -1,    -1,   171,   172,   173,   174,   175,    -1,   177,   178,
      -1,   180,   181,    -1,   183,   184,     3,     4,     5,     6,
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
      -1,    -1,    -1,    -1,   171,   172,   173,   174,   175,    -1,
     177,   178,    -1,   180,   181,    -1,   183,   184,     3,     4,
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
     175,    -1,   177,   178,    -1,   180,   181,    -1,   183,   184,
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
     173,   174,   175,    -1,   177,   178,    -1,   180,   181,    -1,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,   173,   174,
     175,    -1,   177,   178,    -1,   180,   181,    -1,   183,   184,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    98,    49,    -1,    -1,    -1,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    98,
      49,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,   173,   174,   175,    -1,   177,    -1,
      -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,   108,   109,   110,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,    -1,
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,    -1,   173,   174,
     175,    -1,   177,    -1,    -1,   180,   181,    -1,   183,   184,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,    -1,   173,   174,   175,    -1,
     177,    -1,    -1,   180,   181,    -1,   183,   184,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,   108,
     109,   110,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,    -1,   173,   174,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     171,    -1,   173,   174,   175,   176,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,    -1,    -1,    -1,    -1,    31,    -1,
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
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     171,    -1,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,    -1,    -1,    -1,    -1,    31,    -1,
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
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
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
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,    -1,   173,   174,
     175,    -1,    -1,    -1,    -1,   180,   181,    -1,   183,   184,
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
      -1,    -1,    -1,    12,   171,    -1,   173,   174,   175,    -1,
      -1,    -1,    -1,   180,   181,    -1,   183,   184,    -1,    -1,
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
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,    -1,   173,   174,   175,    -1,    -1,    -1,
      -1,   180,   181,    -1,   183,   184,    -1,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,    -1,   173,   174,   175,    -1,    -1,    -1,    -1,   180,
     181,    -1,   183,   184,    -1,    -1,    -1,    -1,    -1,    -1,
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
     173,   174,   175,    -1,    -1,    -1,    -1,   180,   181,    -1,
     183,   184,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106,   107,   108,   109,   110,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,     3,     4,     5,     6,
       7,   136,     9,    10,    11,    12,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,    -1,    -1,   171,    -1,   173,   174,
     175,    -1,    49,    -1,    -1,   180,   181,    -1,   183,   184,
      -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,    66,
      67,    68,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,    -1,   145,    -1,
      -1,   148,   149,    -1,   151,   152,   153,   154,   155,   156,
      -1,    -1,   159,     9,    10,    11,    -1,    -1,    -1,   166,
     167,    -1,   169,    -1,   171,   172,   173,   174,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,   179,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,   179,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,   179,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,   179,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,   179,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,   177,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,   177,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
     177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,   177,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    61,    -1,    -1,    -1,     9,    10,    11,    -1,    69,
      70,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    25,   177,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   177,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
      -1,    -1,    -1,   176,    -1,    -1,   166,     9,    10,    11,
      -1,   171,    -1,    -1,   174,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     122,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   122,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   122,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      90,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    25,    49,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   186,   187,     0,   188,     3,     4,     5,     6,     7,
      12,    41,    42,    43,    48,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    69,    70,    71,    72,    73,    75,    79,    80,    81,
      82,    84,    86,    88,    91,    95,    96,    97,    98,    99,
     100,   101,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   113,   114,   115,   116,   117,   118,   123,   124,   125,
     126,   127,   128,   129,   136,   141,   142,   143,   144,   145,
     146,   148,   149,   151,   152,   153,   154,   156,   160,   166,
     167,   169,   171,   172,   173,   174,   175,   177,   178,   180,
     181,   183,   184,   189,   192,   195,   196,   197,   198,   199,
     200,   203,   218,   219,   223,   228,   234,   289,   290,   295,
     299,   300,   301,   302,   303,   304,   305,   306,   314,   315,
     316,   318,   319,   322,   332,   333,   334,   339,   342,   360,
     365,   367,   368,   369,   370,   371,   372,   373,   374,   376,
     389,   391,   393,   109,   110,   111,   123,   141,   192,   218,
     289,   305,   367,   305,   175,   305,   305,   305,   358,   359,
     305,   305,   305,   305,   305,   305,   305,   305,   305,   305,
     305,   305,   111,   175,   196,   333,   334,   367,   367,    31,
     305,   380,   381,   305,   111,   175,   196,   333,   334,   335,
     366,   372,   377,   378,   175,   296,   336,   175,   296,   297,
     305,   205,   296,   175,   175,   175,   296,   177,   305,   192,
     177,   305,    25,    51,   124,   146,   166,   175,   192,   199,
     394,   404,   405,   177,   302,   305,   178,   211,   305,   144,
     193,   194,   195,    73,   180,   260,   261,   117,   117,    73,
     262,   175,   175,   175,   175,   192,   232,   395,   175,   175,
      73,    78,   137,   138,   139,   386,   387,   144,   178,   195,
     195,    95,   305,   233,   395,   146,   175,   395,   395,   305,
     295,   305,   306,   367,   201,   178,    78,   337,   386,    78,
     386,   386,    26,   144,   162,   396,   175,     8,   177,    31,
     217,   146,   231,   395,   111,   218,   290,   177,   177,   177,
     177,   177,   177,     9,    10,    11,    25,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    49,   177,
      61,    61,   178,   140,   118,   154,   234,   288,   289,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    59,    60,   121,   362,   363,    61,   178,   364,   175,
      61,   178,   180,   373,   175,   217,   218,    13,   305,    40,
     192,   357,   175,   295,   367,   140,   367,   122,   182,     8,
     344,   295,   367,   396,   140,   175,   338,   121,   362,   363,
     364,   176,   305,    26,   203,     8,   177,   203,   204,   297,
     298,   305,   192,   246,   207,   177,   177,   177,   405,   405,
     162,   175,    98,   397,   405,    13,   192,   177,   177,   212,
     102,   159,   177,   195,     8,   177,    90,   178,   367,     8,
     177,    13,     8,   177,   367,   390,   390,   295,   306,   367,
     176,   162,   226,   123,   367,   379,   380,    61,   121,   137,
     387,    72,   305,   367,    78,   137,   387,   195,   191,   177,
     178,   177,   122,   229,   323,   325,    79,   309,   310,   312,
      13,    90,   392,   176,   176,   176,   179,   202,   203,   219,
     223,   228,   305,   181,   183,   184,   192,   397,    31,   258,
     259,   305,   394,   175,   395,   224,   217,   305,   305,   305,
      26,   305,   305,   305,   305,   305,   305,   305,   305,   305,
     305,   305,   305,   305,   305,   305,   305,   305,   305,   305,
     305,   305,   305,   335,   305,   375,   375,   305,   382,   383,
     178,   192,   372,   373,   232,   233,   231,   218,    31,   145,
     299,   302,   305,   305,   305,   305,   305,   305,   305,   305,
     305,   305,   305,   305,   178,   192,   372,   375,   305,   258,
     375,   305,   379,   217,   176,   175,   356,     8,   344,   295,
     176,   192,    31,   305,    31,   305,   176,   176,   372,   258,
     178,   192,   372,   176,   201,   250,   305,    82,    26,   203,
     244,   177,    90,    13,     8,   176,    26,   178,   247,   405,
      79,   401,   402,   403,   175,     8,    42,    43,    61,   124,
     136,   146,   166,   174,   196,   197,   199,   317,   333,   339,
     340,   341,   201,   175,   210,    90,   194,   192,   305,   261,
     340,    73,     8,   176,   176,   176,   176,   176,   177,   192,
     400,   119,   237,   175,     8,   176,   176,    73,    74,   192,
     388,   192,    61,   179,   179,   188,   190,   305,   120,   236,
     161,    46,   146,   161,   327,   122,     8,   344,   176,   405,
     405,    13,   121,   362,   363,   364,   179,     8,   163,   367,
     176,     8,   345,    13,   307,   220,   119,   235,   175,   395,
     305,    26,   182,   182,   122,   179,     8,   344,   305,   396,
     175,   227,   230,   225,   217,    63,   367,   305,   396,   175,
     182,   179,   176,   182,   179,   176,    42,    43,    61,    69,
      70,    79,   124,   136,   166,   174,   192,   347,   349,   352,
     355,   192,   367,   367,   122,   362,   363,   364,   176,   305,
     251,    66,    67,   252,   296,   201,   298,    31,   123,   241,
     367,   340,   192,    26,   203,   245,   177,   248,   177,   248,
       8,   163,   122,     8,   344,   176,   157,   397,   398,   405,
     340,   340,   340,   343,   346,   175,    78,   140,   175,   175,
     140,   178,   179,   333,   178,   214,   192,   179,    13,   367,
     177,    90,     8,   163,   238,   333,   178,   379,   123,   367,
      13,   182,   305,   179,   188,   238,   178,   326,    13,   305,
     309,   177,   405,   178,   192,   372,   405,    31,   305,   340,
     157,   256,   257,   360,   361,   175,   333,   236,   308,   221,
     305,   305,   305,   179,   175,   258,   237,   236,   235,   395,
     335,   179,   175,   258,    13,    69,    70,   192,   348,   348,
     349,   350,   351,   175,    78,   137,   175,   175,     8,   344,
     176,   356,    31,   305,   179,    66,    67,   253,   296,   203,
     177,    83,   177,   367,   175,   122,   240,    13,   201,   248,
      92,    93,    94,   248,   179,   405,   405,   401,     8,   176,
     176,   122,   182,     8,   344,   343,   192,   309,   311,   313,
     343,   192,   340,   384,   385,    73,   215,   340,   405,   192,
       8,   263,   176,   175,   299,   302,   305,   182,   179,   263,
     147,   160,   178,   322,   329,   147,   178,   328,   122,   177,
     305,   396,   175,   367,   176,     8,   345,   405,   406,   256,
     178,   256,   175,   122,   175,   258,   176,   178,   178,   236,
     222,   338,   175,   258,   176,   122,   182,     8,   344,   350,
     137,   309,   353,   354,   350,   349,   367,   296,    26,    68,
     203,   177,   298,   379,   241,   176,   340,    89,    92,   177,
     305,    26,   177,   249,   179,   163,   157,    26,   340,   340,
     176,   122,     8,   344,   176,   176,   122,   179,     8,   344,
     176,   201,    90,   333,    99,   104,   112,   148,   149,   151,
     179,   264,   287,   288,   289,   294,   360,   379,   179,   179,
      46,   305,   305,   305,   179,   175,   258,    26,   399,   157,
     361,    31,    73,   176,   263,   176,   256,   305,   258,   176,
     263,   263,   178,   175,   258,   176,   349,   349,   176,   122,
     176,     8,   344,   176,    26,   201,   177,   176,   176,   208,
     177,   177,   249,   201,   405,   122,   340,   309,   340,   340,
     178,   179,   405,   394,   239,   333,   112,   124,   146,   152,
     273,   274,   275,   333,   150,   279,   280,   115,   175,   192,
     281,   282,   265,   218,   405,     8,   177,   288,   176,   146,
     324,   179,   179,   175,   258,   176,   405,   104,   320,   406,
      73,    13,   399,   179,   399,   176,   176,   179,   179,   263,
     256,   176,   122,   349,   309,   201,   206,    26,   203,   243,
     201,   176,   340,   122,   122,   201,    13,     8,   177,   178,
     178,     8,   177,     3,     4,     5,     6,     7,     9,    10,
      11,    12,    49,    62,    63,    64,    65,    66,    67,    68,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   123,   124,   125,   126,   127,   128,   129,   141,   142,
     143,   145,   154,   155,   156,   159,   166,   167,   169,   171,
     172,   173,   192,   330,   331,     8,   177,   146,   150,   192,
     282,   283,   284,   177,    73,   293,   217,   266,   394,   218,
     258,   176,   175,   178,    31,    73,    13,   340,   177,   178,
     285,   320,   399,   179,   176,   349,   122,    26,   203,   242,
     201,   340,   340,   179,   340,   333,   269,   276,   339,   274,
      13,    26,    43,   277,   280,     8,    29,   176,    25,    42,
      45,    13,     8,   177,   395,   293,    13,   217,   176,    31,
      73,   321,   201,    73,    13,   340,   201,   178,   285,   399,
     349,   201,    87,   213,   179,   192,   199,   270,   271,   272,
       8,   179,   340,   331,   331,    51,   278,   283,   283,    25,
      42,    45,   340,    73,   175,   177,   340,   395,    73,     8,
     345,   179,    13,   340,   179,   201,   285,    85,   177,   102,
     209,   140,    90,   339,   153,    13,   267,   175,    31,    73,
     176,   340,   179,   177,   175,   159,   216,   192,   288,   289,
     340,   157,   254,   255,   361,   268,    73,   333,   214,   155,
     192,   177,   176,     8,   345,   106,   107,   108,   291,   292,
     254,    73,   239,   177,   399,   157,   361,   406,   176,   176,
     177,   177,   178,   286,   291,    31,    73,   399,   178,   201,
     406,    73,    13,   286,   201,   179,    31,    73,    13,   340,
     179,    73,    13,   340,    13,   340,   340
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
#line 701 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 704 "hphp.y"
    { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 711 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 712 "hphp.y"
    { }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 715 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 716 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 717 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 718 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 719 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 720 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 723 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 725 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 726 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 727 "hphp.y"
    { _p->onNamespaceStart("");}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 728 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 729 "hphp.y"
    { _p->nns(); (yyval).reset();}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 730 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 735 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 736 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 737 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 741 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 746 "hphp.y"
    { }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 747 "hphp.y"
    { }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 750 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 751 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 752 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 754 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 758 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 760 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 763 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 765 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 766 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 769 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 776 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 783 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 791 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 794 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 800 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 801 "hphp.y"
    { _p->onStatementListStart((yyval));}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 810 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 814 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 819 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 820 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 822 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 826 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 829 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 833 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 835 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 838 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 840 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 843 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 844 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 845 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 846 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 847 "hphp.y"
    { _p->onReturn((yyval), NULL);}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 848 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 849 "hphp.y"
    { _p->onYieldBreak((yyval));}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 850 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 851 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 852 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 853 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 854 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 855 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 858 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 860 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 864 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 871 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 872 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(2) - (13)]),(yyvsp[(5) - (13)]),(yyvsp[(6) - (13)]),(yyvsp[(9) - (13)]),(yyvsp[(11) - (13)]),(yyvsp[(13) - (13)]));}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 875 "hphp.y"
    { _p->onCompleteLabelScope(false);}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 876 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(2) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 877 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 878 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 882 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 883 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 884 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 885 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 886 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 887 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 888 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)])); }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 889 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 890 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval));
                                         _p->onScopeLabel((yyval), (yyvsp[(1) - (2)]));}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 898 "hphp.y"
    { _p->onNewLabelScope(false);}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 899 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 908 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 909 "hphp.y"
    { (yyval).reset();}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 913 "hphp.y"
    { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 915 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFinally((yyval), (yyvsp[(3) - (4)]));
                                         _p->onCompleteLabelScope(false);}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 921 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 922 "hphp.y"
    { (yyval).reset();}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 926 "hphp.y"
    { (yyval) = 1;}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 927 "hphp.y"
    { (yyval).reset();}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 931 "hphp.y"
    { _p->pushFuncLocation();}
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 936 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 942 "hphp.y"
    { _p->onFunction((yyval),nullptr,(yyvsp[(8) - (9)]),(yyvsp[(2) - (9)]),(yyvsp[(3) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 948 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 954 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 960 "hphp.y"
    { (yyvsp[(5) - (5)]).setText(_p->nsDecl((yyvsp[(5) - (5)]).text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart((yyvsp[(5) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 966 "hphp.y"
    { _p->onFunction((yyval),&(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 974 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 977 "hphp.y"
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

  case 109:

/* Line 1806 of yacc.c  */
#line 992 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 995 "hphp.y"
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

  case 111:

/* Line 1806 of yacc.c  */
#line 1009 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 1012 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 1017 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 1020 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 1027 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));}
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 1030 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,(yyvsp[(4) - (7)]),
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 1038 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 1041 "hphp.y"
    { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,(yyvsp[(5) - (8)]),
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 1049 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1050 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1054 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1057 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1060 "hphp.y"
    { (yyval) = T_CLASS;}
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1061 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1062 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 1066 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 1067 "hphp.y"
    { (yyval).reset();}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1070 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1071 "hphp.y"
    { (yyval).reset();}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 1074 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 1075 "hphp.y"
    { (yyval).reset();}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 1080 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 1083 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 1085 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 1089 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 1090 "hphp.y"
    { (yyval).reset();}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 1093 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 1094 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 1095 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (4)]), NULL);}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 1099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 1101 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 1104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 1106 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 1109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 1111 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 1114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 1116 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 1126 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 1127 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 1128 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 1134 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1136 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1137 "hphp.y"
    { (yyval).reset();}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1140 "hphp.y"
    { (yyval).reset();}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval).reset();}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1146 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1147 "hphp.y"
    { (yyval).reset();}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1152 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1153 "hphp.y"
    { (yyval).reset();}
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 1156 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 1157 "hphp.y"
    { (yyval).reset();}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1160 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 1161 "hphp.y"
    { (yyval).reset();}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval).reset();}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval).reset();}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1176 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 1180 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 1185 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1190 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 1195 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(3) - (6)]),&(yyvsp[(4) - (6)]));}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1200 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(3) - (7)]),&(yyvsp[(4) - (7)]));}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 1206 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(3) - (9)]),&(yyvsp[(4) - (9)]));}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1212 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),&(yyvsp[(4) - (8)]));}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 1218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]);}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 1220 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval).reset();}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1222 "hphp.y"
    { (yyval).reset();}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1227 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1230 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1234 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);}
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1238 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1242 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);}
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1251 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1256 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1263 "hphp.y"
    { (yyval).reset();}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1266 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1267 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1269 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1271 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1276 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1279 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1280 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1281 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1285 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1287 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1288 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1289 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1294 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1295 "hphp.y"
    { (yyval).reset();}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1298 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1299 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1303 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1322 "hphp.y"
    { _p->onNewLabelScope(true);
                                         _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1333 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1335 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1337 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1338 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1341 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1344 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1345 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1346 "hphp.y"
    { (yyval).reset(); }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1359 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1366 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1367 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1372 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1375 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1382 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1384 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1388 "hphp.y"
    { (yyval) = 4;}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1389 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1395 "hphp.y"
    { (yyval) = 6;}
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1397 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1401 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1407 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1408 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1412 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1413 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1417 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1420 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1425 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1430 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1431 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1433 "hphp.y"
    { (yyval) = 0;}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1437 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1438 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1439 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1440 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1445 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1446 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1447 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1448 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1450 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1452 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1456 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1459 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1460 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1464 "hphp.y"
    { (yyval).reset();}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1465 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1469 "hphp.y"
    { (yyval).reset();}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1470 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1473 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval).reset();}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1477 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1478 "hphp.y"
    { (yyval).reset();}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1481 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1488 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1489 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1490 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1491 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1492 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1496 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1497 "hphp.y"
    { (yyval).reset();}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1500 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1501 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1502 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1510 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1514 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1519 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1522 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1523 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1524 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1527 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1537 "hphp.y"
    { (yyval).reset();}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1559 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1569 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1570 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1574 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1579 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1583 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1584 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1585 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1586 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1587 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1589 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1590 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1591 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1592 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1593 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1594 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1595 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1596 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1597 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1598 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1599 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1600 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1601 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1602 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1603 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1604 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1605 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1606 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1607 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1608 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1609 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1610 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1611 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1612 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1613 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1614 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1615 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1616 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1617 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1618 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1619 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1620 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1622 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1623 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1627 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1628 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1629 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1630 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1631 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1633 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1634 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1635 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1638 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1639 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1640 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1643 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1644 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { Token t; 
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1652 "hphp.y"
    { Token u; u.reset();
                                         _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         _p->onClosure((yyval),0,u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1658 "hphp.y"
    { Token t; 
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1664 "hphp.y"
    { Token u; u.reset();
                                         _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         _p->onClosure((yyval),&(yyvsp[(1) - (12)]),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1683 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1690 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1693 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1698 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1699 "hphp.y"
    { (yyval).reset(); }
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval).reset(); }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1709 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1713 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1714 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1719 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1726 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1733 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1735 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1740 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1741 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval).reset();}
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1751 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 1752 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 1753 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1754 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 1761 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1764 "hphp.y"
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

  case 411:

/* Line 1806 of yacc.c  */
#line 1775 "hphp.y"
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

  case 412:

/* Line 1806 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1792 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval).reset();}
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1796 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval).reset();}
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1800 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1804 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       }
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 1878 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 1879 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 1880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 1881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 1885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 1888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 1889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 1890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 1892 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 1893 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 1894 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 1895 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 1896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 1897 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 1898 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 1899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 1900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 1903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 1904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 1905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 1906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 1907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 1908 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 1917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 1918 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 1921 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 1923 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 1927 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 1928 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 1929 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 1933 "hphp.y"
    { (yyval).reset();}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 1934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 1935 "hphp.y"
    { (yyval).reset();}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 1939 "hphp.y"
    { (yyval).reset();}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 1940 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 1941 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 1945 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 1946 "hphp.y"
    { (yyval).reset();}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 1950 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 1954 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 1955 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 1958 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 1959 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 1960 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 1961 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 1962 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 1965 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 1967 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 1971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 1972 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 1973 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 1974 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 1976 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 1978 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 1979 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 1982 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 1983 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 1991 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 1996 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 1997 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 1998 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 1999 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2001 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2003 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval).reset();}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval).reset();}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2014 "hphp.y"
    { (yyval).reset();}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2017 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval).reset();}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2033 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2039 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2044 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2045 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2046 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2050 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2051 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2052 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2054 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2056 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2057 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2059 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval).reset();}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2072 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2075 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2079 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval).reset(); }
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2094 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2100 "hphp.y"
    { (yyval).reset();}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2104 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2111 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2116 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2118 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval).reset();}
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2141 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2170 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2178 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2191 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2203 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2207 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2211 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2215 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2222 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2250 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval).reset();}
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval) = 1;}
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2256 "hphp.y"
    { (yyval)++;}
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2260 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2264 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2268 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2272 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2274 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2277 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2282 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2284 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2285 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2286 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2287 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2293 "hphp.y"
    { (yyval).reset();}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2297 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2298 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2299 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2303 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2305 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2307 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2313 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2317 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2318 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2319 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2320 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2325 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2331 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2335 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2340 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2342 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2343 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2345 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2350 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2352 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2354 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2358 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2359 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2362 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2368 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2369 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2370 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 712:

/* Line 1806 of yacc.c  */
#line 2371 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2372 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2373 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2374 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2375 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2376 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2380 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2381 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2400 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2403 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2408 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2409 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2415 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2419 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2425 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2426 "hphp.y"
    { (yyval).reset(); }
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2430 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2438 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval).reset(); }
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2441 "hphp.y"
    { (yyval).reset(); }
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2445 "hphp.y"
    { (yyval).reset(); }
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2446 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2451 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2452 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2454 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2455 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2461 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2474 "hphp.y"
    {}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2478 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2485 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2488 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2491 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2492 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2495 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2497 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2500 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2503 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2509 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2513 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2521 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2522 "hphp.y"
    { (yyval).reset(); }
    break;



/* Line 1806 of yacc.c  */
#line 10994 "hphp.tab.cpp"
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
#line 2525 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

