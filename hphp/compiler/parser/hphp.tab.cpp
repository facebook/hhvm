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
#define YYLAST   12992

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  199
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  241
/* YYNRULES -- Number of rules.  */
#define YYNRULES  809
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1511

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
    1376,  1379,  1381,  1383,  1385,  1389,  1392,  1393,  1405,  1406,
    1419,  1421,  1423,  1429,  1433,  1439,  1443,  1446,  1447,  1450,
    1451,  1456,  1461,  1465,  1470,  1475,  1480,  1485,  1487,  1489,
    1493,  1496,  1500,  1505,  1508,  1512,  1514,  1517,  1519,  1522,
    1524,  1526,  1528,  1530,  1532,  1534,  1539,  1544,  1547,  1556,
    1567,  1570,  1572,  1576,  1578,  1581,  1583,  1585,  1587,  1589,
    1592,  1597,  1601,  1607,  1608,  1612,  1617,  1619,  1622,  1627,
    1630,  1637,  1638,  1640,  1645,  1646,  1649,  1650,  1652,  1654,
    1658,  1660,  1664,  1666,  1668,  1672,  1676,  1678,  1680,  1682,
    1684,  1686,  1688,  1690,  1692,  1694,  1696,  1698,  1700,  1702,
    1704,  1706,  1708,  1710,  1712,  1714,  1716,  1718,  1720,  1722,
    1724,  1726,  1728,  1730,  1732,  1734,  1736,  1738,  1740,  1742,
    1744,  1746,  1748,  1750,  1752,  1754,  1756,  1758,  1760,  1762,
    1764,  1766,  1768,  1770,  1772,  1774,  1776,  1778,  1780,  1782,
    1784,  1786,  1788,  1790,  1792,  1794,  1796,  1798,  1800,  1802,
    1804,  1806,  1808,  1810,  1812,  1814,  1816,  1818,  1820,  1822,
    1824,  1826,  1828,  1830,  1832,  1834,  1836,  1841,  1843,  1845,
    1847,  1849,  1851,  1853,  1855,  1857,  1860,  1862,  1863,  1864,
    1866,  1868,  1872,  1873,  1875,  1877,  1879,  1881,  1883,  1885,
    1887,  1889,  1891,  1893,  1895,  1897,  1901,  1904,  1906,  1908,
    1911,  1914,  1919,  1924,  1928,  1933,  1935,  1937,  1941,  1945,
    1947,  1949,  1951,  1953,  1957,  1961,  1965,  1968,  1969,  1971,
    1972,  1974,  1975,  1981,  1985,  1989,  1991,  1993,  1995,  1997,
    2001,  2004,  2006,  2008,  2010,  2012,  2014,  2017,  2020,  2025,
    2030,  2034,  2039,  2042,  2043,  2049,  2053,  2057,  2059,  2063,
    2065,  2068,  2069,  2075,  2079,  2082,  2083,  2087,  2088,  2093,
    2096,  2097,  2101,  2105,  2107,  2108,  2110,  2113,  2116,  2121,
    2125,  2129,  2132,  2137,  2140,  2145,  2147,  2149,  2151,  2153,
    2155,  2158,  2163,  2167,  2172,  2176,  2178,  2180,  2182,  2184,
    2187,  2192,  2197,  2201,  2203,  2205,  2209,  2217,  2224,  2233,
    2243,  2252,  2263,  2271,  2278,  2287,  2289,  2292,  2297,  2302,
    2304,  2306,  2311,  2313,  2314,  2316,  2319,  2321,  2323,  2326,
    2331,  2335,  2339,  2340,  2342,  2345,  2350,  2354,  2357,  2361,
    2368,  2369,  2371,  2376,  2379,  2380,  2386,  2390,  2394,  2396,
    2403,  2408,  2413,  2416,  2419,  2420,  2426,  2430,  2434,  2436,
    2439,  2440,  2446,  2450,  2454,  2456,  2459,  2462,  2464,  2467,
    2469,  2474,  2478,  2482,  2489,  2493,  2495,  2497,  2499,  2504,
    2509,  2514,  2519,  2522,  2525,  2530,  2533,  2536,  2538,  2542,
    2546,  2547,  2550,  2556,  2563,  2565,  2568,  2570,  2575,  2579,
    2580,  2582,  2586,  2590,  2592,  2594,  2595,  2596,  2599,  2603,
    2605,  2611,  2615,  2619,  2623,  2625,  2628,  2629,  2634,  2637,
    2640,  2642,  2644,  2646,  2651,  2658,  2660,  2669,  2675,  2677
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
     151,   295,   191,    -1,     4,   119,   366,   191,    -1,     4,
     120,   366,   191,    -1,   104,   253,   191,    -1,   104,   253,
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
    1337,  1352,  1349,  1362,  1364,  1366,  1368,  1370,  1372,  1374,
    1378,  1379,  1380,  1383,  1389,  1392,  1398,  1401,  1406,  1408,
    1413,  1418,  1422,  1423,  1429,  1430,  1435,  1436,  1441,  1442,
    1446,  1447,  1451,  1453,  1459,  1464,  1465,  1467,  1471,  1472,
    1473,  1474,  1478,  1479,  1480,  1481,  1482,  1483,  1485,  1490,
    1493,  1494,  1498,  1499,  1503,  1504,  1507,  1508,  1511,  1512,
    1515,  1516,  1520,  1521,  1522,  1523,  1524,  1525,  1526,  1530,
    1531,  1534,  1535,  1536,  1539,  1541,  1543,  1544,  1547,  1549,
    1553,  1554,  1556,  1557,  1558,  1561,  1565,  1566,  1570,  1571,
    1575,  1576,  1580,  1584,  1589,  1593,  1597,  1602,  1603,  1604,
    1607,  1609,  1610,  1611,  1614,  1615,  1616,  1617,  1618,  1619,
    1620,  1621,  1622,  1623,  1624,  1625,  1626,  1627,  1628,  1629,
    1630,  1631,  1632,  1633,  1634,  1635,  1636,  1637,  1638,  1639,
    1640,  1641,  1642,  1643,  1644,  1645,  1646,  1647,  1648,  1649,
    1650,  1651,  1652,  1653,  1654,  1656,  1657,  1659,  1661,  1662,
    1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,  1671,  1672,
    1673,  1674,  1675,  1676,  1677,  1678,  1680,  1679,  1692,  1691,
    1703,  1707,  1711,  1715,  1721,  1725,  1731,  1733,  1737,  1739,
    1743,  1747,  1748,  1752,  1759,  1766,  1768,  1773,  1774,  1775,
    1779,  1783,  1787,  1791,  1793,  1795,  1797,  1802,  1803,  1808,
    1809,  1810,  1811,  1812,  1813,  1817,  1821,  1825,  1829,  1834,
    1839,  1843,  1844,  1848,  1849,  1853,  1854,  1858,  1859,  1863,
    1867,  1871,  1875,  1877,  1881,  1882,  1883,  1884,  1888,  1894,
    1903,  1916,  1917,  1920,  1923,  1926,  1927,  1930,  1934,  1937,
    1940,  1947,  1948,  1952,  1953,  1955,  1959,  1960,  1961,  1962,
    1963,  1964,  1965,  1966,  1967,  1968,  1969,  1970,  1971,  1972,
    1973,  1974,  1975,  1976,  1977,  1978,  1979,  1980,  1981,  1982,
    1983,  1984,  1985,  1986,  1987,  1988,  1989,  1990,  1991,  1992,
    1993,  1994,  1995,  1996,  1997,  1998,  1999,  2000,  2001,  2002,
    2003,  2004,  2005,  2006,  2007,  2008,  2009,  2010,  2011,  2012,
    2013,  2014,  2015,  2016,  2017,  2018,  2019,  2020,  2021,  2022,
    2023,  2024,  2025,  2026,  2027,  2028,  2029,  2030,  2031,  2032,
    2033,  2034,  2035,  2036,  2037,  2038,  2042,  2047,  2048,  2051,
    2052,  2053,  2057,  2058,  2059,  2063,  2064,  2065,  2069,  2070,
    2071,  2074,  2076,  2080,  2081,  2082,  2084,  2085,  2086,  2087,
    2088,  2089,  2090,  2091,  2092,  2093,  2096,  2101,  2102,  2103,
    2104,  2105,  2107,  2109,  2110,  2112,  2113,  2117,  2120,  2126,
    2127,  2128,  2129,  2130,  2131,  2132,  2137,  2139,  2143,  2144,
    2147,  2148,  2152,  2155,  2157,  2159,  2163,  2164,  2165,  2167,
    2170,  2174,  2175,  2176,  2179,  2180,  2181,  2182,  2183,  2185,
    2187,  2188,  2193,  2195,  2198,  2201,  2203,  2205,  2208,  2210,
    2214,  2216,  2219,  2222,  2228,  2230,  2233,  2234,  2239,  2242,
    2246,  2246,  2251,  2254,  2255,  2259,  2260,  2265,  2266,  2270,
    2271,  2275,  2276,  2281,  2283,  2288,  2289,  2290,  2291,  2292,
    2293,  2294,  2296,  2299,  2301,  2305,  2306,  2307,  2308,  2309,
    2311,  2313,  2315,  2319,  2320,  2321,  2325,  2328,  2331,  2334,
    2338,  2342,  2349,  2353,  2357,  2364,  2365,  2370,  2372,  2373,
    2376,  2377,  2380,  2381,  2385,  2386,  2390,  2391,  2392,  2393,
    2395,  2398,  2401,  2402,  2403,  2405,  2407,  2411,  2412,  2413,
    2415,  2416,  2417,  2421,  2423,  2426,  2428,  2429,  2430,  2431,
    2434,  2436,  2437,  2441,  2443,  2446,  2448,  2449,  2450,  2454,
    2456,  2459,  2462,  2464,  2466,  2470,  2471,  2473,  2474,  2480,
    2481,  2483,  2485,  2487,  2489,  2492,  2493,  2494,  2498,  2499,
    2500,  2501,  2502,  2503,  2504,  2505,  2506,  2510,  2511,  2515,
    2523,  2525,  2529,  2532,  2538,  2539,  2545,  2546,  2553,  2556,
    2560,  2563,  2568,  2569,  2570,  2571,  2575,  2576,  2580,  2582,
    2583,  2585,  2589,  2595,  2597,  2601,  2604,  2607,  2615,  2618,
    2621,  2622,  2625,  2626,  2629,  2633,  2637,  2643,  2651,  2652
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
     278,   282,   278,   278,   278,   278,   278,   278,   278,   278,
     283,   283,   283,   284,   285,   285,   286,   286,   287,   287,
     288,   288,   289,   289,   289,   289,   290,   290,   291,   291,
     292,   292,   293,   293,   294,   295,   295,   295,   296,   296,
     296,   296,   297,   297,   297,   297,   297,   297,   297,   298,
     298,   298,   299,   299,   300,   300,   301,   301,   302,   302,
     303,   303,   304,   304,   304,   304,   304,   304,   304,   305,
     305,   306,   306,   306,   307,   307,   307,   307,   308,   308,
     309,   309,   309,   309,   309,   310,   311,   311,   312,   312,
     313,   313,   314,   315,   316,   317,   318,   319,   319,   319,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   321,   320,   322,   320,
     320,   323,   324,   324,   325,   325,   326,   326,   327,   327,
     328,   329,   329,   330,   331,   332,   332,   333,   333,   333,
     334,   335,   336,   337,   337,   337,   337,   338,   338,   339,
     339,   339,   339,   339,   339,   340,   341,   342,   343,   344,
     345,   346,   346,   347,   347,   348,   348,   349,   349,   350,
     351,   352,   353,   353,   354,   354,   354,   354,   355,   356,
     356,   357,   357,   358,   358,   359,   359,   360,   361,   361,
     362,   362,   362,   363,   363,   363,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   364,   364,   364,   364,
     364,   364,   364,   364,   364,   364,   365,   366,   366,   367,
     367,   367,   368,   368,   368,   369,   369,   369,   370,   370,
     370,   371,   371,   372,   372,   372,   372,   372,   372,   372,
     372,   372,   372,   372,   372,   372,   372,   373,   373,   373,
     373,   373,   373,   373,   373,   373,   373,   374,   374,   375,
     375,   375,   375,   375,   375,   375,   376,   376,   377,   377,
     378,   378,   379,   379,   379,   379,   380,   380,   380,   380,
     380,   381,   381,   381,   382,   382,   382,   382,   382,   382,
     382,   382,   383,   383,   384,   384,   384,   384,   385,   385,
     386,   386,   387,   387,   388,   388,   389,   389,   390,   390,
     392,   391,   393,   394,   394,   395,   395,   396,   396,   397,
     397,   398,   398,   399,   399,   400,   400,   400,   400,   400,
     400,   400,   400,   400,   400,   401,   401,   401,   401,   401,
     401,   401,   401,   402,   402,   402,   403,   403,   403,   403,
     403,   403,   404,   404,   404,   405,   405,   406,   406,   406,
     407,   407,   408,   408,   409,   409,   410,   410,   410,   410,
     410,   410,   411,   411,   411,   411,   411,   412,   412,   412,
     412,   412,   412,   413,   413,   414,   414,   414,   414,   414,
     414,   414,   414,   415,   415,   416,   416,   416,   416,   417,
     417,   418,   418,   418,   418,   419,   419,   419,   419,   420,
     420,   420,   420,   420,   420,   421,   421,   421,   422,   422,
     422,   422,   422,   422,   422,   422,   422,   423,   423,   424,
     425,   425,   426,   426,   427,   427,   428,   428,   429,   429,
     430,   430,   431,   431,   431,   431,   432,   432,   433,   433,
     433,   433,   434,   435,   435,   436,   436,   437,   438,   438,
     438,   438,   438,   438,   438,   438,   438,   438,   439,   439
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
       2,     1,     1,     1,     3,     2,     0,    11,     0,    12,
       1,     1,     5,     3,     5,     3,     2,     0,     2,     0,
       4,     4,     3,     4,     4,     4,     4,     1,     1,     3,
       2,     3,     4,     2,     3,     1,     2,     1,     2,     1,
       1,     1,     1,     1,     1,     4,     4,     2,     8,    10,
       2,     1,     3,     1,     2,     1,     1,     1,     1,     2,
       4,     3,     5,     0,     3,     4,     1,     2,     4,     2,
       6,     0,     1,     4,     0,     2,     0,     1,     1,     3,
       1,     3,     1,     1,     3,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     0,     0,     1,
       1,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     2,
       2,     4,     4,     3,     4,     1,     1,     3,     3,     1,
       1,     1,     1,     3,     3,     3,     2,     0,     1,     0,
       1,     0,     5,     3,     3,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     1,     1,     2,     2,     4,     4,
       3,     4,     2,     0,     5,     3,     3,     1,     3,     1,
       2,     0,     5,     3,     2,     0,     3,     0,     4,     2,
       0,     3,     3,     1,     0,     1,     2,     2,     4,     3,
       3,     2,     4,     2,     4,     1,     1,     1,     1,     1,
       2,     4,     3,     4,     3,     1,     1,     1,     1,     2,
       4,     4,     3,     1,     1,     3,     7,     6,     8,     9,
       8,    10,     7,     6,     8,     1,     2,     4,     4,     1,
       1,     4,     1,     0,     1,     2,     1,     1,     2,     4,
       3,     3,     0,     1,     2,     4,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     2,     1,     2,     1,
       4,     3,     3,     6,     3,     1,     1,     1,     4,     4,
       4,     4,     2,     2,     4,     2,     2,     1,     3,     3,
       0,     2,     5,     6,     1,     2,     1,     4,     3,     0,
       1,     3,     3,     1,     1,     0,     0,     2,     3,     1,
       5,     3,     3,     3,     1,     2,     0,     4,     2,     2,
       1,     1,     1,     4,     6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   650,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   724,     0,   712,   567,
       0,   573,   574,    19,   600,   700,    84,   575,     0,    66,
       0,     0,     0,     0,     0,     0,     0,     0,   115,     0,
       0,     0,     0,     0,     0,   292,   293,   294,   297,   296,
     295,     0,     0,     0,     0,   138,     0,     0,     0,   579,
     581,   582,   576,   577,     0,     0,   583,   578,     0,     0,
     558,    20,    21,    22,    24,    23,     0,   580,     0,     0,
       0,     0,   584,     0,   298,    25,    26,    27,    29,    28,
      30,    31,    32,    33,    34,    35,    36,    37,    38,     0,
      83,    56,   704,   568,     0,     0,     4,    45,    47,    50,
     599,     0,   557,     0,     6,   114,     7,     8,     9,     0,
       0,   290,   329,     0,     0,     0,     0,     0,     0,     0,
     327,   393,   392,   314,   400,     0,     0,   313,   666,   559,
       0,   602,   391,   289,   669,   328,     0,     0,   667,   668,
     665,   695,   699,     0,   381,   601,    10,   297,   296,   295,
       0,     0,    45,   114,     0,   766,   328,   765,     0,   763,
     762,   395,     0,     0,   365,   366,   367,   368,   390,   388,
     387,   386,   385,   384,   383,   382,   560,     0,   779,   559,
       0,   348,   346,     0,   728,     0,   609,   312,   563,     0,
     779,   562,     0,   572,   707,   706,   564,     0,     0,   566,
     389,     0,     0,     0,   317,     0,    64,   319,     0,     0,
      70,    72,     0,     0,    74,     0,     0,     0,   801,   805,
       0,     0,    45,   800,     0,   802,     0,     0,    76,     0,
       0,     0,     0,   105,     0,     0,     0,     0,    40,    41,
     215,     0,     0,   214,   140,   139,   220,     0,     0,     0,
       0,     0,   776,   126,   136,   720,   724,   749,     0,   586,
       0,     0,     0,   747,     0,    15,     0,    49,     0,   320,
     130,   137,   464,   407,     0,   770,   324,   329,     0,   327,
     328,     0,     0,   569,     0,   570,     0,     0,     0,   104,
       0,     0,    52,   208,     0,    18,   113,     0,   135,   122,
     134,   295,   114,   291,    95,    96,    97,    98,    99,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   712,    94,   703,   703,   102,
     734,     0,     0,     0,     0,     0,   288,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   347,
     345,     0,   670,   655,   703,     0,   661,   208,   703,     0,
     705,   696,   720,     0,   114,     0,     0,   652,   647,   609,
       0,     0,     0,     0,   732,     0,   412,   608,   723,     0,
       0,    52,     0,   208,   311,     0,   708,   655,   663,   565,
       0,    56,   176,     0,    81,     0,     0,   318,     0,     0,
       0,     0,     0,    73,    93,    75,   798,   799,     0,   796,
       0,     0,   780,     0,   775,     0,   100,    77,   103,     0,
       0,     0,     0,     0,     0,     0,   420,     0,   427,   429,
     430,   431,   432,   433,   434,   425,   447,   448,    56,     0,
      90,    92,    42,     0,    17,     0,     0,   216,     0,    79,
       0,     0,    80,   767,     0,     0,   329,   327,   328,     0,
       0,   146,     0,   721,     0,     0,     0,     0,   585,   748,
     600,     0,     0,   746,   605,   745,    48,     5,    12,    13,
      78,     0,   144,     0,     0,   401,     0,   609,     0,     0,
       0,     0,   310,   378,   674,    61,    55,    57,    58,    59,
      60,     0,   394,   603,   604,    46,     0,     0,     0,   611,
     209,     0,   396,   116,   142,     0,   351,   353,   352,     0,
       0,   349,   350,   354,   356,   355,   370,   369,   372,   371,
     373,   375,   376,   374,   364,   363,   358,   359,   357,   360,
     361,   362,   377,   702,     0,     0,   738,     0,   609,     0,
     769,   672,   695,   128,   132,   124,   114,     0,     0,   322,
     325,   331,   421,   344,   343,   342,   341,   340,   339,   338,
     337,   336,   335,   334,     0,   657,   656,     0,     0,     0,
       0,     0,     0,     0,   764,   645,   649,   608,   651,     0,
       0,   779,     0,   727,     0,   726,     0,   711,   710,     0,
       0,   657,   656,   315,   178,   180,   316,     0,    56,   160,
      65,   319,     0,     0,     0,     0,   172,   172,    71,     0,
       0,   794,   609,     0,   785,     0,     0,     0,   607,     0,
       0,   558,     0,    25,    50,   588,   557,   596,     0,   587,
      54,   595,     0,     0,   437,     0,     0,   443,   440,   441,
     449,     0,   428,   423,     0,   426,     0,     0,     0,     0,
      39,    43,     0,   213,   221,   218,     0,     0,   758,   761,
     760,   759,    11,   789,     0,     0,     0,   720,   717,     0,
     411,   757,   756,   755,     0,   751,     0,   752,   754,     0,
       5,   321,     0,     0,   458,   459,   467,   466,     0,     0,
     608,   406,   410,     0,   771,     0,     0,   671,   655,   662,
     701,     0,   778,   210,   556,   610,   207,     0,   654,     0,
       0,   144,   398,   118,   380,     0,   415,   416,     0,   413,
     608,   733,     0,     0,   208,   146,   144,   142,     0,   712,
     332,     0,     0,   208,   659,   660,   673,   697,   698,     0,
       0,     0,   633,   616,   617,   618,     0,     0,     0,    25,
     625,   624,   639,   609,     0,   647,   731,   730,     0,   709,
     655,   664,   571,     0,   182,     0,     0,    62,     0,     0,
       0,     0,     0,   152,   153,   164,     0,    56,   162,    87,
     172,     0,   172,     0,     0,   803,     0,   608,   795,   797,
     784,   783,     0,   781,   589,   590,   615,     0,   609,   607,
       0,     0,   409,   607,     0,   740,   422,     0,     0,     0,
     445,   446,   444,     0,     0,   424,     0,   106,     0,   109,
      91,    44,   217,     0,   768,    82,     0,     0,   777,   145,
     147,   223,     0,     0,   718,     0,   750,     0,    16,     0,
     143,   223,     0,     0,   403,     0,   772,     0,     0,   657,
     656,   781,     0,   211,    53,   197,     0,   611,   653,   809,
     654,   141,     0,   654,     0,   379,   737,   736,     0,   208,
       0,     0,     0,   144,   120,   572,   658,   208,     0,     0,
     621,   622,   623,   626,   627,   637,     0,   609,   633,     0,
     620,   641,   633,   608,   644,   646,   648,     0,   725,   658,
       0,     0,     0,     0,   179,    67,     0,   319,   154,   720,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   166,
       0,   792,   793,     0,     0,   807,     0,   593,   608,   606,
       0,   598,     0,   609,     0,     0,   597,   744,     0,   609,
     435,     0,   436,   442,   450,   451,     0,    56,   219,   791,
     788,     0,   289,   722,   720,   323,   326,   330,     0,    14,
     289,   470,     0,     0,   472,   465,   468,     0,   463,     0,
     773,     0,     0,   208,   212,   786,   654,   196,   808,     0,
       0,   223,     0,   654,     0,   208,     0,   693,   223,   223,
       0,     0,   333,   208,     0,   687,     0,   630,   608,   632,
       0,   619,     0,     0,   609,     0,   638,   729,     0,    56,
       0,   175,   161,     0,     0,   151,    85,   165,     0,     0,
     168,     0,   173,   174,    56,   167,   804,   782,     0,   614,
     613,   591,     0,   608,   408,   594,   592,     0,   414,   608,
     739,     0,     0,     0,     0,   148,     0,     0,     0,   287,
       0,     0,     0,   127,   222,   224,     0,   286,     0,   289,
       0,   753,   131,   461,     0,     0,   402,   658,   208,     0,
       0,   453,   195,   809,     0,   199,   786,   289,   786,     0,
     735,     0,   692,   289,   289,   223,   654,     0,   686,   636,
     635,   628,     0,   631,   608,   640,   629,    56,   181,    63,
      68,   155,     0,   163,   169,    56,   171,     0,     0,   405,
       0,   743,   742,     0,    56,   110,   790,     0,     0,     0,
       0,   149,   254,   252,   558,    24,     0,   248,     0,   253,
     264,     0,   262,   267,     0,   266,     0,   265,     0,   114,
     226,     0,   228,     0,   719,   462,   460,   471,   469,   208,
       0,   690,   787,     0,     0,     0,   200,     0,     0,   123,
     453,   786,   694,   129,   133,   289,     0,   688,     0,   643,
       0,   177,     0,    56,   158,    86,   170,   806,   612,     0,
       0,     0,     0,     0,     0,     0,     0,   238,   242,     0,
       0,   233,   522,   521,   518,   520,   519,   539,   541,   540,
     510,   500,   516,   515,   477,   487,   488,   490,   489,   509,
     493,   491,   492,   494,   495,   496,   497,   498,   499,   501,
     502,   503,   504,   505,   506,   508,   507,   478,   479,   480,
     483,   484,   486,   524,   525,   534,   533,   532,   531,   530,
     529,   517,   536,   526,   527,   528,   511,   512,   513,   514,
     537,   538,   542,   544,   543,   545,   546,   523,   548,   547,
     481,   550,   552,   551,   485,   555,   553,   554,   549,   482,
     535,   476,   259,   473,     0,   234,   280,   281,   279,   272,
       0,   273,   235,   306,     0,     0,     0,     0,   114,     0,
     689,     0,    56,     0,   203,     0,   202,   282,    56,   117,
       0,     0,   125,   786,   634,     0,    56,   156,    69,     0,
     404,   741,   438,   108,   236,   237,   309,   150,     0,     0,
     256,   249,     0,     0,     0,   261,   263,     0,     0,   268,
     275,   276,   274,     0,     0,   225,     0,     0,     0,     0,
     691,     0,   456,   611,     0,   204,     0,   201,     0,    56,
     119,     0,   642,     0,     0,     0,    88,   239,    45,     0,
     240,   241,     0,     0,   255,   258,   474,   475,     0,   250,
     277,   278,   270,   271,   269,   307,   304,   229,   227,   308,
       0,   457,   610,     0,   397,     0,   206,   283,     0,   121,
       0,   159,   439,     0,   112,     0,   289,   257,   260,     0,
     654,   231,     0,   454,   452,   205,   399,   157,     0,     0,
      89,   246,     0,   288,   305,   185,     0,   611,   300,   654,
     455,     0,   111,     0,     0,   245,   786,   654,   184,   301,
     302,   303,   809,   299,     0,     0,     0,   244,     0,   183,
     300,     0,   786,     0,   243,   284,    56,   230,   809,     0,
     187,     0,    56,     0,     0,   188,     0,   232,     0,   285,
       0,   191,     0,   190,   107,   192,     0,   189,     0,   194,
     193
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,   116,   720,   507,   172,   257,   258,
     118,   119,   120,   121,   122,   123,   301,   526,   527,   426,
     225,  1212,   432,  1142,  1434,   688,   254,   468,  1396,   860,
     987,  1450,   317,   173,   528,   749,   904,  1031,   529,   544,
     767,   491,   765,   530,   512,   766,   319,   273,   290,   129,
     751,   723,   706,   869,  1160,   951,   813,  1348,  1215,   640,
     819,   431,   648,   821,  1064,   635,   804,   807,   942,  1456,
    1457,   896,   897,   538,   539,   262,   263,   267,   992,  1094,
    1178,  1326,  1440,  1459,  1358,  1400,  1401,  1402,  1166,  1167,
    1168,  1359,  1365,  1409,  1171,  1172,  1176,  1319,  1320,  1321,
    1339,  1487,  1095,  1096,   174,   131,  1472,  1473,  1324,  1098,
     132,   219,   427,   428,   133,   134,   135,   136,   137,   138,
     139,   140,   748,   903,   516,   517,   973,   518,   974,   141,
     142,   143,   667,   144,   145,   251,   146,   252,   456,   457,
     458,   459,   460,   461,   462,   463,   464,   678,   679,   852,
     465,   466,   467,   685,  1194,  1383,   147,   513,  1186,   514,
     882,   728,  1008,  1005,  1312,  1313,   148,   149,   150,   213,
     220,   304,   414,   151,   836,   671,   152,   837,   408,   746,
     838,   791,   923,   925,   926,   927,   793,  1043,  1044,   794,
     616,   399,   182,   183,   153,   899,   382,   383,   739,   154,
     214,   176,   156,   157,   158,   159,   160,   161,   162,   574,
     163,   216,   217,   494,   205,   206,   577,   578,   978,   979,
     282,   283,   714,   164,   484,   165,   521,   166,   244,   274,
     312,   441,   832,  1111,   704,   651,   652,   653,   245,   246,
    1019
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1235
static const yytype_int16 yypact[] =
{
   -1235,   148, -1235, -1235,  3967, 10187, 10187,   -31, 10187, 10187,
   10187, -1235, 10187, 10187, 10187, 10187, 10187, 10187, 10187, 10187,
   10187, 10187, 10187, 10187, 11871, 11871,  8025, 10187, 11920,   -23,
      10, -1235, -1235, -1235, -1235, -1235, -1235, -1235, 10187, -1235,
      10,    13,   138,   157,    10,  8183, 12325,  8341, -1235, 11323,
    7671,   160, 10187, 12265,    -8, -1235, -1235, -1235,    76,    79,
      68,   175,   177,   182,   187, -1235, 12325,   211,   216, -1235,
   -1235, -1235, -1235, -1235,   294, 11967, -1235, -1235, 12325,  8499,
   -1235, -1235, -1235, -1235, -1235, -1235, 12325, -1235,    60,   220,
   12325, 12325, -1235, 10187, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, 10187,
   -1235, -1235,   249,   324,   391,   391, -1235,   417,   312,   229,
   -1235,   271, -1235,    46, -1235,   427, -1235, -1235, -1235, 12308,
     277, -1235, -1235,   275,   279,   284,   286,   300,   303, 10850,
   -1235, -1235,   434, -1235,   436,   443,   314, -1235,    11,   315,
     368, -1235, -1235,   441,    12,   964,    81,   321,   103,   108,
     331,    20, -1235,    29, -1235,   458, -1235, -1235, -1235,   385,
     339,   387, -1235,   427,   277, 12845,  1786, 12845, 10187, 12845,
   12845, 11561,   494, 12325, -1235, -1235,   486, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, 11624,   375, -1235,
     406,   424,   424, 11871, 12505,   357,   578, -1235,   385, 11624,
     375,   452,   453,   400,   115, -1235,   473,    81,  8657, -1235,
   -1235, 10187,  6375,    56, 12845,  7317, -1235, 10187, 10187, 12325,
   -1235, -1235, 10891,   407, -1235, 10932, 11323, 11323,   440, -1235,
     414,  2620,   592, -1235,   596, -1235, 12325,   542, -1235,   425,
   10973,   431,   580, -1235,     9, 11015, 12325,    61, -1235,    41,
   -1235, 11724,    62, -1235, -1235, -1235,   605,    63, 11871, 11871,
   10187,   437,   464, -1235, -1235, 11773,  8025,    74,   362, -1235,
   10345, 11871,   346, -1235, 12325, -1235,   221,   312,   455, 12546,
   -1235, -1235, -1235,   549,   618,   553, 12845,   454, 12845,   461,
     548,  4125, 10187,   269,   444,   401,   269,   290,    32, -1235,
   12325, 11323,   459,  8853, 11323, -1235, -1235, 12085, -1235, -1235,
   -1235, -1235,   427, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   10187, 10187, 10187,  9049, 10187, 10187, 10187, 10187, 10187, 10187,
   10187, 10187, 10187, 10187, 10187, 10187, 10187, 10187, 10187, 10187,
   10187, 10187, 10187, 10187, 10187, 11920, -1235, 10187, 10187, -1235,
   10187,  2040, 12325, 12325, 12308,   558,   527,  7513, 10187, 10187,
   10187, 10187, 10187, 10187, 10187, 10187, 10187, 10187, 10187, -1235,
   -1235,  2237, -1235,   120, 10187, 10187, -1235,  8853, 10187, 10187,
     249,   122, 11773,   469,   427,  9245,  3221, -1235,   471,   653,
   11624,   472,   -29,  2040,   424,  9441, -1235,  9637, -1235,   476,
       2, -1235,    33,  8853, -1235,  2903, -1235,   123, -1235, -1235,
   11056, -1235, -1235, 10187, -1235,   585,  6571,   662,   481, 12738,
     660,    80,    27, -1235, -1235, -1235, -1235, -1235, 11323,   595,
     487,   669, -1235, 11487, -1235,   500, -1235, -1235, -1235,   606,
   10187,   607,   610, 10187, 10187, 10187, -1235,   580, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235,   505, -1235, -1235, -1235,   499,
   -1235, -1235,    47, 12265, -1235, 12325, 10187,   424,    -8, -1235,
   11487,   616, -1235,   424,    85,    87,   502,   503,  1799,   504,
   12325,   571,   508,   424,    88,   509, 12249, 12325, -1235, -1235,
     640,  2519,   -37, -1235, -1235, -1235,   312, -1235, -1235, -1235,
   -1235, 10187,   582,   543,   165, -1235,   581,   698,   517, 11323,
   11323,   695,    54,   650,   116, -1235, -1235, -1235, -1235, -1235,
   -1235,  2595, -1235, -1235, -1235, -1235,    51, 11871,   523,   706,
   12845,   705, -1235, -1235,   602, 12128, 12885, 12922, 11561, 10187,
   12804, 12254, 12943,  7496,  2101,  1889,  3613,  3613,  3613,  3613,
    1486,  1486,  1486,  1486,   750,   750,   477,   477,   477,   486,
     486,   486, -1235, 12845,   526,   528, 12601,   536,   727, 10187,
     -68,   551,   122, -1235, -1235, -1235,   427, 11675, 10187, -1235,
   -1235, 11561, -1235, 11561, 11561, 11561, 11561, 11561, 11561, 11561,
   11561, 11561, 11561, 11561, 10187,   -68,   552,   547,  2853,   554,
     555,  2907,    90,   559, -1235, 11576, -1235, 12325, -1235,   454,
      54,   375, 11871, 12845, 11871, 12642,    57,   126, -1235,   560,
   10187, -1235, -1235, -1235,  6179,   143, 12845,    10, -1235, -1235,
   -1235, 10187, 11339, 11487, 12325,  6767,   561,   570, -1235,    52,
     627, -1235,   757,   579,  3537, 11323, 11487, 11487, 11487,   583,
      25,   628,   584,   586,   234, -1235,   630, -1235,   588, -1235,
   -1235, -1235, 10187,   604, 12845,   608,   761, 11097,   775, -1235,
   12845,  3761, -1235,   505,   711, -1235,  4283, 12189,   597, 12325,
   -1235, -1235,  2992, -1235, -1235,   774, 11871,   600, -1235, -1235,
   -1235, -1235, -1235,   710,    53, 12189,   609, 11773, 11822,   790,
   -1235, -1235, -1235, -1235,   612, -1235, 10187, -1235, -1235,  3333,
   -1235, 12845, 12189,   617, -1235, -1235, -1235, -1235,   791, 10187,
     549, -1235, -1235,   615, -1235, 11323, 11083, -1235,   127, -1235,
   -1235, 11323, -1235,   424, -1235,  9833, -1235, 11487,    17,   599,
   12189,   582, -1235, -1235, 12761, 10187, -1235, -1235, 10187, -1235,
   10187, -1235,  3272,   621,  8853,   571,   582,   602, 12325, 11920,
     424,  3594,   623,  8853, -1235, -1235,   128, -1235, -1235,   801,
   12009, 12009, 11576, -1235, -1235, -1235,   626,    36,   629,   632,
   -1235, -1235, -1235,   808,   633,   471,   424,   424, 10029, -1235,
     133, -1235, -1235,  3718,   282,    10,  7317, -1235,   631,  4441,
     634, 11871,   635,   697,   424, -1235,   813, -1235, -1235, -1235,
   -1235,   396, -1235,   135, 11323, -1235, 11323,   595, -1235, -1235,
   -1235,   819,   638,   641, -1235, -1235,   713,   642,   828, 11487,
     702, 12325,   549, 11487, 12325, 11487, 12845, 10187, 10187, 10187,
   -1235, -1235, -1235, 10187, 10187, -1235,   580, -1235,   769, -1235,
   -1235, -1235, -1235, 11487,   424, -1235, 11323, 12325, -1235,   835,
   -1235, -1235,    91,   655,   424,  7829, -1235,  2288, -1235,  3809,
     835, -1235,   -38,   -27, 12845,   723, -1235,   656, 10187,   -68,
     657, -1235, 11871, 12845, -1235, -1235,   658,   841, -1235, 11323,
      17, -1235,   659,    17,   661, 12761, 12845, 12697,   665,  8853,
     666,   663,   667,   582, -1235,   400,   668,  8853,   670, 10187,
   -1235, -1235, -1235, -1235, -1235,   739,   671,   854, 11576,   729,
   -1235,   549, 11576, 11576, -1235, -1235, -1235, 11871, 12845, -1235,
      10,   838,   800,  7317, -1235, -1235,   678, 10187,   424, 11773,
   11339,   681, 11487,  4599,   450,   683, 10187,    31,   241, -1235,
     709, -1235, -1235, 11254,   850, -1235, 11487, -1235, 11487, -1235,
     689, -1235,   760,   876,   699,   700, -1235,   763,   703,   880,
   12845, 11180, 12845, -1235, 12845, -1235,   701, -1235, -1235, -1235,
     805, 12189,  1089, -1235, 11773, -1235, -1235, 11561,   704, -1235,
    1201, -1235,    71, 10187, -1235, -1235, -1235, 10187, -1235, 10187,
   -1235, 10683,   712,  8853,   424,   872,   112, -1235, -1235,   305,
     714, -1235,   715,    17, 10187,  8853,   716, -1235, -1235, -1235,
     707,   719, -1235,  8853,   721, -1235, 11576, -1235, 11576, -1235,
     722, -1235,   778,   724,   895,   726, -1235,   424,   883, -1235,
     728, -1235, -1235,   731,    92, -1235, -1235, -1235,   732,   734,
   -1235, 10806, -1235, -1235, -1235, -1235, -1235, -1235, 11323, -1235,
     795, -1235, 11487,   549, -1235, -1235, -1235, 11487, -1235, 11487,
   -1235, 10187,   730,  4757, 11323, -1235,   236, 11323, 12189, -1235,
   12207,   768, 12066, -1235, -1235, -1235,   558, 11203,    66,   527,
      93, -1235, -1235,   767, 10724, 10765, 12845,   737,  8853,   740,
   11323,   823, -1235, 11323,   858,   920,   872,  1401,   872,   744,
   12845,   745, -1235,  1461,  1515, -1235,    17,   746, -1235, -1235,
     815, -1235, 11576, -1235,   549, -1235, -1235, -1235,  6179, -1235,
   -1235, -1235,  6963, -1235, -1235, -1235,  6179,   751, 11487, -1235,
     818, -1235,   820,  3321, -1235, -1235, -1235, 12189, 12189,   930,
      59, -1235, -1235, -1235,    67,   752,    69, -1235, 10503, -1235,
   -1235,    70, -1235, -1235, 12024, -1235,   754, -1235,   875,   427,
   -1235, 11323, -1235,   558, -1235, -1235, -1235, -1235, -1235,  8853,
     762, -1235, -1235,   764,   758,   308,   941, 11487,   -58, -1235,
     823,   872, -1235, -1235, -1235,  1588,   765, -1235, 11576, -1235,
     834,  6179,  7159, -1235, -1235, -1235,  6179, -1235, -1235, 11487,
   11487, 10187,  4915,   771,   772, 11487, 12189, -1235, -1235,   448,
   12207, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235,   319, -1235,   768, -1235, -1235, -1235, -1235, -1235,
      75,   101, -1235,   945,    72, 12325,   875,   946,   427,   777,
   -1235,   309, -1235,   891,   952, 11487, -1235, -1235, -1235, -1235,
     779,   -58, -1235,   872, -1235, 11576, -1235, -1235, -1235,  5073,
   -1235, -1235, 11138, -1235, -1235, -1235, -1235, -1235, 11394,    48,
   -1235, -1235, 11487, 10503, 10503,   918, -1235, 12024, 12024,   105,
   -1235, -1235, -1235, 11487,   897, -1235,   786,    77, 11487, 12325,
   -1235,   919, -1235,   983,  5231,   980, 11487, -1235,  5389, -1235,
   -1235,   -58, -1235,  5547,   803,   923,   898, -1235,   907,   859,
   -1235, -1235,   911,   448, -1235, -1235, -1235, -1235,   849, -1235,
     974, -1235, -1235, -1235, -1235, -1235,   991, -1235, -1235, -1235,
     816, -1235,   320,   817, -1235, 11487, -1235, -1235,  5705, -1235,
     821, -1235, -1235,   824,   851, 12325,   527, -1235, -1235, 11487,
     131, -1235,   935, -1235, -1235, -1235, -1235, -1235, 12189,   597,
   -1235,   861, 12325,   405, -1235, -1235,   830,  1006,   449,   131,
   -1235,   948, -1235, 12189,   827, -1235,   872,   139, -1235, -1235,
   -1235, -1235, 11323, -1235,   832,   836,    83, -1235,   246, -1235,
     449,   322,   872,   833, -1235, -1235, -1235, -1235, 11323,   954,
    1015,   246, -1235,  5863,   330,  1016, 11487, -1235,  6021, -1235,
     957,  1020, 11487, -1235, -1235,  1022, 11487, -1235, 11487, -1235,
   -1235
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1235, -1235, -1235,  -458, -1235, -1235, -1235,    -4, -1235,   563,
     -12,  1018,  1235, -1235,  1632, -1235,  -400, -1235,    14, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,  -418,
   -1235, -1235,  -170,     8,     1, -1235, -1235, -1235,     4, -1235,
   -1235, -1235, -1235,     7, -1235, -1235,   673,   676,   677,   886,
     274,  -654,   280,   325,  -419, -1235,    98, -1235, -1235, -1235,
   -1235, -1235, -1235,  -519,   -10, -1235, -1235, -1235, -1235,  -410,
   -1235,  -860, -1235,  -297, -1235, -1235,   572, -1235,  -831, -1235,
   -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235, -1235,  -178,
   -1235, -1235, -1235, -1235, -1235,  -261, -1235,   -36,  -921, -1235,
   -1234,  -437, -1235,  -152,    -2,  -123,  -425, -1235,  -267, -1235,
     -70,   -20,  1023,  -604,  -342, -1235, -1235,   -34, -1235, -1235,
    2822,    15, -1235, -1235,  -675, -1235, -1235, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235,   693, -1235, -1235,   206, -1235,
     611, -1235, -1235, -1235, -1235, -1235, -1235, -1235,   210, -1235,
     613, -1235, -1235,   381,  -134, -1235,   189, -1235, -1235, -1235,
   -1235, -1235, -1235, -1235, -1235,  -882, -1235,  1315,   478,  -325,
   -1235, -1235,   154,   614,  -367, -1235, -1235,  -703,  -373,  -874,
   -1235, -1235,   292,  -611,  -773, -1235, -1235, -1235, -1235, -1235,
     281, -1235, -1235, -1235,  -669,  -987,  -184,  -181,  -128, -1235,
   -1235,    23, -1235, -1235, -1235, -1235,   -11,  -148, -1235,  -239,
   -1235, -1235, -1235,  -370,   798, -1235, -1235, -1235, -1235, -1235,
     467,   347, -1235, -1235,   806, -1235, -1235, -1235,  -308,   -77,
    -191,  -275, -1235, -1072, -1235,   250, -1235, -1235, -1235,  -203,
   -1082
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -780
static const yytype_int16 yytable[] =
{
     117,   365,   130,   393,   792,   126,   541,   323,   127,   291,
     222,   128,   125,   294,   295,   391,   249,   215,   124,   411,
     226,   634,   612,  1017,   230,   589,   618,   155,   386,  1113,
     572,  1195,   416,   436,   437,   417,   536,   810,   442,   297,
    1020,   259,   233,  1022,  1198,   242,  1200,   201,   202,   719,
    1000,   323,   320,   646,   314,   885,  1403,  1062,    11,   741,
     824,   867,   272,   286,   423,   260,   287,  1226,   686,   473,
     478,   481,  -675,  -678,  1181,  -251,   670,  1230,  1314,   898,
    1374,   388,   272,  1367,   381,  1374,   272,   272,   644,   418,
     609,  1226,   381,   696,   311,   696,   708,   902,   708,   708,
     708,   708,    35,   840,  1368,   277,    35,  1390,   442,  1001,
     503,   469,   912,   694,   929,   384,   629,  1103,   384,   575,
    1006,  -779,  1002,   381,   299,   272,  1370,   401,   823,  1341,
    1412,   475,   300,  1337,  1338,   496,   970,   689,   322,   409,
     975,   266,   384,  1371,   731,   607,  1372,  1413,     3,   610,
    1414,   366,   545,    11,  1003,  1040,   718,  1429,   178,  1045,
    -561,   524,   279,  1119,  -676,  1007,   218,   972,   470,  -677,
     280,   281,    11,   930,   895,   736,  -713,  -682,   415,   398,
      11,  -679,   394,   388,  -714,   310,   261,  -716,  -680,  -681,
    1117,   310,   627,   264,  -715,   497,   265,  1123,  1124,   221,
     486,  -684,   227,  -675,  -678,   761,   292,  -198,  -560,   805,
     806,   725,   389,   582,   742,   825,   868,   292,   117,   647,
     402,   117,  1063,   390,   613,   430,   404,   112,   956,   957,
     534,   898,   410,   582,   898,   649,   422,   315,   809,   425,
     543,  1404,   444,   323,   472,   155,   385,   424,   155,   385,
    1227,  1228,   474,   479,   482,   582,  1042,  1182,  -251,  1030,
    1231,  1315,   879,  1375,   582,  1369,  1206,   582,  1418,  1112,
     645,  1391,   506,   385,  1484,   697,   815,   698,   709,   828,
     779,   993,  1141,  1184,   477,   487,   291,   320,  1455,   834,
     835,   483,   483,   488,  1205,  -676,  1479,   117,   493,   130,
    -677,   954,  -610,   958,   502,  -685,   535,  -713,  -682,   125,
     242,   726,  -679,   272,   389,  -714,   733,   734,  -716,  -680,
    -681,  -186,  1046,  1099,   155,  -715,   727,   228,   959,  -610,
     619,  1099,  1362,   590,   956,   957,  1114,   872,   737,  1333,
    1381,   738,   277,  1053,   215,  1363,   229,   898,   940,   941,
     581,  1442,   253,  1489,   898,  1157,  1158,   580,   272,   272,
     272,  1500,  1364,   277,   268,   310,   269,   277,   503,  -779,
     606,   270,   278,   586,  -779,    48,   271,   605,  1115,   831,
     894,  1334,  1382,    55,    56,    57,   167,   168,   321,   763,
    1481,   311,   581,  1443,  1478,  1490,   311,   277,  1150,   621,
     275,   628,   303,  1501,   632,   276,  1494,   280,   281,   293,
    1491,   631,   508,   509,   772,   493,   768,   953,  -779,   277,
     934,  -779,   117,   402,   503,  1129,  -779,  1130,   280,   281,
     763,   279,   280,   281,  1065,   277,   737,  1485,  1486,   738,
     639,   302,   799,   309,   915,   800,  1410,  1411,  1099,   155,
      94,   442,   833,  1458,  1099,  1099,   310,   898,   316,  1210,
     313,   259,   280,   281,   277,   969,   324,   910,   753,   306,
     325,   691,  1458,   411,   277,   326,   918,   327,   977,   503,
    1480,  1406,  1407,   504,   280,   281,   703,   533,   955,   956,
     957,   328,   713,   715,   329,  -417,   988,   357,   801,   498,
     280,   281,   200,   200,   358,   359,   212,   360,   361,  1423,
     387,    55,    56,    57,   167,   168,   321,    31,    32,  -418,
    -683,  1209,   352,   353,   354,  -560,   355,    37,   392,   280,
     281,   284,   887,   995,   397,   355,  1099,   311,   891,   280,
     281,   272,  1059,   956,   957,   381,   403,    55,    56,    57,
      58,    59,   321,   406,  1039,  1469,  1470,  1471,    65,   362,
     743,   395,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,    69,    70,    71,    72,    73,    94,  1054,
     305,   307,   308,  1468,   660,  1057,   407,  1083,   582,   413,
      76,    77,  -559,   412,   415,   363,  1465,  1344,   434,  1069,
    1074,  1070,   438,   439,    87,  -774,  1080,   379,   380,   443,
     770,   790,  1026,   795,    94,   445,   446,   808,   480,    92,
    1034,   960,   448,   961,  1100,   499,   490,   489,   515,   505,
     117,   519,   130,    55,    56,    57,   167,   168,   321,   532,
     816,   117,   125,   520,   522,   796,   510,   797,   -51,  1138,
     499,   523,   505,   499,   505,   505,    48,   155,   542,   818,
     615,   617,   620,   989,  1146,   814,   626,   637,   155,   381,
     423,  1135,   641,   643,   650,   200,   654,   655,   672,   673,
     675,   200,   117,   676,   130,   861,   684,   200,   687,   695,
     705,   914,   699,   700,   125,   702,  1018,   707,  1012,   710,
      94,   716,   722,   729,   724,  1149,   730,   732,   735,   155,
    1151,  -419,  1152,   744,   745,   117,  1109,   130,   747,   864,
     126,   750,   756,   127,   757,   890,   128,   125,  1121,   759,
     493,   874,   889,   124,  1392,   760,  1127,  1211,   524,   200,
     764,   773,   155,   774,   776,  1216,   200,   200,   752,   826,
     802,   777,   820,   200,  1222,   449,   450,   451,   215,   200,
     891,   822,   452,   453,   272,   827,   454,   455,   841,   829,
     844,   898,   839,   842,   849,   843,   922,   922,   790,  1159,
     845,  1218,   847,   853,   856,   943,   848,   863,   900,   859,
     898,   865,   349,   350,   351,   352,   353,   354,   898,   355,
     866,   871,   117,   875,   883,   117,   886,   130,   876,   881,
     909,  1190,   917,  1349,   919,   928,   933,   125,   931,   950,
     944,   932,   945,   935,   949,   947,   952,   963,   964,   155,
    1336,   965,   155,   212,   948,   966,   968,   971,   967,   498,
     976,   996,   986,   991,   994,  1009,  1013,  1010,  1015,  1016,
    1023,  1021,  1350,  1351,  1025,  1028,  1027,  1033,  1356,  1029,
    1035,  1036,  1038,   990,  1049,  1147,  1041,  1037,  1050,  1052,
     200,  1056,  1066,  1327,  1060,   117,  1068,   130,   200,  1071,
     126,  1156,  1072,   127,  1073,  1077,   128,   125,  1079,  1075,
    1076,  1082,  1329,   124,  1180,  1084,  1078,  1101,  1110,  1125,
    1132,  1108,   155,  1134,  1116,  1118,  1122,  1192,  1126,  1137,
    1018,  1128,  1131,  1185,  1133,  1014,  1136,  1148,  1170,  1139,
    1048,  1140,  1154,  1143,   790,  1144,  1189,  1193,   790,   790,
    1191,  1196,  1384,  1197,  1201,  1202,  1207,  1208,  1388,   117,
    1219,  1217,  1220,  1225,  1229,  1322,  1393,  1183,  1323,   117,
    1332,   130,  1330,  1331,  1335,  1343,  1345,  1051,  1373,  1378,
    1047,   125,  1354,  1355,  1385,  1386,   155,  1380,  1387,  1408,
    1416,  1389,   493,   814,   323,  1417,   155,   367,   368,   369,
     370,   371,   372,   373,   374,   375,   376,   377,   378,  1428,
    1097,  1422,  1421,  1425,  1431,  1405,  1432,  -247,  1097,  1435,
    1433,  1436,  1438,  1368,  1439,  1441,  1415,  1444,  1460,  1325,
    1449,  1419,  1447,  1448,  1467,   200,  1463,   493,  1477,  1426,
    1466,  1475,  1482,   379,   380,  1492,  1483,  1495,  1496,  1502,
    1505,  1462,   790,  1506,   790,  1508,   690,   585,   583,   364,
     584,   913,   198,   198,  1476,   911,   210,   880,  1055,  1474,
     693,  1145,  1361,  1366,  1497,  1488,  1177,   669,  1445,  1377,
     592,   223,   985,   983,   855,   200,  1340,   210,   682,  1032,
     683,  1004,  1454,   924,   495,   485,   936,   962,     0,   117,
       0,   130,     0,   242,     0,   381,  1493,     0,  1175,     0,
       0,   125,  1498,  1086,   669,     0,     0,   366,     0,     0,
     200,     0,   200,     0,  1179,     0,   155,     0,     0,     0,
       0,     0,     0,     0,     0,  1097,     0,     0,     0,     0,
     200,  1097,  1097,     0,     0,     0,     0,     0,   790,  1503,
      11,     0,     0,     0,   117,  1507,   130,     0,   117,  1509,
       0,  1510,   117,     0,   130,     0,   125,     0,     0,     0,
       0,     0,     0,     0,   125,     0,  1214,     0,  1379,     0,
       0,   155,     0,     0,  1311,   155,     0,     0,     0,   155,
    1318,     0,     0,     0,   200,     0,     0,   242,     0,     0,
       0,     0,     0,     0,     0,   200,   200,     0,  1087,     0,
       0,  1328,     0,  1088,     0,    55,    56,    57,   167,   168,
     321,  1089,     0,  1097,   790,  1086,     0,   117,   117,   130,
       0,     0,   117,     0,   130,   198,     0,     0,   117,   125,
     130,   198,     0,     0,   125,     0,  1347,   198,     0,     0,
     125,     0,     0,     0,   155,   155,     0,  1090,  1091,   155,
    1092,     0,    11,     0,     0,   155,     0,   212,  1376,     0,
       0,     0,     0,     0,   210,   210,     0,   669,     0,   210,
       0,     0,    94,     0,     0,     0,     0,     0,     0,  1018,
     669,   669,   669,     0,     0,     0,     0,     0,     0,   198,
       0,     0,  1093,     0,  1452,  1018,   198,   198,     0,   200,
       0,     0,     0,   198,     0,     0,     0,     0,     0,   198,
    1087,     0,  1420,     0,     0,  1088,     0,    55,    56,    57,
     167,   168,   321,  1089,     0,     0,     0,     0,     0,     0,
       0,   272,     0,     0,     0,     0,     0,     0,     0,   210,
     323,     0,   210,     0,     0,     0,     0,     0,     0,   199,
     199,   790,     0,   211,     0,   117,     0,   130,     0,  1090,
    1091,     0,  1092,     0,  1398,     0,     0,   125,     0,  1311,
    1311,   669,     0,  1318,  1318,     0,     0,     0,     0,     0,
     200,     0,   155,   210,    94,   272,     0,     0,     0,     0,
     117,     0,   130,     0,   117,     0,   130,     0,     0,   117,
       0,   130,   125,     0,  1102,     0,   125,     0,     0,     0,
       0,   125,     0,     0,     0,  1086,     0,   155,     0,     0,
     198,   155,     0,     0,     0,   200,   155,     0,   198,     0,
       0,     0,     0,     0,   117,     0,   130,   200,   200,     0,
       0,  1451,     0,     0,  1453,     0,   125,     0,     0,     0,
       0,     0,    11,     0,     0,     0,     0,     0,  1464,     0,
       0,   155,     0,   669,     0,     0,   210,   669,     0,   669,
       0,   664,     0,     0,     0,  1086,     0,     0,     0,     0,
       0,     0,   200,     0,     0,     0,     0,   669,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   117,
       0,   130,     0,     0,   117,     0,   130,     0,   664,     0,
    1087,   125,    11,     0,     0,  1088,   125,    55,    56,    57,
     167,   168,   321,  1089,     0,     0,   155,     0,   199,  1086,
       0,   155,  -780,  -780,  -780,  -780,   347,   348,   349,   350,
     351,   352,   353,   354,     0,   355,     0,   210,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1090,
    1091,     0,  1092,     0,     0,   198,    11,     0,     0,     0,
    1087,     0,     0,     0,     0,  1088,   669,    55,    56,    57,
     167,   168,   321,  1089,    94,     0,   199,     0,     0,     0,
     669,     0,   669,   199,   199,     0,     0,     0,     0,     0,
     199,     0,  1086,     0,  1199,     0,   199,     0,     0,     0,
       0,     0,     0,     0,     0,   198,     0,     0,     0,  1090,
    1091,     0,  1092,     0,  1087,     0,     0,     0,     0,  1088,
       0,    55,    56,    57,   167,   168,   321,  1089,     0,    11,
       0,     0,     0,     0,    94,     0,     0,     0,     0,     0,
     198,     0,   198,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1203,     0,     0,     0,     0,     0,
     198,   664,     0,  1090,  1091,     0,  1092,     0,     0,     0,
     211,     0,   210,   210,   664,   664,   664,     0,   665,     0,
       0,   243,     0,     0,     0,     0,   669,  1087,    94,     0,
       0,   669,  1088,   669,    55,    56,    57,   167,   168,   321,
    1089,     0,     0,     0,     0,   210,     0,   199,  1204,     0,
       0,     0,     0,     0,   198,   665,     0,     0,     0,     0,
       0,     0,     0,   210,     0,   198,   198,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1090,  1091,     0,  1092,
     210,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   210,     0,     0,     0,     0,   668,   210,
       0,    94,   669,     0,     0,   664,     0,     0,   210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1342,     0,     0,     0,     0,     0,   210,     0,     0,
       0,     0,     0,     0,     0,   668,     0,     0,     0,   395,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,   669,   395,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,     0,     0,     0,     0,     0,   198,
       0,     0,     0,   669,   669,     0,     0,     0,     0,   669,
       0,     0,   210,  1360,   210,   379,   380,     0,     0,     0,
       0,     0,   199,     0,     0,     0,     0,   664,   379,   380,
       0,   664,     0,   664,     0,     0,     0,     0,   243,   243,
       0,     0,     0,   243,     0,     0,     0,     0,   665,     0,
       0,   664,     0,     0,   210,     0,     0,     0,     0,     0,
       0,   665,   665,   665,     0,     0,     0,     0,     0,     0,
       0,     0,   199,     0,     0,     0,     0,   381,     0,     0,
     198,     0,     0,     0,     0,     0,     0,   210,     0,     0,
     381,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,   199,   355,   199,
       0,     0,     0,   243,     0,     0,   243,     0,     0,   669,
       0,     0,     0,     0,     0,   198,     0,   199,   668,     0,
       0,     0,     0,     0,     0,     0,     0,   198,   198,     0,
     664,   668,   668,   668,     0,     0,   669,     0,     0,     0,
       0,   210,   665,     0,   664,     0,   664,   669,     0,   701,
       0,     0,   669,     0,     0,     0,     0,     0,     0,     0,
     669,     0,   858,     0,     0,     0,     0,     0,     0,   210,
       0,   199,   198,     0,     0,     0,     0,  1437,     0,     0,
     870,     0,   199,   199,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   870,     0,   669,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   669,     0,     0,     0,     0,     0,     0,
       0,     0,   668,     0,     0,   901,     0,     0,     0,     0,
     243,     0,     0,     0,   665,   666,     0,     0,   665,     0,
     665,     0,     0,     0,   211,     0,   210,     0,     0,     0,
     664,     0,     0,     0,     0,   664,     0,   664,   665,     0,
       0,     0,   210,     0,     0,   210,   210,     0,   210,     0,
     669,    33,   666,    35,     0,   210,   669,     0,     0,     0,
     669,     0,   669,     0,     0,     0,   199,     0,   210,     0,
       0,   210,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,   243,   243,     0,   668,     0,     0,     0,   668,     0,
     668,     0,     0,     0,     0,     0,   664,     0,     0,     0,
       0,     0,     0,     0,     0,   210,   210,     0,   668,     0,
       0,     0,     0,     0,     0,     0,     0,   665,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,     0,   210,
       0,   665,     0,   665,     0,     0,     0,   199,     0,     0,
       0,     0,     0,     0,    95,   664,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,     0,
       0,     0,   579,     0,   112,     0,     0,   664,   664,     0,
       0,     0,     0,   664,   210,     0,     0,     0,   210,     0,
       0,     0,   199,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   199,   199,     0,   668,     0,     0,
       0,     0,     0,     0,     0,   666,     0,     0,     0,     0,
       0,   668,     0,   668,     0,     0,   243,   243,   666,   666,
     666,     0,     0,     0,     0,     0,     0,   330,   331,   332,
       0,     0,     0,     0,     0,     0,  1085,   665,    33,   199,
      35,     0,   665,   333,   665,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,     0,   355,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   664,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   243,     0,     0,
       0,     0,     0,   243,     0,     0,   210,     0,     0,   666,
     664,     0,     0,   665,     0,    81,    82,   668,    83,    84,
      85,   664,   668,     0,   668,     0,   664,     0,     0,     0,
       0,     0,     0,  1161,   664,  1169,     0,     0,     0,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,   604,
       0,   112,   665,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   664,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   665,   665,   243,   664,   243,     0,
     665,     0,     0,   668,     0,     0,   210,     0,     0,     0,
       0,   666,  1223,  1224,     0,   666,     0,   666,     0,     0,
       0,   210,     0,     0,   998,     0,     0,     0,     0,     0,
     210,     0,     0,     0,     0,   666,     0,     0,   243,     0,
       0,     0,     0,     0,     0,     0,   210,     0,     0,     0,
       0,     0,   668,     0,   664,     0,     0,     0,     0,     0,
     664,     0,     0,     0,   664,     0,   664,     0,   330,   331,
     332,   243,     0,     0,   668,   668,     0,     0,     0,     0,
     668,  1357,     0,     0,   333,  1169,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,     0,   355,     0,
     665,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   666,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   243,     0,   665,   666,     0,
     666,     0,     0,     0,   330,   331,   332,     0,   665,     0,
       0,     0,     0,   665,     0,     0,     0,     0,     0,     0,
     333,   665,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,     0,   355,   236,     0,     0,     0,     0,
     668,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     665,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   237,     0,     0,   665,     0,     0,   668,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   668,     0,
       0,    33,     0,   668,     0,     0,     0,     0,     0,     0,
     243,   668,     0,     0,   666,     0,     0,     0,     0,   666,
       0,   666,   717,     0,     0,     0,   243,     0,   440,   243,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   243,
       0,   665,     0,     0,     0,     0,     0,   665,     0,     0,
     668,   665,   243,   665,   238,   243,     0,     0,     0,     0,
       0,     0,     0,     0,   668,     0,     0,     0,     0,     0,
       0,   171,     0,  1461,    78,     0,   239,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,  1161,     0,
     666,     0,     0,     0,     0,     0,   240,     0,   740,     0,
       0,     0,     0,     0,    95,     0,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   241,
       0,   668,     0,   243,     0,     0,     0,   668,     0,     0,
       0,   668,     0,   668,     0,     0,     0,   175,   177,   666,
     179,   180,   181,     0,   184,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,     0,     0,   204,   207,
       0,   666,   666,     0,     0,     0,     0,   666,     0,     0,
     224,     0,   330,   331,   332,     0,     0,   232,     0,   235,
       0,     0,   250,     0,   255,     0,     0,     0,   333,     0,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   289,   355,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   296,   330,   331,   332,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   298,   333,     0,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,     0,   355,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   666,     0,     0,
       0,     0,     0,     0,    33,     0,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1399,     0,     0,     0,   666,     0,     0,     0,     0,     0,
     396,   330,   331,   332,     0,   666,     0,     0,     0,     0,
     666,     0,     0,     0,     0,     0,     0,   333,   666,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     420,   355,     0,   420,     0,     0,   775,     0,     0,   224,
     429,    81,    82,     0,    83,    84,    85,   666,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   666,     0,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   298,     0,     0,   630,     0,   112,   204,     0,
     778,     0,   501,     0,   243,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     243,     0,     0,     0,   531,     0,     0,     0,   666,     0,
       0,     0,     0,     0,   666,   540,     0,     0,   666,     0,
     666,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   546,   547,   548,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,     0,     0,   573,
     573,     0,   576,     0,     0,   862,     0,     0,     0,   591,
     593,   594,   595,   596,   597,   598,   599,   600,   601,   602,
     603,     0,     0,     0,     0,     0,   573,   608,     0,   540,
     573,   611,     0,     0,     0,     0,     0,   591,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   623,     0,   625,
     330,   331,   332,     0,     0,   540,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   636,   333,     0,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,     0,   674,     0,     0,   677,   680,   681,     0,     0,
       0,   330,   331,   332,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   333,   692,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
       0,   355,     0,     0,     0,     0,     0,     0,     0,     0,
     330,   331,   332,   721,     0,     0,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,   333,     0,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,   754,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,   762,    31,    32,    33,    34,    35,     0,    36,     0,
     289,   614,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,   771,     0,    45,    46,
      47,    48,    49,    50,    51,     0,    52,    53,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,    64,
      65,    66,   803,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,   224,     0,   908,     0,     0,     0,    74,
       0,     0,     0,     0,    75,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,   846,     0,     0,     0,     0,    89,
      90,  1221,    91,     0,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,   110,   111,   878,   112,   113,     0,
     114,   115,     0,     0,     0,     0,     0,     0,   877,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   884,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   236,     0,     0,     0,     0,   893,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   905,     0,     0,
     906,     0,   907,     0,     0,     0,   540,     0,   237,     0,
       0,     0,     0,     0,     0,   540,     0,     0,     0,     0,
       0,     0,     0,   330,   331,   332,     0,     0,    33,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   333,
     938,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,     0,   355,     0,  -780,  -780,  -780,  -780,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   238,   355,     0,     0,     0,     0,     0,     0,   980,
     981,   982,     0,     0,     0,   677,   984,     0,   171,     0,
       0,    78,     0,   239,     0,    81,    82,     0,    83,    84,
      85,     0,     0,     0,   830,     0,     0,   997,     0,     0,
       0,     0,     0,   240,     0,     0,     0,     0,     0,     0,
    1011,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   241,   330,   331,   332,
       0,   540,     0,     0,     0,     0,     0,     0,     0,   540,
       0,   997,     0,   333,     0,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,     0,   355,     0,   224,
     330,   331,   332,     0,     0,     0,     0,     0,  1061,     0,
       0,     0,     0,     0,     0,     0,   333,   916,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,  1104,     0,     0,     0,  1105,
       0,  1106,     0,     0,     0,   540,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1120,   540,     0,     0,
      11,    12,    13,     0,     0,   540,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,  1153,    45,    46,    47,    48,    49,    50,
      51,   939,    52,    53,    54,    55,    56,    57,    58,    59,
      60,     0,    61,    62,    63,    64,    65,    66,     0,     0,
     540,     0,    67,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,   854,
      75,    76,    77,    78,    79,    80,     0,    81,    82,     0,
      83,    84,    85,    86,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,    90,     0,    91,    10,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     110,   111,   999,   112,   113,     0,   114,   115,    11,    12,
      13,   540,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,  1352,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,    49,    50,    51,     0,
      52,    53,    54,    55,    56,    57,    58,    59,    60,     0,
      61,    62,    63,    64,    65,    66,     0,     0,     0,     0,
      67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
       0,     0,     0,    74,     0,     0,     0,     0,    75,    76,
      77,    78,    79,    80,     0,    81,    82,     0,    83,    84,
      85,    86,     0,    87,     0,     0,     0,    88,     5,     6,
       7,     8,     9,    89,    90,     0,    91,    10,    92,    93,
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
     105,   106,   107,   108,   109,     0,   110,   111,   525,   112,
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
     107,   108,   109,     0,   110,   111,   857,   112,   113,     0,
     114,   115,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,   946,    41,     0,    42,     0,    43,
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
      39,    40,     0,    41,     0,    42,     0,    43,  1058,     0,
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
    1155,   112,   113,     0,   114,   115,    11,    12,    13,     0,
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
     105,   106,   107,   108,   109,     0,   110,   111,  1353,   112,
     113,     0,   114,   115,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
    1394,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,    55,
      56,    57,    58,    59,    60,     0,    61,    62,    63,     0,
      65,    66,     0,     0,     0,     0,    67,    68,    69,    70,
      71,    72,    73,     0,     0,     0,     0,     0,     0,    74,
       0,     0,     0,     0,   171,    76,    77,    78,    79,    80,
       0,    81,    82,     0,    83,    84,    85,    86,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,     0,   110,   111,     0,   112,   113,     0,
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
     109,     0,   110,   111,  1424,   112,   113,     0,   114,   115,
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
     110,   111,  1427,   112,   113,     0,   114,   115,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,  1430,    42,     0,    43,     0,     0,    44,     0,
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
     105,   106,   107,   108,   109,     0,   110,   111,  1446,   112,
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
     107,   108,   109,     0,   110,   111,  1499,   112,   113,     0,
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
     109,     0,   110,   111,  1504,   112,   113,     0,   114,   115,
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
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     110,   111,     0,   112,   113,     0,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   421,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   638,     0,     0,
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
       0,     0,     0,   817,     0,     0,     0,     0,     0,     0,
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
      85,     0,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,   110,   111,
       0,   112,   113,     0,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1213,
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
       0,     0,     0,     0,     0,  1346,     0,     0,     0,     0,
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
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
     110,   111,     0,   112,   113,     0,   114,   115,     0,    12,
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
      85,     0,     0,    87,     0,     0,     0,    88,     0,     0,
       0,     0,     0,    89,     0,     0,     0,     0,    92,    93,
      94,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,   110,   111,
       0,   112,   113,     0,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,   587,   355,     0,     0,     0,     0,
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
       0,     0,     0,     0,   171,    76,    77,    78,   588,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,    87,
       0,     0,     0,    88,     5,     6,     7,     8,     9,    89,
       0,     0,     0,    10,    92,    93,    94,    95,   247,    96,
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
       0,    10,    92,    93,    94,    95,   247,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,   248,     0,     0,   112,   113,     0,   114,   115,
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
     171,    76,    77,    78,   588,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       0,     0,     0,     0,     0,    89,     0,     0,     0,     0,
      92,    93,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,     0,   112,   113,     0,   114,   115,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   203,     0,     0,     0,
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
       0,    87,     0,     0,     0,    88,     5,     6,     7,     8,
       9,    89,     0,     0,     0,    10,    92,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,     0,   112,
     113,     0,   114,   115,     0,    12,    13,     0,     0,     0,
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
     107,   108,   109,     0,   231,     0,     0,   112,   113,     0,
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
     109,     0,   234,     0,     0,   112,   113,     0,   114,   115,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   288,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,   167,   168,
     169,     0,     0,    62,    63,     0,     0,     0,     0,     0,
       0,     0,   170,    68,    69,    70,    71,    72,    73,     0,
       0,     0,     0,     0,     0,    74,     0,     0,     0,     0,
     171,    76,    77,    78,     0,    80,     0,    81,    82,     0,
      83,    84,    85,     0,     0,    87,     0,     0,     0,    88,
       5,     6,     7,     8,     9,    89,     0,     0,     0,    10,
      92,     0,    94,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,     0,
       0,     0,     0,   112,   113,     0,   114,   115,     0,    12,
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
     103,   104,   105,   106,   107,   108,   109,   419,     0,     0,
       0,   112,   113,     0,   114,   115,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   537,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,   549,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,   587,     0,     0,     0,
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
       0,     0,   622,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,   624,     0,
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
       0,     0,     0,     0,   892,     0,     0,     0,     0,     0,
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
     937,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   112,   113,     0,   114,   115,     0,    12,
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
       0,     0,     0,     0,    31,    32,    33,   500,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,   167,   168,   169,     0,     0,    62,
      63,     0,     0,     0,     0,     0,     0,     0,   170,    68,
      69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
       0,    74,     0,     0,     0,     0,   171,    76,    77,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,    87,     0,     0,     0,    88,  1232,  1233,  1234,  1235,
    1236,    89,  1237,  1238,  1239,  1240,    92,     0,    94,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,     0,   112,
     113,     0,   114,   115,     0,     0,     0,     0,     0,     0,
       0,     0,  1241,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  1242,  1243,  1244,  1245,  1246,
    1247,  1248,     0,     0,    33,     0,     0,     0,     0,     0,
       0,     0,     0,  1249,  1250,  1251,  1252,  1253,  1254,  1255,
    1256,  1257,  1258,  1259,  1260,  1261,  1262,  1263,  1264,  1265,
    1266,  1267,  1268,  1269,  1270,  1271,  1272,  1273,  1274,  1275,
    1276,  1277,  1278,  1279,  1280,  1281,  1282,  1283,  1284,  1285,
    1286,  1287,  1288,  1289,     0,     0,  1290,  1291,  1292,  1293,
    1294,  1295,  1296,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1297,  1298,  1299,     0,  1300,     0,
       0,    81,    82,     0,    83,    84,    85,  1301,  1302,  1303,
       0,     0,  1304,     0,     0,     0,     0,     0,     0,  1305,
    1306,     0,  1307,     0,  1308,  1309,  1310,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   330,   331,   332,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   333,     0,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,     0,   355,   330,   331,   332,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   333,
       0,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,     0,   355,   330,   331,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     333,     0,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,     0,   355,   330,   331,   332,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   333,  1062,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,     0,   355,     0,     0,     0,   330,
     331,   332,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   333,  1107,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,     0,   355,
     330,   331,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   333,  1187,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,   330,   331,   332,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   333,  1188,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
       0,   355,   330,   331,   332,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1063,   333,     0,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,     0,   355,     0,   330,   331,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     333,   356,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,     0,   355,   330,   331,   332,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   333,   433,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,     0,   355,   330,   331,   332,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   333,   435,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,     0,   355,   330,   331,   332,
       0,     0,     0,     0,    33,     0,    35,     0,     0,     0,
       0,     0,     0,   333,   447,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,     0,   355,     0,   330,
     331,   332,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   333,   471,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,   348,   349,   350,   351,   352,   353,   354,   236,   355,
       0,    81,    82,     0,    83,    84,    85,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   633,     0,     0,     0,
       0,     0,     0,     0,   237,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,     0,     0,    33,   888,     0,   112,     0,   236,
       0,   850,   851,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  -288,     0,     0,     0,   237,     0,     0,     0,    55,
      56,    57,   167,   168,   321,     0,     0,     0,     0,  1395,
       0,     0,     0,     0,     0,    33,     0,   238,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   171,     0,     0,    78,   236,   239,
       0,    81,    82,     0,    83,    84,    85,     0,     0,  1081,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   240,
     811,     0,     0,     0,   237,     0,    94,    95,   238,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   241,     0,    33,   171,     0,     0,    78,     0,
     239,     0,    81,    82,     0,    83,    84,    85,     0,     0,
      33,  1067,    35,     0,     0,     0,     0,     0,     0,     0,
     240,     0,     0,     0,     0,     0,     0,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   241,     0,     0,     0,   238,     0,     0,
     196,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   812,     0,   171,    33,     0,    78,     0,   239,
       0,    81,    82,     0,    83,    84,    85,     0,     0,     0,
     171,     0,     0,    78,     0,    80,     0,    81,    82,   240,
      83,    84,    85,     0,     0,     0,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   241,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   197,   656,
     657,     0,     0,   112,     0,   171,     0,     0,    78,     0,
       0,     0,    81,    82,     0,    83,    84,    85,   658,     0,
       0,     0,     0,     0,     0,     0,    31,    32,    33,     0,
       0,     0,     0,     0,     0,     0,    37,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,     0,     0,     0,   333,  1397,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,   659,    69,    70,    71,    72,    73,     0,   780,   781,
       0,     0,     0,   660,     0,     0,     0,     0,   171,    76,
      77,    78,     0,   661,     0,    81,    82,   782,    83,    84,
      85,     0,     0,    87,     0,   783,   784,    33,     0,     0,
       0,     0,     0,   662,     0,   785,     0,     0,    92,     0,
       0,   663,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    27,    28,     0,     0,
       0,     0,     0,     0,     0,    33,     0,    35,     0,     0,
     786,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   787,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,     0,   196,     0,     0,   769,     0,
       0,     0,   788,     0,     0,     0,    33,     0,    35,     0,
     789,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   171,     0,     0,    78,     0,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,     0,    88,     0,   196,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,    35,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   400,     0,     0,   171,     0,   112,    78,
       0,    80,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,     0,     0,   196,     0,     0,     0,     0,
       0,     0,     0,     0,    33,     0,    35,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   197,   171,     0,     0,    78,   112,
      80,     0,    81,    82,     0,    83,    84,    85,     0,     0,
       0,     0,     0,     0,   196,     0,     0,     0,     0,     0,
       0,     0,     0,    33,     0,    35,   492,     0,    95,     0,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   197,   171,     0,   476,    78,   112,    80,
       0,    81,    82,     0,    83,    84,    85,     0,     0,     0,
       0,     0,     0,   196,     0,     0,     0,     0,     0,     0,
       0,     0,    33,     0,    35,   873,     0,    95,     0,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   197,   171,     0,     0,    78,   112,    80,     0,
      81,    82,     0,    83,    84,    85,     0,     0,     0,     0,
       0,     0,   196,     0,     0,     0,     0,     0,     0,     0,
       0,    33,     0,    35,     0,     0,    95,     0,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   197,   171,     0,     0,    78,   112,    80,     0,    81,
      82,     0,    83,    84,    85,     0,     0,     0,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,    33,     0,
       0,     0,     0,     0,     0,    95,     0,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     197,   171,     0,     0,    78,   112,    80,     0,    81,    82,
       0,    83,    84,    85,     0,     0,     0,     0,   920,   921,
      33,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    95,    33,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   209,
       0,   284,     0,     0,   112,    81,    82,     0,    83,    84,
      85,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    33,     0,     0,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,    33,    81,    82,   285,
      83,    84,    85,     0,     0,     0,     0,     0,     0,     0,
    1316,     0,    81,    82,  1317,    83,    84,    85,     0,     0,
       0,  1173,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,    95,    33,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,  1174,    81,    82,     0,    83,    84,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    81,    82,     0,    83,    84,    85,     0,
      95,     0,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,  1174,     0,     0,     0,    95,
      33,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   542,     0,    81,    82,    33,    83,
      84,    85,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,    95,   355,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   752,     0,  1162,
      33,     0,   711,   712,     0,     0,     0,     0,     0,     0,
     171,  1163,     0,    78,     0,    80,    33,    81,    82,     0,
      83,    84,    85,     0,     0,     0,     0,     0,   171,     0,
       0,    78,     0,  1164,     0,    81,    82,     0,    83,  1165,
      85,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,    33,
       0,    95,     0,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,    33,    81,    82,     0,
      83,    84,    85,     0,     0,     0,     0,     0,     0,   256,
       0,     0,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,     0,    95,     0,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   318,     0,    81,    82,     0,    83,
      84,    85,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    81,    82,     0,    83,    84,    85,     0,
       0,     0,    95,     0,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,     0,     0,    95,
       0,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   330,   331,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     333,     0,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,     0,   355,   330,   331,   332,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   333,     0,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351,   352,   353,   354,     0,   355,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     330,   331,   332,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   333,   405,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,   330,   331,   332,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   333,   511,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
       0,   355,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   330,   331,   332,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   333,   758,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,     0,   355,   330,   331,   332,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   333,   798,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,   348,
     349,   350,   351,   352,   353,   354,     0,   355,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,     0,
     355,     0,     0,   330,   331,   332,     0,     0,     0,  1024,
       0,     0,     0,     0,     0,     0,     0,     0,   642,   333,
     755,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   352,
     353,   354,     0,   355,   330,   331,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     333,     0,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,     0,   355,   331,   332,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     333,     0,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,   348,   349,   350,   351,
     352,   353,   354,   332,   355,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   333,     0,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
       0,   355,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,     0,   355
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-1235))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-780))

static const yytype_int16 yycheck[] =
{
       4,   153,     4,   173,   615,     4,   314,   130,     4,    86,
      30,     4,     4,    90,    91,   163,    50,    28,     4,   210,
      40,   421,   392,   897,    44,   367,   399,     4,   156,  1016,
     355,  1113,   216,   236,   237,   216,   311,   641,   241,   109,
     900,    53,    46,   903,  1116,    49,  1118,    24,    25,   507,
     881,   174,   129,    26,     8,   730,     8,    26,    41,     8,
       8,     8,    66,    75,     8,    73,    78,     8,   468,     8,
       8,     8,    61,    61,     8,     8,   443,     8,     8,   748,
       8,    61,    86,     8,   121,     8,    90,    91,     8,   217,
     387,     8,   121,     8,   162,     8,     8,   751,     8,     8,
       8,     8,    73,    78,    29,    73,    73,  1341,   311,   147,
      78,   102,   766,   480,    78,    61,   413,    46,    61,   358,
     147,   189,   160,   121,   109,   129,    25,   197,   647,  1201,
      25,    90,   109,   191,   192,    61,   839,    90,   130,   209,
     843,    73,    61,    42,   517,   384,    45,    42,     0,   388,
      45,   153,   322,    41,   192,   928,   193,  1391,   189,   932,
     140,   190,   137,  1023,    61,   192,   189,   842,   159,    61,
     138,   139,    41,   137,   157,   121,    61,    61,   121,   183,
      41,    61,   174,    61,    61,   144,   194,    61,    61,    61,
    1021,   144,   190,   117,    61,   121,   117,  1028,  1029,   189,
     270,   189,   189,   192,   192,   578,   146,   190,   140,    66,
      67,    46,   192,   361,   163,   163,   163,   146,   222,   192,
     197,   225,   191,   194,   394,   229,   203,   194,    93,    94,
     198,   900,   209,   381,   903,   438,   222,   191,   638,   225,
     317,   193,   246,   366,   256,   222,   192,   191,   225,   192,
     191,   192,   191,   191,   191,   403,   931,   191,   191,   913,
     191,   191,   720,   191,   412,   190,  1126,   415,   191,   157,
     190,  1343,   284,   192,   191,   190,   643,   190,   190,   652,
     190,   190,   190,   190,   261,   270,   363,   364,   157,   656,
     657,   268,   269,   270,  1125,   192,   157,   301,   275,   301,
     192,   820,   190,   822,   281,   189,   310,   192,   192,   301,
     314,   146,   192,   317,   192,   192,   519,   520,   192,   192,
     192,   190,   933,   992,   301,   192,   161,   189,   193,   190,
     400,  1000,    13,   367,    93,    94,    31,   707,   522,    31,
      31,   522,    73,   947,   355,    26,   189,  1016,    66,    67,
     361,    31,   192,    31,  1023,   119,   120,   361,   362,   363,
     364,    31,    43,    73,   189,   144,   189,    73,    78,   140,
     381,   189,    78,   365,   140,    98,   189,   381,    73,   654,
     747,    73,    73,   106,   107,   108,   109,   110,   111,   580,
    1472,   162,   403,    73,  1466,    73,   162,    73,  1073,   403,
     189,   412,    78,    73,   415,   189,  1488,   138,   139,   189,
    1482,   415,   191,   192,   605,   392,   586,   817,   189,    73,
     793,   192,   426,   400,    78,  1036,   192,  1038,   138,   139,
     621,   137,   138,   139,   193,    73,   620,   191,   192,   620,
     426,   192,   626,    26,   769,   626,  1367,  1368,  1117,   426,
     173,   654,   655,  1440,  1123,  1124,   144,  1126,    31,  1134,
     189,   473,   138,   139,    73,   838,   191,   764,   545,    78,
     191,   475,  1459,   664,    73,   191,   773,   191,   845,    78,
    1467,  1363,  1364,   137,   138,   139,   490,   197,    92,    93,
      94,   191,   496,   497,   191,    61,   863,    61,   626,   137,
     138,   139,    24,    25,    61,   191,    28,   192,   140,  1383,
     189,   106,   107,   108,   109,   110,   111,    69,    70,    61,
     189,  1132,    45,    46,    47,   140,    49,    79,   189,   138,
     139,   144,   735,   875,    40,    49,  1205,   162,   741,   138,
     139,   545,    92,    93,    94,   121,   140,   106,   107,   108,
     109,   110,   111,   196,   927,   106,   107,   108,   117,   118,
     537,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,   125,   126,   127,   128,   129,   173,   949,
     113,   114,   115,  1457,   136,   952,     8,   987,   736,   189,
     142,   143,   140,   140,   121,   154,   191,  1208,   191,   966,
     973,   968,   162,   189,   156,    13,   979,    59,    60,    13,
     587,   615,   909,   617,   173,    73,   191,   637,    13,   171,
     917,   824,   191,   826,   994,   278,   162,   190,    79,   282,
     634,    13,   634,   106,   107,   108,   109,   110,   111,   195,
     644,   645,   634,    90,   190,   622,   191,   624,   189,  1049,
     303,   190,   305,   306,   307,   308,    98,   634,   189,   645,
     189,     8,   190,   866,  1064,   642,   190,    82,   645,   121,
       8,  1044,   191,    13,    79,   197,   189,     8,   178,    73,
      73,   203,   686,    73,   686,   689,   181,   209,   189,    73,
     119,   768,   190,   190,   686,   191,   899,   189,   889,   190,
     173,    61,   120,   122,   161,  1072,     8,   190,    13,   686,
    1077,    61,  1079,   190,     8,   719,  1013,   719,    13,   696,
     719,   119,   196,   719,   196,   736,   719,   719,  1025,   193,
     707,   708,   736,   719,  1345,     8,  1033,  1137,   190,   261,
     189,   189,   719,   196,   190,  1145,   268,   269,   189,   122,
     190,   196,   191,   275,  1154,   175,   176,   177,   769,   281,
     963,   191,   182,   183,   768,     8,   186,   187,   140,   190,
     140,  1440,   189,   189,    13,   189,   780,   781,   782,  1087,
     192,  1148,   178,     8,    73,   805,   178,    13,   189,   192,
    1459,   191,    42,    43,    44,    45,    46,    47,  1467,    49,
      90,   192,   806,    13,    13,   809,   191,   809,   196,   192,
     189,  1108,   189,  1213,    13,   189,     8,   809,   189,   122,
     806,   189,   191,   190,   189,   191,    13,     8,   190,   806,
    1197,   190,   809,   355,   811,   122,     8,   841,   196,   137,
     844,   875,    73,     8,   189,   122,   189,   191,   190,     8,
     189,   192,  1219,  1220,   189,   192,   190,   189,  1225,   192,
     190,   122,     8,   867,    26,  1068,   137,   196,    68,   191,
     392,   190,   163,  1181,   191,   879,    26,   879,   400,   190,
     879,  1084,   122,   879,     8,   122,   879,   879,     8,   190,
     190,   190,  1189,   879,  1097,    90,   193,   193,    26,   192,
     122,   189,   879,     8,   190,   190,   190,  1110,   189,    26,
    1113,   190,   190,   146,   190,   892,   190,   122,   150,   191,
     940,   190,   192,   191,   928,   191,   189,   104,   932,   933,
     190,    73,  1332,    13,   190,   190,   190,   122,  1338,   943,
     122,   190,   122,    13,   192,   191,  1346,  1099,    73,   953,
     192,   953,   190,   189,    13,   190,   122,   943,    13,    13,
     937,   953,   191,   191,    73,    13,   943,   190,  1335,    51,
      73,   192,   949,   950,  1097,   189,   953,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,  1389,
     992,     8,    73,    13,   191,  1362,    73,    90,  1000,   140,
     102,    90,   153,    29,    13,   189,  1373,   190,    73,  1179,
     159,  1378,   191,   189,     8,   537,   155,   994,   191,  1386,
     190,    73,   190,    59,    60,   192,   190,    73,    13,    13,
      73,  1449,  1036,    13,  1038,    13,   473,   364,   362,   153,
     363,   767,    24,    25,  1463,   765,    28,   722,   950,  1459,
     478,  1061,  1230,  1314,  1491,  1480,  1092,   443,  1425,  1326,
     367,    38,   856,   853,   683,   587,  1200,    49,   457,   915,
     457,   882,  1439,   781,   276,   269,   795,   827,    -1,  1083,
      -1,  1083,    -1,  1087,    -1,   121,  1486,    -1,  1092,    -1,
      -1,  1083,  1492,     4,   480,    -1,    -1,  1099,    -1,    -1,
     622,    -1,   624,    -1,  1096,    -1,  1083,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1117,    -1,    -1,    -1,    -1,
     642,  1123,  1124,    -1,    -1,    -1,    -1,    -1,  1132,  1496,
      41,    -1,    -1,    -1,  1138,  1502,  1138,    -1,  1142,  1506,
      -1,  1508,  1146,    -1,  1146,    -1,  1138,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1146,    -1,  1142,    -1,  1328,    -1,
      -1,  1138,    -1,    -1,  1168,  1142,    -1,    -1,    -1,  1146,
    1174,    -1,    -1,    -1,   696,    -1,    -1,  1181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   707,   708,    -1,    99,    -1,
      -1,  1183,    -1,   104,    -1,   106,   107,   108,   109,   110,
     111,   112,    -1,  1205,  1208,     4,    -1,  1211,  1212,  1211,
      -1,    -1,  1216,    -1,  1216,   197,    -1,    -1,  1222,  1211,
    1222,   203,    -1,    -1,  1216,    -1,  1212,   209,    -1,    -1,
    1222,    -1,    -1,    -1,  1211,  1212,    -1,   148,   149,  1216,
     151,    -1,    41,    -1,    -1,  1222,    -1,   769,  1325,    -1,
      -1,    -1,    -1,    -1,   236,   237,    -1,   643,    -1,   241,
      -1,    -1,   173,    -1,    -1,    -1,    -1,    -1,    -1,  1472,
     656,   657,   658,    -1,    -1,    -1,    -1,    -1,    -1,   261,
      -1,    -1,   193,    -1,  1436,  1488,   268,   269,    -1,   811,
      -1,    -1,    -1,   275,    -1,    -1,    -1,    -1,    -1,   281,
      99,    -1,  1379,    -1,    -1,   104,    -1,   106,   107,   108,
     109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1325,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   311,
    1453,    -1,   314,    -1,    -1,    -1,    -1,    -1,    -1,    24,
      25,  1345,    -1,    28,    -1,  1349,    -1,  1349,    -1,   148,
     149,    -1,   151,    -1,  1358,    -1,    -1,  1349,    -1,  1363,
    1364,   747,    -1,  1367,  1368,    -1,    -1,    -1,    -1,    -1,
     892,    -1,  1349,   355,   173,  1379,    -1,    -1,    -1,    -1,
    1384,    -1,  1384,    -1,  1388,    -1,  1388,    -1,    -1,  1393,
      -1,  1393,  1384,    -1,   193,    -1,  1388,    -1,    -1,    -1,
      -1,  1393,    -1,    -1,    -1,     4,    -1,  1384,    -1,    -1,
     392,  1388,    -1,    -1,    -1,   937,  1393,    -1,   400,    -1,
      -1,    -1,    -1,    -1,  1428,    -1,  1428,   949,   950,    -1,
      -1,  1435,    -1,    -1,  1436,    -1,  1428,    -1,    -1,    -1,
      -1,    -1,    41,    -1,    -1,    -1,    -1,    -1,  1452,    -1,
      -1,  1428,    -1,   839,    -1,    -1,   438,   843,    -1,   845,
      -1,   443,    -1,    -1,    -1,     4,    -1,    -1,    -1,    -1,
      -1,    -1,   994,    -1,    -1,    -1,    -1,   863,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1493,
      -1,  1493,    -1,    -1,  1498,    -1,  1498,    -1,   480,    -1,
      99,  1493,    41,    -1,    -1,   104,  1498,   106,   107,   108,
     109,   110,   111,   112,    -1,    -1,  1493,    -1,   203,     4,
      -1,  1498,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,   519,   520,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   148,
     149,    -1,   151,    -1,    -1,   537,    41,    -1,    -1,    -1,
      99,    -1,    -1,    -1,    -1,   104,   952,   106,   107,   108,
     109,   110,   111,   112,   173,    -1,   261,    -1,    -1,    -1,
     966,    -1,   968,   268,   269,    -1,    -1,    -1,    -1,    -1,
     275,    -1,     4,    -1,   193,    -1,   281,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   587,    -1,    -1,    -1,   148,
     149,    -1,   151,    -1,    99,    -1,    -1,    -1,    -1,   104,
      -1,   106,   107,   108,   109,   110,   111,   112,    -1,    41,
      -1,    -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,    -1,
     622,    -1,   624,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   193,    -1,    -1,    -1,    -1,    -1,
     642,   643,    -1,   148,   149,    -1,   151,    -1,    -1,    -1,
     355,    -1,   654,   655,   656,   657,   658,    -1,   443,    -1,
      -1,    49,    -1,    -1,    -1,    -1,  1072,    99,   173,    -1,
      -1,  1077,   104,  1079,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,   687,    -1,   392,   193,    -1,
      -1,    -1,    -1,    -1,   696,   480,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   705,    -1,   707,   708,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   148,   149,    -1,   151,
     722,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   735,    -1,    -1,    -1,    -1,   443,   741,
      -1,   173,  1148,    -1,    -1,   747,    -1,    -1,   750,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   193,    -1,    -1,    -1,    -1,    -1,   769,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   480,    -1,    -1,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,  1197,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,    -1,    -1,   811,
      -1,    -1,    -1,  1219,  1220,    -1,    -1,    -1,    -1,  1225,
      -1,    -1,   824,  1229,   826,    59,    60,    -1,    -1,    -1,
      -1,    -1,   537,    -1,    -1,    -1,    -1,   839,    59,    60,
      -1,   843,    -1,   845,    -1,    -1,    -1,    -1,   236,   237,
      -1,    -1,    -1,   241,    -1,    -1,    -1,    -1,   643,    -1,
      -1,   863,    -1,    -1,   866,    -1,    -1,    -1,    -1,    -1,
      -1,   656,   657,   658,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   587,    -1,    -1,    -1,    -1,   121,    -1,    -1,
     892,    -1,    -1,    -1,    -1,    -1,    -1,   899,    -1,    -1,
     121,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,   622,    49,   624,
      -1,    -1,    -1,   311,    -1,    -1,   314,    -1,    -1,  1335,
      -1,    -1,    -1,    -1,    -1,   937,    -1,   642,   643,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   949,   950,    -1,
     952,   656,   657,   658,    -1,    -1,  1362,    -1,    -1,    -1,
      -1,   963,   747,    -1,   966,    -1,   968,  1373,    -1,   190,
      -1,    -1,  1378,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1386,    -1,   687,    -1,    -1,    -1,    -1,    -1,    -1,   991,
      -1,   696,   994,    -1,    -1,    -1,    -1,  1403,    -1,    -1,
     705,    -1,   707,   708,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   722,    -1,  1425,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1439,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   747,    -1,    -1,   750,    -1,    -1,    -1,    -1,
     438,    -1,    -1,    -1,   839,   443,    -1,    -1,   843,    -1,
     845,    -1,    -1,    -1,   769,    -1,  1068,    -1,    -1,    -1,
    1072,    -1,    -1,    -1,    -1,  1077,    -1,  1079,   863,    -1,
      -1,    -1,  1084,    -1,    -1,  1087,  1088,    -1,  1090,    -1,
    1496,    71,   480,    73,    -1,  1097,  1502,    -1,    -1,    -1,
    1506,    -1,  1508,    -1,    -1,    -1,   811,    -1,  1110,    -1,
      -1,  1113,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,   519,   520,    -1,   839,    -1,    -1,    -1,   843,    -1,
     845,    -1,    -1,    -1,    -1,    -1,  1148,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1157,  1158,    -1,   863,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   952,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,  1181,
      -1,   966,    -1,   968,    -1,    -1,    -1,   892,    -1,    -1,
      -1,    -1,    -1,    -1,   174,  1197,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,    -1,
      -1,    -1,   192,    -1,   194,    -1,    -1,  1219,  1220,    -1,
      -1,    -1,    -1,  1225,  1226,    -1,    -1,    -1,  1230,    -1,
      -1,    -1,   937,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   949,   950,    -1,   952,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   643,    -1,    -1,    -1,    -1,
      -1,   966,    -1,   968,    -1,    -1,   654,   655,   656,   657,
     658,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,   991,  1072,    71,   994,
      73,    -1,  1077,    25,  1079,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1335,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   735,    -1,    -1,
      -1,    -1,    -1,   741,    -1,    -1,  1358,    -1,    -1,   747,
    1362,    -1,    -1,  1148,    -1,   148,   149,  1072,   151,   152,
     153,  1373,  1077,    -1,  1079,    -1,  1378,    -1,    -1,    -1,
      -1,    -1,    -1,  1088,  1386,  1090,    -1,    -1,    -1,    -1,
      -1,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,    -1,    -1,    -1,   192,
      -1,   194,  1197,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1425,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1219,  1220,   824,  1439,   826,    -1,
    1225,    -1,    -1,  1148,    -1,    -1,  1448,    -1,    -1,    -1,
      -1,   839,  1157,  1158,    -1,   843,    -1,   845,    -1,    -1,
      -1,  1463,    -1,    -1,   196,    -1,    -1,    -1,    -1,    -1,
    1472,    -1,    -1,    -1,    -1,   863,    -1,    -1,   866,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1488,    -1,    -1,    -1,
      -1,    -1,  1197,    -1,  1496,    -1,    -1,    -1,    -1,    -1,
    1502,    -1,    -1,    -1,  1506,    -1,  1508,    -1,     9,    10,
      11,   899,    -1,    -1,  1219,  1220,    -1,    -1,    -1,    -1,
    1225,  1226,    -1,    -1,    25,  1230,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
    1335,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   952,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   963,    -1,  1362,   966,    -1,
     968,    -1,    -1,    -1,     9,    10,    11,    -1,  1373,    -1,
      -1,    -1,    -1,  1378,    -1,    -1,    -1,    -1,    -1,    -1,
      25,  1386,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    25,    -1,    -1,    -1,    -1,
    1335,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1425,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    51,    -1,    -1,  1439,    -1,    -1,  1362,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1373,    -1,
      -1,    71,    -1,  1378,    -1,    -1,    -1,    -1,    -1,    -1,
    1068,  1386,    -1,    -1,  1072,    -1,    -1,    -1,    -1,  1077,
      -1,  1079,   193,    -1,    -1,    -1,  1084,    -1,    98,  1087,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1097,
      -1,  1496,    -1,    -1,    -1,    -1,    -1,  1502,    -1,    -1,
    1425,  1506,  1110,  1508,   124,  1113,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1439,    -1,    -1,    -1,    -1,    -1,
      -1,   141,    -1,  1448,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,  1463,    -1,
    1148,    -1,    -1,    -1,    -1,    -1,   166,    -1,   193,    -1,
      -1,    -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
      -1,  1496,    -1,  1181,    -1,    -1,    -1,  1502,    -1,    -1,
      -1,  1506,    -1,  1508,    -1,    -1,    -1,     5,     6,  1197,
       8,     9,    10,    -1,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    26,    27,
      -1,  1219,  1220,    -1,    -1,    -1,    -1,  1225,    -1,    -1,
      38,    -1,     9,    10,    11,    -1,    -1,    45,    -1,    47,
      -1,    -1,    50,    -1,    52,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    79,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    93,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   109,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1335,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1358,    -1,    -1,    -1,  1362,    -1,    -1,    -1,    -1,    -1,
     178,     9,    10,    11,    -1,  1373,    -1,    -1,    -1,    -1,
    1378,    -1,    -1,    -1,    -1,    -1,    -1,    25,  1386,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     218,    49,    -1,   221,    -1,    -1,   193,    -1,    -1,   227,
     228,   148,   149,    -1,   151,   152,   153,  1425,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1439,    -1,    -1,    -1,    -1,    -1,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   270,    -1,    -1,   192,    -1,   194,   276,    -1,
     193,    -1,   280,    -1,  1472,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1488,    -1,    -1,    -1,   302,    -1,    -1,    -1,  1496,    -1,
      -1,    -1,    -1,    -1,  1502,   313,    -1,    -1,  1506,    -1,
    1508,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   350,   351,   352,   353,   354,    -1,    -1,   357,
     358,    -1,   360,    -1,    -1,   193,    -1,    -1,    -1,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   377,
     378,    -1,    -1,    -1,    -1,    -1,   384,   385,    -1,   387,
     388,   389,    -1,    -1,    -1,    -1,    -1,   395,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   405,    -1,   407,
       9,    10,    11,    -1,    -1,   413,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   423,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,   450,    -1,    -1,   453,   454,   455,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   476,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       9,    10,    11,   511,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,   549,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,   579,    69,    70,    71,    72,    73,    -1,    75,    -1,
     588,   190,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,   604,    -1,    95,    96,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,   116,
     117,   118,   630,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,   641,    -1,   193,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,   672,    -1,    -1,    -1,    -1,   166,
     167,   180,   169,    -1,   171,   172,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,   191,   192,   193,   194,   195,    -1,
     197,   198,    -1,    -1,    -1,    -1,    -1,    -1,   716,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   729,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    -1,    -1,    -1,   745,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   755,    -1,    -1,
     758,    -1,   760,    -1,    -1,    -1,   764,    -1,    51,    -1,
      -1,    -1,    -1,    -1,    -1,   773,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     798,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,   124,    49,    -1,    -1,    -1,    -1,    -1,    -1,   847,
     848,   849,    -1,    -1,    -1,   853,   854,    -1,   141,    -1,
      -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,   157,    -1,    -1,   875,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,    -1,
     888,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,     9,    10,    11,
      -1,   909,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   917,
      -1,   919,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,   947,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,   956,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   193,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,  1003,    -1,    -1,    -1,  1007,
      -1,  1009,    -1,    -1,    -1,  1013,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1024,  1025,    -1,    -1,
      41,    42,    43,    -1,    -1,  1033,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,  1081,    95,    96,    97,    98,    99,   100,
     101,   193,   103,   104,   105,   106,   107,   108,   109,   110,
     111,    -1,   113,   114,   115,   116,   117,   118,    -1,    -1,
    1108,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,   188,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,   167,    -1,   169,    12,
     171,   172,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,   192,   193,   194,   195,    -1,   197,   198,    41,    42,
      43,  1189,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,  1221,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,   167,    -1,   169,    12,   171,   172,
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
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
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
      81,    82,    -1,    84,    -1,    86,    -1,    88,    89,    -1,
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
      87,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,   106,
     107,   108,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,   191,   192,    -1,   194,   195,    -1,
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
      -1,    84,    85,    86,    -1,    88,    -1,    -1,    91,    -1,
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
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
     191,   192,    -1,   194,   195,    -1,   197,   198,    -1,    42,
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
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,   171,   172,
     173,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,    -1,   191,   192,
      -1,   194,   195,    -1,   197,   198,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    31,    49,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,   173,   174,   175,   176,
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
      -1,    12,   171,   172,   173,   174,   175,   176,   177,   178,
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
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,
     171,   172,   173,   174,    -1,   176,   177,   178,   179,   180,
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
      -1,    -1,    -1,    12,   171,    -1,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,   191,    -1,    -1,   194,   195,    -1,
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
      -1,    -1,    -1,    -1,    95,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,    -1,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      -1,    -1,    -1,   194,   195,    -1,   197,   198,    -1,    42,
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
     183,   184,   185,   186,   187,   188,   189,   190,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,    -1,   173,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    -1,
      -1,    -1,    -1,   194,   195,    -1,   197,   198,    -1,    42,
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
       7,   166,     9,    10,    11,    12,   171,    -1,   173,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,    -1,    -1,    -1,   194,
     195,    -1,   197,   198,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   159,    -1,    -1,    -1,    -1,    -1,    -1,   166,
     167,    -1,   169,    -1,   171,   172,   173,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   193,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   193,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   193,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   191,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   191,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,   191,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   191,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    71,    -1,    73,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   191,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   191,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    25,    49,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   190,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,    -1,    -1,    71,   192,    -1,   194,    -1,    25,
      -1,   184,   185,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    51,    -1,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,    -1,    -1,   181,
      -1,    -1,    -1,    -1,    -1,    71,    -1,   124,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,    -1,    -1,   144,    25,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   179,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,
      31,    -1,    -1,    -1,    51,    -1,   173,   174,   124,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,    -1,    71,   141,    -1,    -1,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      71,   157,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     166,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    -1,    -1,    -1,   124,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,    -1,   141,    71,    -1,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,
     141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,   166,
     151,   152,   153,    -1,    -1,    -1,    -1,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,    42,
      43,    -1,    -1,   194,    -1,   141,    -1,    -1,   144,    -1,
      -1,    -1,   148,   149,    -1,   151,   152,   153,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    79,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,    -1,    -1,    -1,    25,   193,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,   124,   125,   126,   127,   128,   129,    -1,    42,    43,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    61,   151,   152,
     153,    -1,    -1,   156,    -1,    69,    70,    71,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    79,    -1,    -1,   171,    -1,
      -1,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,    -1,
     124,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    63,    -1,
      -1,    -1,   166,    -1,    -1,    -1,    71,    -1,    73,    -1,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   141,    -1,    -1,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      -1,    -1,    -1,    -1,   160,    -1,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,    73,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,    -1,    -1,   141,    -1,   194,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    73,    -1,    -1,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   141,    -1,    -1,   144,   194,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    71,    -1,    73,   123,    -1,   174,    -1,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   141,    -1,   192,   144,   194,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    73,   123,    -1,   174,    -1,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   141,    -1,    -1,   144,   194,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    -1,    73,    -1,    -1,   174,    -1,   176,   177,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
     188,   189,   141,    -1,    -1,   144,   194,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,
      -1,    -1,    -1,    -1,    -1,   174,    -1,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   188,
     189,   141,    -1,    -1,   144,   194,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    69,    70,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   174,    71,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
      -1,   144,    -1,    -1,   194,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      -1,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,    71,   148,   149,   192,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     146,    -1,   148,   149,   150,   151,   152,   153,    -1,    -1,
      -1,   115,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   174,    71,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   148,   149,    -1,   151,   152,   153,    -1,
     174,    -1,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,    -1,    -1,    -1,   174,
      71,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,    -1,   148,   149,    71,   151,
     152,   153,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,   174,    49,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,    -1,   112,
      71,    -1,    73,    74,    -1,    -1,    -1,    -1,    -1,    -1,
     141,   124,    -1,   144,    -1,   146,    71,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,   141,    -1,
      -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,    -1,    71,
      -1,   174,    -1,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,    71,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,   144,
      -1,    -1,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,    -1,    -1,   174,    -1,   176,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,    -1,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,    -1,   174,    -1,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,    -1,    -1,   174,
      -1,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   122,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   122,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   122,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   122,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,   122,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    11,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49
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
     377,   179,   190,   215,    90,   366,     4,    99,   104,   112,
     148,   149,   151,   193,   278,   301,   302,   303,   308,   393,
     412,   193,   193,    46,   319,   319,   319,   193,   189,   272,
      26,   432,   157,   394,    31,    73,   190,   277,   190,   270,
     319,   272,   190,   277,   277,   192,   189,   272,   190,   382,
     382,   190,   122,   190,     8,   377,   190,    26,   215,   191,
     190,   190,   222,   191,   191,   263,   215,   438,   122,   373,
     323,   373,   373,   319,   192,   193,   438,   119,   120,   427,
     253,   366,   112,   124,   146,   152,   287,   288,   289,   366,
     150,   293,   294,   115,   189,   206,   295,   296,   279,   232,
     438,     8,   191,   302,   190,   146,   357,   193,   193,   189,
     272,   190,   438,   104,   353,   439,    73,    13,   432,   193,
     432,   190,   190,   193,   193,   277,   270,   190,   122,   382,
     323,   215,   220,    26,   217,   257,   215,   190,   373,   122,
     122,   180,   215,   366,   366,    13,     8,   191,   192,   192,
       8,   191,     3,     4,     5,     6,     7,     9,    10,    11,
      12,    49,    62,    63,    64,    65,    66,    67,    68,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     123,   124,   125,   126,   127,   128,   129,   141,   142,   143,
     145,   154,   155,   156,   159,   166,   167,   169,   171,   172,
     173,   206,   363,   364,     8,   191,   146,   150,   206,   296,
     297,   298,   191,    73,   307,   231,   280,   427,   232,   272,
     190,   189,   192,    31,    73,    13,   373,   191,   192,   299,
     353,   432,   193,   190,   382,   122,    26,   217,   256,   215,
     373,   373,   319,   193,   191,   191,   373,   366,   283,   290,
     372,   288,    13,    26,    43,   291,   294,     8,    29,   190,
      25,    42,    45,    13,     8,   191,   428,   307,    13,   231,
     190,    31,    73,   354,   215,    73,    13,   373,   215,   192,
     299,   432,   382,   215,    87,   181,   227,   193,   206,   213,
     284,   285,   286,     8,   193,   373,   364,   364,    51,   292,
     297,   297,    25,    42,    45,   373,    73,   189,   191,   373,
     428,    73,     8,   378,   193,    13,   373,   193,   215,   299,
      85,   191,    73,   102,   223,   140,    90,   372,   153,    13,
     281,   189,    31,    73,   190,   373,   193,   191,   189,   159,
     230,   206,   302,   303,   373,   157,   268,   269,   394,   282,
      73,   366,   228,   155,   206,   191,   190,     8,   378,   106,
     107,   108,   305,   306,   268,    73,   253,   191,   432,   157,
     394,   439,   190,   190,   191,   191,   192,   300,   305,    31,
      73,   432,   192,   215,   439,    73,    13,   300,   215,   193,
      31,    73,    13,   373,   193,    73,    13,   373,    13,   373,
     373
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
#line 1369 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), true); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1371 "hphp.y"
    { _p->onTraitRequire((yyval), (yyvsp[(3) - (4)]), false); }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1372 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1375 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1378 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1379 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1380 "hphp.y"
    { (yyval).reset(); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1386 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1390 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1393 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1400 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1401 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1406 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1409 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1416 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1418 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1422 "hphp.y"
    { (yyval) = 4;}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1423 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1429 "hphp.y"
    { (yyval) = 6;}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1431 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1435 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1437 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1441 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1442 "hphp.y"
    { scalar_null(_p, (yyval));}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1446 "hphp.y"
    { scalar_num(_p, (yyval), "1");}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1447 "hphp.y"
    { scalar_num(_p, (yyval), "0");}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1451 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1454 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1459 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1464 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1465 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1467 "hphp.y"
    { (yyval) = 0;}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1471 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1472 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1473 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1474 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1478 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1479 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1480 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1481 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1482 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1484 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1486 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1490 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1493 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1494 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1498 "hphp.y"
    { (yyval).reset();}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1499 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1503 "hphp.y"
    { (yyval).reset();}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1504 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;}
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
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1512 "hphp.y"
    { (yyval).reset();}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1520 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1521 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1522 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1523 "hphp.y"
    { (yyval) = T_STATIC;}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1524 "hphp.y"
    { (yyval) = T_ABSTRACT;}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1525 "hphp.y"
    { (yyval) = T_FINAL;}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1526 "hphp.y"
    { (yyval) = T_ASYNC;}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1530 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval).reset();}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1534 "hphp.y"
    { (yyval) = T_PUBLIC;}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = T_PROTECTED;}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval) = T_PRIVATE;}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1540 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1542 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1543 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1553 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1555 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1556 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1557 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1558 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 1561 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 1566 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 1570 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 1571 "hphp.y"
    { (yyval).reset();}
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 1575 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));}
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 1576 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 1580 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 1585 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 1589 "hphp.y"
    { _p->onAwait((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 1593 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 1598 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);}
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 1602 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 1603 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 1604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 1608 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));}
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 1609 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 1610 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);}
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 1613 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));}
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 1614 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);}
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 1615 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);}
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 1616 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);}
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 1617 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);}
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 1618 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);}
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 1619 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);}
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 1620 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);}
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 1621 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);}
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 1622 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);}
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 1623 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);}
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 1624 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);}
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 1625 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);}
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 1626 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);}
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 1627 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);}
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 1628 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);}
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 1629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);}
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 1630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);}
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 1631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);}
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 1632 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);}
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 1633 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);}
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 1634 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');}
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 1635 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');}
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 1636 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');}
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 1637 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');}
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 1638 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');}
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 1639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');}
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 1640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');}
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 1641 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');}
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 1642 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');}
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 1643 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);}
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 1644 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);}
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 1645 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 1646 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 1647 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);}
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 1648 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);}
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 1649 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);}
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 1650 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);}
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 1651 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);}
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 1652 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);}
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 1653 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');}
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 1654 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);}
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 1656 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');}
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 1657 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);}
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 1660 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);}
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 1661 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 1662 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));}
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 1663 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));}
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 1665 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);}
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 1666 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);}
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 1667 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);}
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 1668 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);}
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 1669 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);}
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 1670 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);}
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 1671 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);}
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 1672 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);}
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 1673 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);}
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 1674 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 1675 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 1676 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 1677 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));}
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 1678 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);}
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 1680 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 1686 "hphp.y"
    { Token u; u.reset();
                                         _p->finishStatement((yyvsp[(10) - (11)]), (yyvsp[(10) - (11)])); (yyvsp[(10) - (11)]) = 1;
                                         _p->onClosure((yyval),0,u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 1692 "hphp.y"
    { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 1698 "hphp.y"
    { Token u; u.reset();
                                         _p->finishStatement((yyvsp[(11) - (12)]), (yyvsp[(11) - (12)])); (yyvsp[(11) - (12)]) = 1;
                                         _p->onClosure((yyval),&(yyvsp[(1) - (12)]),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]));
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 1707 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 1714 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 1717 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 1724 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 1727 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 1732 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval).reset(); }
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 1738 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval).reset(); }
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 1743 "hphp.y"
    { _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY);}
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 1747 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 1748 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 1753 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 1760 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);}
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 1767 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 1769 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 1773 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 1775 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 1779 "hphp.y"
    { _p->onQuery((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 1783 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);}
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 1787 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 1792 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), NULL); }
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 1794 "hphp.y"
    { _p->onQueryBody((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]), &(yyvsp[(3) - (3)])); }
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 1796 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (1)]), NULL); }
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 1798 "hphp.y"
    { _p->onQueryBody((yyval), NULL, (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 1802 "hphp.y"
    { _p->onQueryBodyClause((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 1804 "hphp.y"
    { _p->onQueryBodyClause((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 1817 "hphp.y"
    { _p->onFromClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 1821 "hphp.y"
    { _p->onLetClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 1825 "hphp.y"
    { _p->onWhereClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onJoinClause((yyval), (yyvsp[(2) - (8)]), (yyvsp[(4) - (8)]), (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)])); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 1835 "hphp.y"
    { _p->onJoinIntoClause((yyval), (yyvsp[(2) - (10)]), (yyvsp[(4) - (10)]), (yyvsp[(6) - (10)]), (yyvsp[(8) - (10)]), (yyvsp[(10) - (10)])); }
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 1839 "hphp.y"
    { _p->onOrderbyClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 1843 "hphp.y"
    { _p->onOrdering((yyval), NULL, (yyvsp[(1) - (1)])); }
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 1844 "hphp.y"
    { _p->onOrdering((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 1848 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (1)]), NULL); }
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 1849 "hphp.y"
    { _p->onOrderingExpr((yyval), (yyvsp[(1) - (2)]), &(yyvsp[(2) - (2)])); }
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 1863 "hphp.y"
    { _p->onSelectClause((yyval), (yyvsp[(2) - (2)])); }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onGroupClause((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onIntoClause((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 1876 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);}
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval).reset();}
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 1884 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 1891 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));}
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 1894 "hphp.y"
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

  case 460:

/* Line 1806 of yacc.c  */
#line 1905 "hphp.y"
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

  case 461:

/* Line 1806 of yacc.c  */
#line 1916 "hphp.y"
    { (yyval).reset(); (yyval).setText("");}
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 1917 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));}
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);}
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval).reset();}
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 1926 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);}
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 1927 "hphp.y"
    { (yyval).reset();}
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 1930 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 1934 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));}
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 1937 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       }
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 1947 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 1948 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 1952 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 1954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);}
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 1956 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);}
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 1959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 1962 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 1966 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 1967 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 1971 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 1972 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 491:

/* Line 1806 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 492:

/* Line 1806 of yacc.c  */
#line 1975 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 1976 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 1977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 1978 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 1979 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 1980 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 1981 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 1982 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 1983 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 1984 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 1985 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 1986 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 1987 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 1988 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 1989 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 1990 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 1991 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 1992 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 1993 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 1994 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 512:

/* Line 1806 of yacc.c  */
#line 1995 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 513:

/* Line 1806 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 514:

/* Line 1806 of yacc.c  */
#line 1997 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 515:

/* Line 1806 of yacc.c  */
#line 1998 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 516:

/* Line 1806 of yacc.c  */
#line 1999 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 517:

/* Line 1806 of yacc.c  */
#line 2000 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 2001 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 2002 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 2003 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 2006 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 2008 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 2009 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 2010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 2011 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 2012 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 2013 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 2014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 2015 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 2016 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 2021 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 2022 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 2023 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 2024 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 2025 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 2026 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 2027 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 2028 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 2029 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 2030 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 2035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 2036 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 2037 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 2038 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 2043 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 2047 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 2048 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 2051 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 2052 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 2053 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);}
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 2057 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);}
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 2058 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);}
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 2059 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);}
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 2063 "hphp.y"
    { (yyval).reset();}
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 2065 "hphp.y"
    { (yyval).reset();}
    break;

  case 568:

/* Line 1806 of yacc.c  */
#line 2069 "hphp.y"
    { (yyval).reset();}
    break;

  case 569:

/* Line 1806 of yacc.c  */
#line 2070 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);}
    break;

  case 570:

/* Line 1806 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 571:

/* Line 1806 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 572:

/* Line 1806 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval).reset();}
    break;

  case 573:

/* Line 1806 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 574:

/* Line 1806 of yacc.c  */
#line 2081 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 575:

/* Line 1806 of yacc.c  */
#line 2082 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 576:

/* Line 1806 of yacc.c  */
#line 2084 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));}
    break;

  case 577:

/* Line 1806 of yacc.c  */
#line 2085 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));}
    break;

  case 578:

/* Line 1806 of yacc.c  */
#line 2086 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));}
    break;

  case 579:

/* Line 1806 of yacc.c  */
#line 2087 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 580:

/* Line 1806 of yacc.c  */
#line 2088 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));}
    break;

  case 581:

/* Line 1806 of yacc.c  */
#line 2089 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));}
    break;

  case 582:

/* Line 1806 of yacc.c  */
#line 2090 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));}
    break;

  case 583:

/* Line 1806 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));}
    break;

  case 584:

/* Line 1806 of yacc.c  */
#line 2092 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));}
    break;

  case 585:

/* Line 1806 of yacc.c  */
#line 2095 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 586:

/* Line 1806 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 587:

/* Line 1806 of yacc.c  */
#line 2101 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 588:

/* Line 1806 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 589:

/* Line 1806 of yacc.c  */
#line 2103 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 590:

/* Line 1806 of yacc.c  */
#line 2104 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 591:

/* Line 1806 of yacc.c  */
#line 2106 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 592:

/* Line 1806 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 593:

/* Line 1806 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); }
    break;

  case 594:

/* Line 1806 of yacc.c  */
#line 2111 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 595:

/* Line 1806 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 596:

/* Line 1806 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 597:

/* Line 1806 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 598:

/* Line 1806 of yacc.c  */
#line 2121 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);}
    break;

  case 599:

/* Line 1806 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 600:

/* Line 1806 of yacc.c  */
#line 2127 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 601:

/* Line 1806 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 602:

/* Line 1806 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 603:

/* Line 1806 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));}
    break;

  case 604:

/* Line 1806 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));}
    break;

  case 605:

/* Line 1806 of yacc.c  */
#line 2133 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));}
    break;

  case 606:

/* Line 1806 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 607:

/* Line 1806 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval).reset();}
    break;

  case 608:

/* Line 1806 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval).reset();}
    break;

  case 609:

/* Line 1806 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval).reset();}
    break;

  case 610:

/* Line 1806 of yacc.c  */
#line 2147 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();}
    break;

  case 611:

/* Line 1806 of yacc.c  */
#line 2148 "hphp.y"
    { (yyval).reset();}
    break;

  case 612:

/* Line 1806 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 613:

/* Line 1806 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 614:

/* Line 1806 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 615:

/* Line 1806 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 616:

/* Line 1806 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 617:

/* Line 1806 of yacc.c  */
#line 2164 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));}
    break;

  case 618:

/* Line 1806 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));}
    break;

  case 619:

/* Line 1806 of yacc.c  */
#line 2169 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));}
    break;

  case 620:

/* Line 1806 of yacc.c  */
#line 2171 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));}
    break;

  case 621:

/* Line 1806 of yacc.c  */
#line 2174 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 622:

/* Line 1806 of yacc.c  */
#line 2175 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));}
    break;

  case 623:

/* Line 1806 of yacc.c  */
#line 2176 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 624:

/* Line 1806 of yacc.c  */
#line 2179 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 625:

/* Line 1806 of yacc.c  */
#line 2180 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));}
    break;

  case 626:

/* Line 1806 of yacc.c  */
#line 2181 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);}
    break;

  case 627:

/* Line 1806 of yacc.c  */
#line 2182 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);}
    break;

  case 628:

/* Line 1806 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 629:

/* Line 1806 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);}
    break;

  case 630:

/* Line 1806 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 631:

/* Line 1806 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); }
    break;

  case 632:

/* Line 1806 of yacc.c  */
#line 2194 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 633:

/* Line 1806 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval).reset();}
    break;

  case 634:

/* Line 1806 of yacc.c  */
#line 2200 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 635:

/* Line 1806 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 636:

/* Line 1806 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 637:

/* Line 1806 of yacc.c  */
#line 2205 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 638:

/* Line 1806 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 639:

/* Line 1806 of yacc.c  */
#line 2210 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 640:

/* Line 1806 of yacc.c  */
#line 2215 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 641:

/* Line 1806 of yacc.c  */
#line 2216 "hphp.y"
    { (yyval).reset(); }
    break;

  case 642:

/* Line 1806 of yacc.c  */
#line 2221 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); }
    break;

  case 643:

/* Line 1806 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); }
    break;

  case 644:

/* Line 1806 of yacc.c  */
#line 2229 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 645:

/* Line 1806 of yacc.c  */
#line 2230 "hphp.y"
    { (yyval).reset();}
    break;

  case 646:

/* Line 1806 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);}
    break;

  case 647:

/* Line 1806 of yacc.c  */
#line 2234 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);}
    break;

  case 648:

/* Line 1806 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 649:

/* Line 1806 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 650:

/* Line 1806 of yacc.c  */
#line 2246 "hphp.y"
    { only_in_hh_syntax(_p);}
    break;

  case 651:

/* Line 1806 of yacc.c  */
#line 2248 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 652:

/* Line 1806 of yacc.c  */
#line 2251 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 653:

/* Line 1806 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 654:

/* Line 1806 of yacc.c  */
#line 2255 "hphp.y"
    { (yyval).reset();}
    break;

  case 655:

/* Line 1806 of yacc.c  */
#line 2259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 656:

/* Line 1806 of yacc.c  */
#line 2261 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 657:

/* Line 1806 of yacc.c  */
#line 2265 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);}
    break;

  case 658:

/* Line 1806 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);}
    break;

  case 659:

/* Line 1806 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 660:

/* Line 1806 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 661:

/* Line 1806 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 662:

/* Line 1806 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 663:

/* Line 1806 of yacc.c  */
#line 2282 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));}
    break;

  case 664:

/* Line 1806 of yacc.c  */
#line 2284 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));}
    break;

  case 665:

/* Line 1806 of yacc.c  */
#line 2288 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 666:

/* Line 1806 of yacc.c  */
#line 2289 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 667:

/* Line 1806 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 668:

/* Line 1806 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 669:

/* Line 1806 of yacc.c  */
#line 2292 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 670:

/* Line 1806 of yacc.c  */
#line 2293 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 671:

/* Line 1806 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 672:

/* Line 1806 of yacc.c  */
#line 2298 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 673:

/* Line 1806 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 674:

/* Line 1806 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 675:

/* Line 1806 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 676:

/* Line 1806 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 677:

/* Line 1806 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 678:

/* Line 1806 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 679:

/* Line 1806 of yacc.c  */
#line 2310 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 680:

/* Line 1806 of yacc.c  */
#line 2312 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 681:

/* Line 1806 of yacc.c  */
#line 2314 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);}
    break;

  case 682:

/* Line 1806 of yacc.c  */
#line 2315 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 683:

/* Line 1806 of yacc.c  */
#line 2319 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 684:

/* Line 1806 of yacc.c  */
#line 2320 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 685:

/* Line 1806 of yacc.c  */
#line 2321 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 686:

/* Line 1806 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));}
    break;

  case 687:

/* Line 1806 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 688:

/* Line 1806 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 689:

/* Line 1806 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));}
    break;

  case 690:

/* Line 1806 of yacc.c  */
#line 2341 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));}
    break;

  case 691:

/* Line 1806 of yacc.c  */
#line 2345 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));}
    break;

  case 692:

/* Line 1806 of yacc.c  */
#line 2352 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));}
    break;

  case 693:

/* Line 1806 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));}
    break;

  case 694:

/* Line 1806 of yacc.c  */
#line 2360 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));}
    break;

  case 695:

/* Line 1806 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 696:

/* Line 1806 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 697:

/* Line 1806 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 698:

/* Line 1806 of yacc.c  */
#line 2372 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 699:

/* Line 1806 of yacc.c  */
#line 2373 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 700:

/* Line 1806 of yacc.c  */
#line 2376 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 701:

/* Line 1806 of yacc.c  */
#line 2377 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);}
    break;

  case 702:

/* Line 1806 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 703:

/* Line 1806 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval).reset();}
    break;

  case 704:

/* Line 1806 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval) = 1;}
    break;

  case 705:

/* Line 1806 of yacc.c  */
#line 2386 "hphp.y"
    { (yyval)++;}
    break;

  case 706:

/* Line 1806 of yacc.c  */
#line 2390 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 707:

/* Line 1806 of yacc.c  */
#line 2391 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 708:

/* Line 1806 of yacc.c  */
#line 2392 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 709:

/* Line 1806 of yacc.c  */
#line 2394 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 710:

/* Line 1806 of yacc.c  */
#line 2397 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 711:

/* Line 1806 of yacc.c  */
#line 2398 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 713:

/* Line 1806 of yacc.c  */
#line 2402 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 714:

/* Line 1806 of yacc.c  */
#line 2404 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));}
    break;

  case 715:

/* Line 1806 of yacc.c  */
#line 2406 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));}
    break;

  case 716:

/* Line 1806 of yacc.c  */
#line 2407 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 717:

/* Line 1806 of yacc.c  */
#line 2411 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);}
    break;

  case 718:

/* Line 1806 of yacc.c  */
#line 2412 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));}
    break;

  case 719:

/* Line 1806 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));}
    break;

  case 720:

/* Line 1806 of yacc.c  */
#line 2415 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);}
    break;

  case 721:

/* Line 1806 of yacc.c  */
#line 2416 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));}
    break;

  case 722:

/* Line 1806 of yacc.c  */
#line 2417 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));}
    break;

  case 723:

/* Line 1806 of yacc.c  */
#line 2422 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 724:

/* Line 1806 of yacc.c  */
#line 2423 "hphp.y"
    { (yyval).reset();}
    break;

  case 725:

/* Line 1806 of yacc.c  */
#line 2427 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);}
    break;

  case 726:

/* Line 1806 of yacc.c  */
#line 2428 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);}
    break;

  case 727:

/* Line 1806 of yacc.c  */
#line 2429 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);}
    break;

  case 728:

/* Line 1806 of yacc.c  */
#line 2430 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);}
    break;

  case 729:

/* Line 1806 of yacc.c  */
#line 2433 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);}
    break;

  case 730:

/* Line 1806 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);}
    break;

  case 731:

/* Line 1806 of yacc.c  */
#line 2436 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);}
    break;

  case 732:

/* Line 1806 of yacc.c  */
#line 2437 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);}
    break;

  case 733:

/* Line 1806 of yacc.c  */
#line 2442 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 734:

/* Line 1806 of yacc.c  */
#line 2443 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 735:

/* Line 1806 of yacc.c  */
#line 2447 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 736:

/* Line 1806 of yacc.c  */
#line 2448 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 737:

/* Line 1806 of yacc.c  */
#line 2449 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 738:

/* Line 1806 of yacc.c  */
#line 2450 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 739:

/* Line 1806 of yacc.c  */
#line 2455 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 740:

/* Line 1806 of yacc.c  */
#line 2456 "hphp.y"
    { _p->onEmptyCollection((yyval));}
    break;

  case 741:

/* Line 1806 of yacc.c  */
#line 2461 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));}
    break;

  case 742:

/* Line 1806 of yacc.c  */
#line 2463 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));}
    break;

  case 743:

/* Line 1806 of yacc.c  */
#line 2465 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 744:

/* Line 1806 of yacc.c  */
#line 2466 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));}
    break;

  case 745:

/* Line 1806 of yacc.c  */
#line 2470 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);}
    break;

  case 746:

/* Line 1806 of yacc.c  */
#line 2472 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);}
    break;

  case 747:

/* Line 1806 of yacc.c  */
#line 2473 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);}
    break;

  case 748:

/* Line 1806 of yacc.c  */
#line 2475 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); }
    break;

  case 749:

/* Line 1806 of yacc.c  */
#line 2480 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));}
    break;

  case 750:

/* Line 1806 of yacc.c  */
#line 2482 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));}
    break;

  case 751:

/* Line 1806 of yacc.c  */
#line 2484 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 752:

/* Line 1806 of yacc.c  */
#line 2486 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);}
    break;

  case 753:

/* Line 1806 of yacc.c  */
#line 2488 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));}
    break;

  case 754:

/* Line 1806 of yacc.c  */
#line 2489 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);}
    break;

  case 755:

/* Line 1806 of yacc.c  */
#line 2492 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;}
    break;

  case 756:

/* Line 1806 of yacc.c  */
#line 2493 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;}
    break;

  case 757:

/* Line 1806 of yacc.c  */
#line 2494 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;}
    break;

  case 758:

/* Line 1806 of yacc.c  */
#line 2498 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);}
    break;

  case 759:

/* Line 1806 of yacc.c  */
#line 2499 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);}
    break;

  case 760:

/* Line 1806 of yacc.c  */
#line 2500 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 761:

/* Line 1806 of yacc.c  */
#line 2501 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),'!',1);}
    break;

  case 762:

/* Line 1806 of yacc.c  */
#line 2502 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);}
    break;

  case 763:

/* Line 1806 of yacc.c  */
#line 2503 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);}
    break;

  case 764:

/* Line 1806 of yacc.c  */
#line 2504 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);}
    break;

  case 765:

/* Line 1806 of yacc.c  */
#line 2505 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);}
    break;

  case 766:

/* Line 1806 of yacc.c  */
#line 2506 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);}
    break;

  case 767:

/* Line 1806 of yacc.c  */
#line 2510 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));}
    break;

  case 768:

/* Line 1806 of yacc.c  */
#line 2511 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));}
    break;

  case 769:

/* Line 1806 of yacc.c  */
#line 2516 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);}
    break;

  case 772:

/* Line 1806 of yacc.c  */
#line 2530 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); }
    break;

  case 773:

/* Line 1806 of yacc.c  */
#line 2533 "hphp.y"
    { _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); }
    break;

  case 774:

/* Line 1806 of yacc.c  */
#line 2538 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 775:

/* Line 1806 of yacc.c  */
#line 2539 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 776:

/* Line 1806 of yacc.c  */
#line 2545 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 777:

/* Line 1806 of yacc.c  */
#line 2549 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]); }
    break;

  case 778:

/* Line 1806 of yacc.c  */
#line 2555 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 779:

/* Line 1806 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval).reset(); }
    break;

  case 780:

/* Line 1806 of yacc.c  */
#line 2560 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 781:

/* Line 1806 of yacc.c  */
#line 2563 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 782:

/* Line 1806 of yacc.c  */
#line 2568 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); }
    break;

  case 783:

/* Line 1806 of yacc.c  */
#line 2569 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 784:

/* Line 1806 of yacc.c  */
#line 2570 "hphp.y"
    { (yyval).reset(); }
    break;

  case 785:

/* Line 1806 of yacc.c  */
#line 2571 "hphp.y"
    { (yyval).reset(); }
    break;

  case 786:

/* Line 1806 of yacc.c  */
#line 2575 "hphp.y"
    { (yyval).reset(); }
    break;

  case 787:

/* Line 1806 of yacc.c  */
#line 2576 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 788:

/* Line 1806 of yacc.c  */
#line 2581 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); }
    break;

  case 789:

/* Line 1806 of yacc.c  */
#line 2582 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); }
    break;

  case 790:

/* Line 1806 of yacc.c  */
#line 2584 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); }
    break;

  case 791:

/* Line 1806 of yacc.c  */
#line 2585 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); }
    break;

  case 792:

/* Line 1806 of yacc.c  */
#line 2591 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); }
    break;

  case 795:

/* Line 1806 of yacc.c  */
#line 2602 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); }
    break;

  case 796:

/* Line 1806 of yacc.c  */
#line 2604 "hphp.y"
    {}
    break;

  case 797:

/* Line 1806 of yacc.c  */
#line 2608 "hphp.y"
    { (yyval).setText("array"); }
    break;

  case 798:

/* Line 1806 of yacc.c  */
#line 2615 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 799:

/* Line 1806 of yacc.c  */
#line 2618 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); }
    break;

  case 800:

/* Line 1806 of yacc.c  */
#line 2621 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 801:

/* Line 1806 of yacc.c  */
#line 2622 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); }
    break;

  case 802:

/* Line 1806 of yacc.c  */
#line 2625 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 803:

/* Line 1806 of yacc.c  */
#line 2627 "hphp.y"
    { (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); }
    break;

  case 804:

/* Line 1806 of yacc.c  */
#line 2630 "hphp.y"
    { _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); }
    break;

  case 805:

/* Line 1806 of yacc.c  */
#line 2633 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); }
    break;

  case 806:

/* Line 1806 of yacc.c  */
#line 2639 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); }
    break;

  case 807:

/* Line 1806 of yacc.c  */
#line 2643 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); }
    break;

  case 808:

/* Line 1806 of yacc.c  */
#line 2651 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 809:

/* Line 1806 of yacc.c  */
#line 2652 "hphp.y"
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
#line 2655 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

