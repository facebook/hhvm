
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
#define yyparse         Compilerparse
#define yylex           Compilerlex
#define yyerror         Compilererror
#define yylval          Compilerlval
#define yychar          Compilerchar
#define yydebug         Compilerdebug
#define yynerrs         Compilernerrs
#define yylloc          Compilerlloc

/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "hphp.y"

#ifdef TEST_PARSER
#include "hphp/util/parser/test/parser.h"
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
#define yyerror(loc,p,msg) p->fatal(loc,msg)

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
// continuation transformations

void prepare_generator(Parser *_p, Token &stmt, Token &params) {
  // 1. add prologue and epilogue to original body and store it back to "stmt"
  {
    // hphp_unpack_continuation()
    Token empty;
    Token cname;   cname.setText("hphp_unpack_continuation");
    Token unpack;  _p->onCall(unpack, false, cname, empty, NULL, true);
    Token sunpack; _p->onExpStatement(sunpack, unpack);

    Token stmts0;  _p->onStatementListStart(stmts0);
    Token stmts1;  _p->addStatement(stmts1, stmts0, sunpack);
    Token stmts2;  _p->addStatement(stmts2, stmts1, stmt);

    stmt.reset();
    _p->finishStatement(stmt, stmts2); stmt = 1;
  }

  // 2. prepare a single continuation parameter list and store it in "params"
  {
    Token type;    type.setText("Continuation");
    Token var;     var.setText(CONTINUATION_OBJECT_NAME);
    params.reset();
    type.reset();
    _p->onParam(params, NULL, type, var, false, NULL, NULL);
  }
}

// create a generator function with original name and parameters
void create_generator(Parser *_p, Token &out, Token &params,
                      Token &name, const std::string &closureName,
                      const char *clsname, Token *modifiers,
                      Token &origGenFunc, bool isHhvm, Token *attr) {
  _p->pushFuncLocation();
  if (clsname) {
    _p->onMethodStart(name, *modifiers, false);
  } else {
    _p->onFunctionStart(name, false);
  }

  Token scont;
  {
    Token cname;
    if (isHhvm) {
      Token cn;    cn.setText(clsname ? "__CLASS__" : "");
                   _p->onScalar(
                     cname,
                     clsname ? T_CLASS_C : T_CONSTANT_ENCAPSED_STRING,
                     cn);
    } else {
      Token cn;    cn.setText(clsname ? clsname : "");
                   _p->onScalar(cname, T_CONSTANT_ENCAPSED_STRING, cn);
    }

    Token fn;      fn.setText(closureName);
    Token fname;   _p->onScalar(fname, T_CONSTANT_ENCAPSED_STRING, fn);

    Token ofn;     ofn.setText(clsname ? "__METHOD__" : "__FUNCTION__");
    Token oname;   _p->onScalar(oname, clsname ? T_METHOD_C : T_FUNC_C, ofn);

    Token param1;  _p->onCallParam(param1, NULL, cname, false);
                   _p->onCallParam(param1, &param1, fname, false);
                   _p->onCallParam(param1, &param1, oname, false);

    Token stmts0;  _p->onStatementListStart(stmts0);

    Token cname0;  cname0.setText("hphp_create_continuation");
    Token call;    _p->onCall(call, false, cname0, param1, NULL, true);
    Token ret;     _p->onReturn(ret, &call);

    Token stmts1;  _p->addStatement(stmts1, stmts0, ret);
    _p->finishStatement(scont, stmts1); scont = 1;
  }

  Token ret, ref;
  ret.setText("Continuation");
  ret.setCheck();
  if (clsname) {
    Token closure;
    _p->onMethod(closure, *modifiers, ret, ref, name, params, scont, attr);
    origGenFunc = closure;

    Token stmts0;  _p->onStatementListStart(stmts0);
    Token stmts1;  _p->addStatement(stmts1, stmts0, closure);
    Token stmts2;  _p->addStatement(stmts2, stmts1, out);
    _p->finishStatement(out, stmts2); out = 1;
  } else {
    out.reset();
    _p->onFunction(out, modifiers, ret, ref, name, params, scont, attr);
    origGenFunc = out;
  }
}

///////////////////////////////////////////////////////////////////////////////

static void user_attribute_check(Parser *_p) {
  if (!_p->scanner().hipHopSyntaxEnabled()) {
    HPHP_PARSER_ERROR("User attributes are not enabled", _p);
  }
}

static void finally_statement(Parser *_p) {
  if (!_p->enableFinallyStatement()) {
    HPHP_PARSER_ERROR("Finally statement is not enabled", _p);
  }
}

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
  if (!_p->enableXHP()) {
    HPHP_PARSER_ERROR("XHP: not enabled", _p);
  }

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
   * T_STRING in the parser, and the parser uses always uses type code 5 for
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
  if (!_p->enableXHP()) {
    HPHP_PARSER_ERROR("XHP: not enabled", _p);
  }

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
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, 0, false);
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
  if (!_p->enableXHP()) {
    HPHP_PARSER_ERROR("XHP: not enabled", _p);
  }

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
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, 0, false);
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
  if (!_p->enableXHP()) {
    HPHP_PARSER_ERROR("XHP: not enabled", _p);
  }

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
    _p->onMethod(out, modifiers, ret, ref, fname, params, stmt, 0, false);
  }
}

static void only_in_hh_syntax(Parser *_p) {
  if (!_p->scanner().hipHopSyntaxEnabled()) {
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


/* Line 189 of yacc.c  */
#line 743 "new_hphp.tab.cpp"

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
     T_PAAMAYIM_NEKUDOTAYIM = 376,
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
     T_HACK_ERROR = 394,
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
     T_COMPILER_HALT_OFFSET = 407
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
#line 950 "new_hphp.tab.cpp"

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
#define YYLAST   10170

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  182
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  209
/* YYNRULES -- Number of rules.  */
#define YYNRULES  717
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1341

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   407

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   180,     2,   177,    47,    31,   181,
     172,   173,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   174,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   179,    30,     2,   178,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   175,    29,   176,    50,     2,     2,     2,
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
     164,   165,   166,   167,   168,   169,   170,   171
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    11,    13,    15,    17,
      19,    21,    26,    30,    31,    38,    39,    45,    49,    52,
      54,    56,    58,    60,    62,    64,    68,    70,    72,    75,
      79,    84,    86,    90,    92,    96,    99,   101,   104,   107,
     113,   118,   121,   122,   124,   126,   128,   130,   134,   140,
     149,   150,   155,   156,   163,   164,   175,   176,   181,   184,
     188,   191,   195,   198,   202,   206,   210,   214,   218,   224,
     226,   228,   229,   239,   245,   260,   266,   270,   274,   277,
     280,   283,   286,   289,   299,   300,   301,   307,   309,   310,
     312,   313,   315,   316,   328,   329,   342,   343,   352,   353,
     363,   364,   372,   373,   382,   383,   391,   392,   401,   403,
     405,   407,   409,   411,   414,   417,   420,   421,   424,   425,
     428,   429,   431,   435,   437,   441,   444,   445,   447,   450,
     452,   457,   459,   464,   466,   471,   473,   478,   482,   488,
     492,   497,   502,   508,   514,   519,   520,   522,   524,   529,
     530,   536,   537,   540,   541,   545,   546,   550,   553,   555,
     556,   560,   565,   572,   578,   584,   591,   600,   608,   611,
     612,   614,   617,   621,   626,   630,   632,   634,   637,   642,
     646,   652,   654,   658,   661,   662,   663,   668,   669,   675,
     678,   679,   690,   691,   703,   707,   711,   715,   719,   725,
     728,   731,   732,   739,   745,   750,   754,   756,   758,   762,
     767,   769,   771,   773,   775,   780,   782,   786,   789,   790,
     793,   794,   796,   800,   802,   804,   806,   808,   812,   817,
     822,   827,   829,   831,   834,   837,   840,   844,   848,   850,
     852,   854,   856,   860,   862,   864,   866,   867,   869,   872,
     874,   876,   878,   880,   882,   884,   888,   894,   896,   900,
     906,   911,   915,   919,   922,   924,   928,   932,   934,   936,
     937,   940,   944,   951,   953,   955,   957,   964,   968,   973,
     980,   984,   988,   992,   996,  1000,  1004,  1008,  1012,  1016,
    1020,  1024,  1027,  1030,  1033,  1036,  1040,  1044,  1048,  1052,
    1056,  1060,  1064,  1068,  1072,  1076,  1080,  1084,  1088,  1092,
    1096,  1100,  1103,  1106,  1109,  1112,  1116,  1120,  1124,  1128,
    1132,  1136,  1140,  1144,  1148,  1152,  1158,  1163,  1165,  1168,
    1171,  1174,  1177,  1180,  1183,  1186,  1189,  1192,  1194,  1196,
    1198,  1202,  1205,  1206,  1218,  1219,  1232,  1234,  1236,  1238,
    1244,  1248,  1254,  1258,  1261,  1262,  1265,  1266,  1271,  1276,
    1280,  1285,  1290,  1295,  1300,  1302,  1304,  1308,  1314,  1315,
    1319,  1324,  1326,  1329,  1334,  1337,  1344,  1345,  1347,  1352,
    1353,  1356,  1357,  1359,  1361,  1365,  1367,  1371,  1373,  1375,
    1379,  1383,  1385,  1387,  1389,  1391,  1393,  1395,  1397,  1399,
    1401,  1403,  1405,  1407,  1409,  1411,  1413,  1415,  1417,  1419,
    1421,  1423,  1425,  1427,  1429,  1431,  1433,  1435,  1437,  1439,
    1441,  1443,  1445,  1447,  1449,  1451,  1453,  1455,  1457,  1459,
    1461,  1463,  1465,  1467,  1469,  1471,  1473,  1475,  1477,  1479,
    1481,  1483,  1485,  1487,  1489,  1491,  1493,  1495,  1497,  1499,
    1501,  1503,  1505,  1507,  1509,  1511,  1513,  1515,  1517,  1519,
    1521,  1523,  1525,  1527,  1529,  1531,  1533,  1535,  1537,  1539,
    1544,  1546,  1548,  1550,  1552,  1554,  1556,  1558,  1560,  1563,
    1565,  1566,  1567,  1569,  1571,  1575,  1576,  1578,  1580,  1582,
    1584,  1586,  1588,  1590,  1592,  1594,  1596,  1598,  1600,  1604,
    1607,  1609,  1611,  1614,  1617,  1622,  1626,  1631,  1633,  1635,
    1639,  1643,  1645,  1647,  1649,  1651,  1655,  1659,  1663,  1666,
    1667,  1669,  1670,  1672,  1673,  1679,  1683,  1687,  1689,  1691,
    1693,  1695,  1699,  1702,  1704,  1706,  1708,  1710,  1712,  1715,
    1718,  1723,  1727,  1732,  1735,  1736,  1742,  1746,  1750,  1752,
    1756,  1758,  1761,  1762,  1768,  1772,  1775,  1776,  1780,  1781,
    1786,  1789,  1790,  1794,  1798,  1800,  1801,  1803,  1806,  1809,
    1814,  1818,  1822,  1825,  1830,  1833,  1838,  1840,  1842,  1844,
    1846,  1848,  1851,  1856,  1860,  1865,  1869,  1871,  1873,  1875,
    1877,  1880,  1885,  1890,  1894,  1896,  1898,  1902,  1910,  1917,
    1926,  1936,  1945,  1956,  1964,  1971,  1973,  1976,  1981,  1986,
    1988,  1990,  1995,  1997,  1998,  2000,  2003,  2005,  2007,  2010,
    2015,  2019,  2023,  2024,  2026,  2029,  2034,  2038,  2041,  2045,
    2052,  2053,  2055,  2060,  2063,  2064,  2070,  2074,  2078,  2080,
    2087,  2092,  2097,  2100,  2103,  2104,  2110,  2114,  2118,  2120,
    2123,  2124,  2130,  2134,  2138,  2140,  2143,  2146,  2148,  2151,
    2153,  2158,  2162,  2166,  2173,  2177,  2179,  2181,  2183,  2188,
    2193,  2196,  2199,  2204,  2207,  2210,  2212,  2216,  2220,  2221,
    2224,  2230,  2237,  2239,  2242,  2244,  2249,  2253,  2254,  2256,
    2260,  2264,  2266,  2268,  2269,  2270,  2273,  2277,  2279,  2285,
    2289,  2293,  2297,  2299,  2302,  2303,  2308,  2311,  2314,  2316,
    2318,  2320,  2325,  2332,  2334,  2343,  2349,  2351
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     183,     0,    -1,    -1,   184,   185,    -1,   185,   186,    -1,
      -1,   200,    -1,   212,    -1,   215,    -1,   220,    -1,   377,
      -1,   116,   172,   173,   174,    -1,   141,   192,   174,    -1,
      -1,   141,   192,   175,   187,   185,   176,    -1,    -1,   141,
     175,   188,   185,   176,    -1,   104,   190,   174,    -1,   197,
     174,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   190,     8,   191,    -1,   191,    -1,
     192,    -1,   144,   192,    -1,   192,    90,   189,    -1,   144,
     192,    90,   189,    -1,   189,    -1,   192,   144,   189,    -1,
     192,    -1,   141,   144,   192,    -1,   144,   192,    -1,   193,
      -1,   193,   380,    -1,   193,   380,    -1,   197,     8,   378,
      13,   324,    -1,    99,   378,    13,   324,    -1,   198,   199,
      -1,    -1,   200,    -1,   212,    -1,   215,    -1,   220,    -1,
     175,   198,   176,    -1,    65,   283,   200,   242,   244,    -1,
      65,   283,    26,   198,   243,   245,    68,   174,    -1,    -1,
      82,   283,   201,   236,    -1,    -1,    81,   202,   200,    82,
     283,   174,    -1,    -1,    84,   172,   285,   174,   285,   174,
     285,   173,   203,   234,    -1,    -1,    91,   283,   204,   239,
      -1,    95,   174,    -1,    95,    69,   174,    -1,    97,   174,
      -1,    97,    69,   174,    -1,   100,   174,    -1,   100,   289,
     174,    -1,   145,    95,   174,    -1,   105,   250,   174,    -1,
     111,   252,   174,    -1,    80,   284,   174,    -1,   113,   172,
     374,   173,   174,    -1,   174,    -1,    75,    -1,    -1,    86,
     172,   289,    90,   233,   232,   173,   205,   235,    -1,    88,
     172,   238,   173,   237,    -1,   101,   175,   198,   176,   102,
     172,   317,    73,   173,   175,   198,   176,   206,   209,    -1,
     101,   175,   198,   176,   207,    -1,   103,   289,   174,    -1,
      96,   189,   174,    -1,   289,   174,    -1,   286,   174,    -1,
     287,   174,    -1,   288,   174,    -1,   189,    26,    -1,   206,
     102,   172,   317,    73,   173,   175,   198,   176,    -1,    -1,
      -1,   208,   159,   175,   198,   176,    -1,   207,    -1,    -1,
      31,    -1,    -1,    98,    -1,    -1,   211,   210,   379,   213,
     172,   246,   173,   383,   175,   198,   176,    -1,    -1,   344,
     211,   210,   379,   214,   172,   246,   173,   383,   175,   198,
     176,    -1,    -1,   226,   223,   216,   227,   228,   175,   253,
     176,    -1,    -1,   344,   226,   223,   217,   227,   228,   175,
     253,   176,    -1,    -1,   118,   224,   218,   229,   175,   253,
     176,    -1,    -1,   344,   118,   224,   219,   229,   175,   253,
     176,    -1,    -1,   154,   225,   221,   228,   175,   253,   176,
      -1,    -1,   344,   154,   225,   222,   228,   175,   253,   176,
      -1,   379,    -1,   146,    -1,   379,    -1,   379,    -1,   117,
      -1,   110,   117,    -1,   109,   117,    -1,   119,   317,    -1,
      -1,   120,   230,    -1,    -1,   119,   230,    -1,    -1,   317,
      -1,   230,     8,   317,    -1,   317,    -1,   231,     8,   317,
      -1,   122,   233,    -1,    -1,   351,    -1,    31,   351,    -1,
     200,    -1,    26,   198,    85,   174,    -1,   200,    -1,    26,
     198,    87,   174,    -1,   200,    -1,    26,   198,    83,   174,
      -1,   200,    -1,    26,   198,    89,   174,    -1,   189,    13,
     324,    -1,   238,     8,   189,    13,   324,    -1,   175,   240,
     176,    -1,   175,   174,   240,   176,    -1,    26,   240,    92,
     174,    -1,    26,   174,   240,    92,   174,    -1,   240,    93,
     289,   241,   198,    -1,   240,    94,   241,   198,    -1,    -1,
      26,    -1,   174,    -1,   242,    66,   283,   200,    -1,    -1,
     243,    66,   283,    26,   198,    -1,    -1,    67,   200,    -1,
      -1,    67,    26,   198,    -1,    -1,   247,     8,   157,    -1,
     247,   329,    -1,   157,    -1,    -1,   345,   390,    73,    -1,
     345,   390,    31,    73,    -1,   345,   390,    31,    73,    13,
     324,    -1,   345,   390,    73,    13,   324,    -1,   247,     8,
     345,   390,    73,    -1,   247,     8,   345,   390,    31,    73,
      -1,   247,     8,   345,   390,    31,    73,    13,   324,    -1,
     247,     8,   345,   390,    73,    13,   324,    -1,   249,   329,
      -1,    -1,   289,    -1,    31,   351,    -1,   249,     8,   289,
      -1,   249,     8,    31,   351,    -1,   250,     8,   251,    -1,
     251,    -1,    73,    -1,   177,   351,    -1,   177,   175,   289,
     176,    -1,   252,     8,    73,    -1,   252,     8,    73,    13,
     324,    -1,    73,    -1,    73,    13,   324,    -1,   253,   254,
      -1,    -1,    -1,   276,   255,   280,   174,    -1,    -1,   278,
     389,   256,   280,   174,    -1,   281,   174,    -1,    -1,   277,
     211,   210,   379,   172,   257,   246,   173,   383,   275,    -1,
      -1,   344,   277,   211,   210,   379,   172,   258,   246,   173,
     383,   275,    -1,   148,   263,   174,    -1,   149,   269,   174,
      -1,   151,   271,   174,    -1,   104,   231,   174,    -1,   104,
     231,   175,   259,   176,    -1,   259,   260,    -1,   259,   261,
      -1,    -1,   196,   140,   189,   155,   231,   174,    -1,   262,
      90,   277,   189,   174,    -1,   262,    90,   278,   174,    -1,
     196,   140,   189,    -1,   189,    -1,   264,    -1,   263,     8,
     264,    -1,   265,   314,   267,   268,    -1,   146,    -1,   124,
      -1,   317,    -1,   112,    -1,   152,   175,   266,   176,    -1,
     323,    -1,   266,     8,   323,    -1,    13,   324,    -1,    -1,
      51,   153,    -1,    -1,   270,    -1,   269,     8,   270,    -1,
     150,    -1,   272,    -1,   189,    -1,   115,    -1,   172,   273,
     173,    -1,   172,   273,   173,    45,    -1,   172,   273,   173,
      25,    -1,   172,   273,   173,    42,    -1,   272,    -1,   274,
      -1,   274,    45,    -1,   274,    25,    -1,   274,    42,    -1,
     273,     8,   273,    -1,   273,    29,   273,    -1,   189,    -1,
     146,    -1,   150,    -1,   174,    -1,   175,   198,   176,    -1,
     278,    -1,   112,    -1,   278,    -1,    -1,   279,    -1,   278,
     279,    -1,   106,    -1,   107,    -1,   108,    -1,   111,    -1,
     110,    -1,   109,    -1,   280,     8,    73,    -1,   280,     8,
      73,    13,   324,    -1,    73,    -1,    73,    13,   324,    -1,
     281,     8,   378,    13,   324,    -1,    99,   378,    13,   324,
      -1,   172,   282,   173,    -1,    63,   319,   322,    -1,    62,
     289,    -1,   306,    -1,   172,   289,   173,    -1,   284,     8,
     289,    -1,   289,    -1,   284,    -1,    -1,   145,   289,    -1,
     351,    13,   286,    -1,   123,   172,   363,   173,    13,   286,
      -1,   290,    -1,   351,    -1,   282,    -1,   123,   172,   363,
     173,    13,   289,    -1,   351,    13,   289,    -1,   351,    13,
      31,   351,    -1,   351,    13,    31,    63,   319,   322,    -1,
     351,    24,   289,    -1,   351,    23,   289,    -1,   351,    22,
     289,    -1,   351,    21,   289,    -1,   351,    20,   289,    -1,
     351,    19,   289,    -1,   351,    18,   289,    -1,   351,    17,
     289,    -1,   351,    16,   289,    -1,   351,    15,   289,    -1,
     351,    14,   289,    -1,   351,    60,    -1,    60,   351,    -1,
     351,    59,    -1,    59,   351,    -1,   289,    27,   289,    -1,
     289,    28,   289,    -1,   289,     9,   289,    -1,   289,    11,
     289,    -1,   289,    10,   289,    -1,   289,    29,   289,    -1,
     289,    31,   289,    -1,   289,    30,   289,    -1,   289,    44,
     289,    -1,   289,    42,   289,    -1,   289,    43,   289,    -1,
     289,    45,   289,    -1,   289,    46,   289,    -1,   289,    47,
     289,    -1,   289,    41,   289,    -1,   289,    40,   289,    -1,
      42,   289,    -1,    43,   289,    -1,    48,   289,    -1,    50,
     289,    -1,   289,    33,   289,    -1,   289,    32,   289,    -1,
     289,    35,   289,    -1,   289,    34,   289,    -1,   289,    36,
     289,    -1,   289,    39,   289,    -1,   289,    37,   289,    -1,
     289,    38,   289,    -1,   289,    49,   319,    -1,   172,   290,
     173,    -1,   289,    25,   289,    26,   289,    -1,   289,    25,
      26,   289,    -1,   373,    -1,    58,   289,    -1,    57,   289,
      -1,    56,   289,    -1,    55,   289,    -1,    54,   289,    -1,
      53,   289,    -1,    52,   289,    -1,    64,   320,    -1,    51,
     289,    -1,   326,    -1,   299,    -1,   298,    -1,   178,   321,
     178,    -1,    12,   289,    -1,    -1,   211,   210,   172,   291,
     246,   173,   383,   304,   175,   198,   176,    -1,    -1,   111,
     211,   210,   172,   292,   246,   173,   383,   304,   175,   198,
     176,    -1,   302,    -1,   300,    -1,    79,    -1,   294,     8,
     293,   122,   289,    -1,   293,   122,   289,    -1,   295,     8,
     293,   122,   324,    -1,   293,   122,   324,    -1,   294,   328,
      -1,    -1,   295,   328,    -1,    -1,   166,   172,   296,   173,
      -1,   124,   172,   364,   173,    -1,    61,   364,   179,    -1,
     317,   175,   366,   176,    -1,   317,   175,   368,   176,    -1,
     302,    61,   359,   179,    -1,   303,    61,   359,   179,    -1,
     299,    -1,   375,    -1,   172,   290,   173,    -1,   104,   172,
     305,   329,   173,    -1,    -1,   305,     8,    73,    -1,   305,
       8,    31,    73,    -1,    73,    -1,    31,    73,    -1,   160,
     146,   307,   161,    -1,   309,    46,    -1,   309,   161,   310,
     160,    46,   308,    -1,    -1,   146,    -1,   309,   311,    13,
     312,    -1,    -1,   310,   313,    -1,    -1,   146,    -1,   147,
      -1,   175,   289,   176,    -1,   147,    -1,   175,   289,   176,
      -1,   306,    -1,   315,    -1,   314,    26,   315,    -1,   314,
      43,   315,    -1,   189,    -1,    64,    -1,    98,    -1,    99,
      -1,   100,    -1,   145,    -1,   101,    -1,   102,    -1,   159,
      -1,   103,    -1,    65,    -1,    66,    -1,    68,    -1,    67,
      -1,    82,    -1,    83,    -1,    81,    -1,    84,    -1,    85,
      -1,    86,    -1,    87,    -1,    88,    -1,    89,    -1,    49,
      -1,    90,    -1,    91,    -1,    92,    -1,    93,    -1,    94,
      -1,    95,    -1,    97,    -1,    96,    -1,    80,    -1,    12,
      -1,   117,    -1,   118,    -1,   119,    -1,   120,    -1,    63,
      -1,    62,    -1,   112,    -1,     5,    -1,     7,    -1,     6,
      -1,     4,    -1,     3,    -1,   141,    -1,   104,    -1,   105,
      -1,   114,    -1,   115,    -1,   116,    -1,   111,    -1,   110,
      -1,   109,    -1,   108,    -1,   107,    -1,   106,    -1,   113,
      -1,   123,    -1,   124,    -1,     9,    -1,    11,    -1,    10,
      -1,   125,    -1,   127,    -1,   126,    -1,   128,    -1,   129,
      -1,   143,    -1,   142,    -1,   171,    -1,   154,    -1,   156,
      -1,   155,    -1,   167,    -1,   169,    -1,   166,    -1,   195,
     172,   248,   173,    -1,   196,    -1,   146,    -1,   317,    -1,
     111,    -1,   357,    -1,   317,    -1,   111,    -1,   361,    -1,
     172,   173,    -1,   283,    -1,    -1,    -1,    78,    -1,   370,
      -1,   172,   248,   173,    -1,    -1,    69,    -1,    70,    -1,
      79,    -1,   128,    -1,   129,    -1,   143,    -1,   125,    -1,
     156,    -1,   126,    -1,   127,    -1,   142,    -1,   171,    -1,
     136,    78,   137,    -1,   136,   137,    -1,   323,    -1,   194,
      -1,    42,   324,    -1,    43,   324,    -1,   124,   172,   327,
     173,    -1,    61,   327,   179,    -1,   166,   172,   297,   173,
      -1,   325,    -1,   301,    -1,   196,   140,   189,    -1,   146,
     140,   189,    -1,   194,    -1,    72,    -1,   375,    -1,   323,
      -1,   180,   370,   180,    -1,   181,   370,   181,    -1,   136,
     370,   137,    -1,   330,   328,    -1,    -1,     8,    -1,    -1,
       8,    -1,    -1,   330,     8,   324,   122,   324,    -1,   330,
       8,   324,    -1,   324,   122,   324,    -1,   324,    -1,    69,
      -1,    70,    -1,    79,    -1,   136,    78,   137,    -1,   136,
     137,    -1,    69,    -1,    70,    -1,   189,    -1,   331,    -1,
     189,    -1,    42,   332,    -1,    43,   332,    -1,   124,   172,
     334,   173,    -1,    61,   334,   179,    -1,   166,   172,   337,
     173,    -1,   335,   328,    -1,    -1,   335,     8,   333,   122,
     333,    -1,   335,     8,   333,    -1,   333,   122,   333,    -1,
     333,    -1,   336,     8,   333,    -1,   333,    -1,   338,   328,
      -1,    -1,   338,     8,   293,   122,   333,    -1,   293,   122,
     333,    -1,   336,   328,    -1,    -1,   172,   339,   173,    -1,
      -1,   341,     8,   189,   340,    -1,   189,   340,    -1,    -1,
     343,   341,   328,    -1,    41,   342,    40,    -1,   344,    -1,
      -1,   347,    -1,   121,   356,    -1,   121,   189,    -1,   121,
     175,   289,   176,    -1,    61,   359,   179,    -1,   175,   289,
     176,    -1,   352,   348,    -1,   172,   282,   173,   348,    -1,
     362,   348,    -1,   172,   282,   173,   348,    -1,   356,    -1,
     316,    -1,   354,    -1,   355,    -1,   349,    -1,   351,   346,
      -1,   172,   282,   173,   346,    -1,   318,   140,   356,    -1,
     353,   172,   248,   173,    -1,   172,   351,   173,    -1,   316,
      -1,   354,    -1,   355,    -1,   349,    -1,   351,   347,    -1,
     172,   282,   173,   347,    -1,   353,   172,   248,   173,    -1,
     172,   351,   173,    -1,   356,    -1,   349,    -1,   172,   351,
     173,    -1,   351,   121,   189,   380,   172,   248,   173,    -1,
     351,   121,   356,   172,   248,   173,    -1,   351,   121,   175,
     289,   176,   172,   248,   173,    -1,   172,   282,   173,   121,
     189,   380,   172,   248,   173,    -1,   172,   282,   173,   121,
     356,   172,   248,   173,    -1,   172,   282,   173,   121,   175,
     289,   176,   172,   248,   173,    -1,   318,   140,   189,   380,
     172,   248,   173,    -1,   318,   140,   356,   172,   248,   173,
      -1,   357,    -1,   360,   357,    -1,   357,    61,   359,   179,
      -1,   357,   175,   289,   176,    -1,   358,    -1,    73,    -1,
     177,   175,   289,   176,    -1,   289,    -1,    -1,   177,    -1,
     360,   177,    -1,   356,    -1,   350,    -1,   361,   346,    -1,
     172,   282,   173,   346,    -1,   318,   140,   356,    -1,   172,
     351,   173,    -1,    -1,   350,    -1,   361,   347,    -1,   172,
     282,   173,   347,    -1,   172,   351,   173,    -1,   363,     8,
      -1,   363,     8,   351,    -1,   363,     8,   123,   172,   363,
     173,    -1,    -1,   351,    -1,   123,   172,   363,   173,    -1,
     365,   328,    -1,    -1,   365,     8,   289,   122,   289,    -1,
     365,     8,   289,    -1,   289,   122,   289,    -1,   289,    -1,
     365,     8,   289,   122,    31,   351,    -1,   365,     8,    31,
     351,    -1,   289,   122,    31,   351,    -1,    31,   351,    -1,
     367,   328,    -1,    -1,   367,     8,   289,   122,   289,    -1,
     367,     8,   289,    -1,   289,   122,   289,    -1,   289,    -1,
     369,   328,    -1,    -1,   369,     8,   324,   122,   324,    -1,
     369,     8,   324,    -1,   324,   122,   324,    -1,   324,    -1,
     370,   371,    -1,   370,    78,    -1,   371,    -1,    78,   371,
      -1,    73,    -1,    73,    61,   372,   179,    -1,    73,   121,
     189,    -1,   138,   289,   176,    -1,   138,    72,    61,   289,
     179,   176,    -1,   139,   351,   176,    -1,   189,    -1,    74,
      -1,    73,    -1,   114,   172,   374,   173,    -1,   115,   172,
     351,   173,    -1,     7,   289,    -1,     6,   289,    -1,     5,
     172,   289,   173,    -1,     4,   289,    -1,     3,   289,    -1,
     351,    -1,   374,     8,   351,    -1,   318,   140,   189,    -1,
      -1,    90,   389,    -1,   167,   379,    13,   389,   174,    -1,
     169,   379,   376,    13,   389,   174,    -1,   189,    -1,   389,
     189,    -1,   189,    -1,   189,   162,   384,   163,    -1,   162,
     381,   163,    -1,    -1,   389,    -1,   381,     8,   389,    -1,
     381,     8,   157,    -1,   381,    -1,   157,    -1,    -1,    -1,
      26,   389,    -1,   384,     8,   189,    -1,   189,    -1,   384,
       8,   189,    90,   389,    -1,   189,    90,   389,    -1,    79,
     122,   389,    -1,   386,     8,   385,    -1,   385,    -1,   386,
     328,    -1,    -1,   166,   172,   387,   173,    -1,    25,   389,
      -1,    51,   389,    -1,   196,    -1,   124,    -1,   388,    -1,
     124,   162,   389,   163,    -1,   124,   162,   389,     8,   389,
     163,    -1,   146,    -1,   172,    98,   172,   382,   173,    26,
     389,   173,    -1,   172,   381,     8,   389,   173,    -1,   389,
      -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   804,   804,   804,   809,   811,   814,   815,   816,   817,
     818,   819,   822,   824,   824,   826,   826,   828,   829,   834,
     835,   836,   837,   838,   839,   843,   845,   848,   849,   850,
     851,   856,   857,   861,   862,   864,   867,   873,   880,   887,
     891,   897,   899,   902,   903,   904,   905,   908,   909,   913,
     918,   918,   922,   922,   927,   926,   930,   930,   933,   934,
     935,   936,   937,   938,   939,   940,   941,   942,   943,   944,
     945,   948,   946,   951,   953,   961,   964,   965,   969,   970,
     971,   972,   973,   980,   986,   990,   990,   996,   997,  1001,
    1002,  1006,  1011,  1010,  1020,  1019,  1032,  1031,  1050,  1048,
    1067,  1066,  1075,  1073,  1085,  1084,  1096,  1094,  1107,  1108,
    1112,  1115,  1118,  1119,  1120,  1123,  1125,  1128,  1129,  1132,
    1133,  1136,  1137,  1141,  1142,  1147,  1148,  1151,  1152,  1156,
    1157,  1161,  1162,  1166,  1167,  1171,  1172,  1177,  1178,  1183,
    1184,  1185,  1186,  1189,  1192,  1194,  1197,  1198,  1202,  1204,
    1207,  1210,  1213,  1214,  1217,  1218,  1222,  1224,  1226,  1227,
    1231,  1233,  1235,  1238,  1241,  1244,  1247,  1251,  1258,  1260,
    1263,  1264,  1265,  1267,  1272,  1273,  1276,  1277,  1278,  1282,
    1283,  1285,  1286,  1290,  1292,  1295,  1295,  1299,  1298,  1302,
    1306,  1304,  1317,  1314,  1325,  1327,  1329,  1331,  1333,  1337,
    1338,  1339,  1342,  1348,  1351,  1357,  1360,  1365,  1367,  1372,
    1377,  1381,  1382,  1388,  1389,  1394,  1395,  1400,  1401,  1405,
    1406,  1410,  1412,  1418,  1423,  1424,  1426,  1430,  1431,  1432,
    1433,  1437,  1438,  1439,  1440,  1441,  1442,  1444,  1449,  1452,
    1453,  1457,  1458,  1461,  1462,  1465,  1466,  1469,  1470,  1474,
    1475,  1476,  1477,  1478,  1479,  1482,  1484,  1486,  1487,  1490,
    1492,  1496,  1497,  1499,  1500,  1503,  1507,  1508,  1512,  1513,
    1517,  1521,  1525,  1530,  1531,  1532,  1535,  1537,  1538,  1539,
    1542,  1543,  1544,  1545,  1546,  1547,  1548,  1549,  1550,  1551,
    1552,  1553,  1554,  1555,  1556,  1557,  1558,  1559,  1560,  1561,
    1562,  1563,  1564,  1565,  1566,  1567,  1568,  1569,  1570,  1571,
    1572,  1573,  1574,  1575,  1576,  1577,  1578,  1579,  1580,  1581,
    1582,  1584,  1585,  1587,  1589,  1590,  1591,  1592,  1593,  1594,
    1595,  1596,  1597,  1598,  1599,  1600,  1601,  1602,  1603,  1604,
    1605,  1606,  1608,  1607,  1616,  1615,  1623,  1624,  1628,  1632,
    1636,  1642,  1646,  1652,  1654,  1658,  1660,  1664,  1669,  1670,
    1674,  1681,  1688,  1690,  1695,  1696,  1697,  1701,  1705,  1709,
    1710,  1711,  1712,  1716,  1722,  1727,  1736,  1737,  1740,  1743,
    1746,  1747,  1750,  1754,  1757,  1760,  1767,  1768,  1772,  1773,
    1775,  1779,  1780,  1781,  1782,  1783,  1784,  1785,  1786,  1787,
    1788,  1789,  1790,  1791,  1792,  1793,  1794,  1795,  1796,  1797,
    1798,  1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,
    1808,  1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,
    1818,  1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,
    1828,  1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,
    1838,  1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,  1847,
    1848,  1849,  1850,  1851,  1852,  1853,  1854,  1855,  1856,  1860,
    1865,  1866,  1869,  1870,  1871,  1875,  1876,  1877,  1881,  1882,
    1883,  1887,  1888,  1889,  1892,  1894,  1898,  1899,  1900,  1902,
    1903,  1904,  1905,  1906,  1907,  1908,  1909,  1910,  1911,  1914,
    1919,  1920,  1921,  1922,  1923,  1925,  1926,  1929,  1930,  1934,
    1937,  1943,  1944,  1945,  1946,  1947,  1948,  1949,  1954,  1956,
    1960,  1961,  1964,  1965,  1969,  1972,  1974,  1976,  1980,  1981,
    1982,  1984,  1987,  1991,  1992,  1993,  1996,  1997,  1998,  1999,
    2000,  2002,  2003,  2009,  2011,  2014,  2017,  2019,  2021,  2024,
    2026,  2030,  2032,  2035,  2038,  2044,  2046,  2049,  2050,  2055,
    2058,  2062,  2062,  2067,  2070,  2071,  2075,  2076,  2081,  2082,
    2086,  2087,  2091,  2092,  2097,  2099,  2104,  2105,  2106,  2107,
    2108,  2109,  2110,  2112,  2115,  2117,  2121,  2122,  2123,  2124,
    2125,  2127,  2129,  2131,  2135,  2136,  2137,  2141,  2144,  2147,
    2150,  2154,  2158,  2165,  2169,  2176,  2177,  2182,  2184,  2185,
    2188,  2189,  2192,  2193,  2197,  2198,  2202,  2203,  2204,  2205,
    2207,  2210,  2213,  2214,  2215,  2217,  2219,  2223,  2224,  2225,
    2227,  2228,  2229,  2233,  2235,  2238,  2240,  2241,  2242,  2243,
    2246,  2248,  2249,  2253,  2255,  2258,  2260,  2261,  2262,  2266,
    2268,  2271,  2274,  2276,  2278,  2282,  2283,  2285,  2286,  2292,
    2293,  2295,  2297,  2299,  2301,  2304,  2305,  2306,  2310,  2311,
    2312,  2313,  2314,  2315,  2316,  2320,  2321,  2325,  2333,  2335,
    2339,  2343,  2351,  2352,  2358,  2359,  2367,  2370,  2374,  2377,
    2382,  2383,  2384,  2385,  2389,  2390,  2394,  2396,  2397,  2399,
    2403,  2409,  2411,  2415,  2418,  2421,  2430,  2433,  2436,  2437,
    2440,  2441,  2445,  2450,  2454,  2460,  2468,  2469
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
  "T_DOLLAR_OPEN_CURLY_BRACES", "T_CURLY_OPEN", "T_PAAMAYIM_NEKUDOTAYIM",
  "T_NAMESPACE", "T_NS_C", "T_DIR", "T_NS_SEPARATOR", "T_YIELD",
  "T_XHP_LABEL", "T_XHP_TEXT", "T_XHP_ATTRIBUTE", "T_XHP_CATEGORY",
  "T_XHP_CATEGORY_LABEL", "T_XHP_CHILDREN", "T_XHP_ENUM", "T_XHP_REQUIRED",
  "T_TRAIT", "T_INSTEADOF", "T_TRAIT_C", "T_VARARG", "T_HACK_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "T_NEWTYPE", "T_UNRESOLVED_NEWTYPE",
  "T_COMPILER_HALT_OFFSET", "'('", "')'", "';'", "'{'", "'}'", "'$'",
  "'`'", "']'", "'\"'", "'\\''", "$accept", "start", "$@1",
  "top_statement_list", "top_statement", "$@2", "$@3", "ident",
  "use_declarations", "use_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@4", "$@5",
  "$@6", "$@7", "$@8", "additional_catches", "finally", "$@9",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@10", "$@11",
  "class_declaration_statement", "$@12", "$@13", "$@14", "$@15",
  "trait_declaration_statement", "$@16", "$@17", "class_decl_name",
  "interface_decl_name", "trait_decl_name", "class_entry_type",
  "extends_from", "implements_list", "interface_extends_list",
  "interface_list", "trait_list", "foreach_optional_arg",
  "foreach_variable", "for_statement", "foreach_statement",
  "while_statement", "declare_statement", "declare_list",
  "switch_case_list", "case_list", "case_separator", "elseif_list",
  "new_elseif_list", "else_single", "new_else_single", "parameter_list",
  "non_empty_parameter_list", "function_call_parameter_list",
  "non_empty_fcall_parameter_list", "global_var_list", "global_var",
  "static_var_list", "class_statement_list", "class_statement", "$@18",
  "$@19", "$@20", "$@21", "trait_rules", "trait_precedence_rule",
  "trait_alias_rule", "trait_alias_rule_method", "xhp_attribute_stmt",
  "xhp_attribute_decl", "xhp_attribute_decl_type", "xhp_attribute_enum",
  "xhp_attribute_default", "xhp_attribute_is_required",
  "xhp_category_stmt", "xhp_category_decl", "xhp_children_stmt",
  "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "class_variable_declaration", "class_constant_declaration",
  "expr_with_parens", "parenthesis_expr", "expr_list", "for_expr",
  "yield_expr", "yield_assign_expr", "yield_list_assign_expr", "expr",
  "expr_no_variable", "$@22", "$@23", "shape_keyname",
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
  "static_array_pair_list", "possible_comma",
  "possible_comma_in_hphp_syntax", "non_empty_static_array_pair_list",
  "common_scalar_ae", "static_numeric_scalar_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_shape_pair_list_ae",
  "non_empty_static_shape_pair_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@24", "non_empty_user_attributes",
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
     406,   407,    40,    41,    59,   123,   125,    36,    96,    93,
      34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   182,   184,   183,   185,   185,   186,   186,   186,   186,
     186,   186,   186,   187,   186,   188,   186,   186,   186,   189,
     189,   189,   189,   189,   189,   190,   190,   191,   191,   191,
     191,   192,   192,   193,   193,   193,   194,   195,   196,   197,
     197,   198,   198,   199,   199,   199,   199,   200,   200,   200,
     201,   200,   202,   200,   203,   200,   204,   200,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   205,   200,   200,   200,   200,   200,   200,   200,   200,
     200,   200,   200,   206,   206,   208,   207,   209,   209,   210,
     210,   211,   213,   212,   214,   212,   216,   215,   217,   215,
     218,   215,   219,   215,   221,   220,   222,   220,   223,   223,
     224,   225,   226,   226,   226,   227,   227,   228,   228,   229,
     229,   230,   230,   231,   231,   232,   232,   233,   233,   234,
     234,   235,   235,   236,   236,   237,   237,   238,   238,   239,
     239,   239,   239,   240,   240,   240,   241,   241,   242,   242,
     243,   243,   244,   244,   245,   245,   246,   246,   246,   246,
     247,   247,   247,   247,   247,   247,   247,   247,   248,   248,
     249,   249,   249,   249,   250,   250,   251,   251,   251,   252,
     252,   252,   252,   253,   253,   255,   254,   256,   254,   254,
     257,   254,   258,   254,   254,   254,   254,   254,   254,   259,
     259,   259,   260,   261,   261,   262,   262,   263,   263,   264,
     264,   265,   265,   265,   265,   266,   266,   267,   267,   268,
     268,   269,   269,   270,   271,   271,   271,   272,   272,   272,
     272,   273,   273,   273,   273,   273,   273,   273,   274,   274,
     274,   275,   275,   276,   276,   277,   277,   278,   278,   279,
     279,   279,   279,   279,   279,   280,   280,   280,   280,   281,
     281,   282,   282,   282,   282,   283,   284,   284,   285,   285,
     286,   287,   288,   289,   289,   289,   290,   290,   290,   290,
     290,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   290,   290,   290,   290,   290,   290,   290,   290,
     290,   290,   291,   290,   292,   290,   290,   290,   293,   294,
     294,   295,   295,   296,   296,   297,   297,   298,   299,   299,
     300,   301,   302,   302,   303,   303,   303,   304,   304,   305,
     305,   305,   305,   306,   307,   307,   308,   308,   309,   309,
     310,   310,   311,   312,   312,   313,   313,   313,   314,   314,
     314,   315,   315,   315,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   315,   315,   315,
     315,   315,   315,   315,   315,   315,   315,   315,   315,   316,
     317,   317,   318,   318,   318,   319,   319,   319,   320,   320,
     320,   321,   321,   321,   322,   322,   323,   323,   323,   323,
     323,   323,   323,   323,   323,   323,   323,   323,   323,   323,
     324,   324,   324,   324,   324,   324,   324,   324,   324,   325,
     325,   326,   326,   326,   326,   326,   326,   326,   327,   327,
     328,   328,   329,   329,   330,   330,   330,   330,   331,   331,
     331,   331,   331,   332,   332,   332,   333,   333,   333,   333,
     333,   333,   333,   334,   334,   335,   335,   335,   335,   336,
     336,   337,   337,   338,   338,   339,   339,   340,   340,   341,
     341,   343,   342,   344,   345,   345,   346,   346,   347,   347,
     348,   348,   349,   349,   350,   350,   351,   351,   351,   351,
     351,   351,   351,   351,   351,   351,   352,   352,   352,   352,
     352,   352,   352,   352,   353,   353,   353,   354,   354,   354,
     354,   354,   354,   355,   355,   356,   356,   357,   357,   357,
     358,   358,   359,   359,   360,   360,   361,   361,   361,   361,
     361,   361,   362,   362,   362,   362,   362,   363,   363,   363,
     363,   363,   363,   364,   364,   365,   365,   365,   365,   365,
     365,   365,   365,   366,   366,   367,   367,   367,   367,   368,
     368,   369,   369,   369,   369,   370,   370,   370,   370,   371,
     371,   371,   371,   371,   371,   372,   372,   372,   373,   373,
     373,   373,   373,   373,   373,   374,   374,   375,   376,   376,
     377,   377,   378,   378,   379,   379,   380,   380,   381,   381,
     382,   382,   382,   382,   383,   383,   384,   384,   384,   384,
     385,   386,   386,   387,   387,   388,   389,   389,   389,   389,
     389,   389,   389,   389,   389,   389,   390,   390
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     0,     1,     1,     1,     1,
       1,     4,     3,     0,     6,     0,     5,     3,     2,     1,
       1,     1,     1,     1,     1,     3,     1,     1,     2,     3,
       4,     1,     3,     1,     3,     2,     1,     2,     2,     5,
       4,     2,     0,     1,     1,     1,     1,     3,     5,     8,
       0,     4,     0,     6,     0,    10,     0,     4,     2,     3,
       2,     3,     2,     3,     3,     3,     3,     3,     5,     1,
       1,     0,     9,     5,    14,     5,     3,     3,     2,     2,
       2,     2,     2,     9,     0,     0,     5,     1,     0,     1,
       0,     1,     0,    11,     0,    12,     0,     8,     0,     9,
       0,     7,     0,     8,     0,     7,     0,     8,     1,     1,
       1,     1,     1,     2,     2,     2,     0,     2,     0,     2,
       0,     1,     3,     1,     3,     2,     0,     1,     2,     1,
       4,     1,     4,     1,     4,     1,     4,     3,     5,     3,
       4,     4,     5,     5,     4,     0,     1,     1,     4,     0,
       5,     0,     2,     0,     3,     0,     3,     2,     1,     0,
       3,     4,     6,     5,     5,     6,     8,     7,     2,     0,
       1,     2,     3,     4,     3,     1,     1,     2,     4,     3,
       5,     1,     3,     2,     0,     0,     4,     0,     5,     2,
       0,    10,     0,    11,     3,     3,     3,     3,     5,     2,
       2,     0,     6,     5,     4,     3,     1,     1,     3,     4,
       1,     1,     1,     1,     4,     1,     3,     2,     0,     2,
       0,     1,     3,     1,     1,     1,     1,     3,     4,     4,
       4,     1,     1,     2,     2,     2,     3,     3,     1,     1,
       1,     1,     3,     1,     1,     1,     0,     1,     2,     1,
       1,     1,     1,     1,     1,     3,     5,     1,     3,     5,
       4,     3,     3,     2,     1,     3,     3,     1,     1,     0,
       2,     3,     6,     1,     1,     1,     6,     3,     4,     6,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     1,     1,
       3,     2,     0,    11,     0,    12,     1,     1,     1,     5,
       3,     5,     3,     2,     0,     2,     0,     4,     4,     3,
       4,     4,     4,     4,     1,     1,     3,     5,     0,     3,
       4,     1,     2,     4,     2,     6,     0,     1,     4,     0,
       2,     0,     1,     1,     3,     1,     3,     1,     1,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     1,
       0,     0,     1,     1,     3,     0,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     2,
       1,     1,     2,     2,     4,     3,     4,     1,     1,     3,
       3,     1,     1,     1,     1,     3,     3,     3,     2,     0,
       1,     0,     1,     0,     5,     3,     3,     1,     1,     1,
       1,     3,     2,     1,     1,     1,     1,     1,     2,     2,
       4,     3,     4,     2,     0,     5,     3,     3,     1,     3,
       1,     2,     0,     5,     3,     2,     0,     3,     0,     4,
       2,     0,     3,     3,     1,     0,     1,     2,     2,     4,
       3,     3,     2,     4,     2,     4,     1,     1,     1,     1,
       1,     2,     4,     3,     4,     3,     1,     1,     1,     1,
       2,     4,     4,     3,     1,     1,     3,     7,     6,     8,
       9,     8,    10,     7,     6,     1,     2,     4,     4,     1,
       1,     4,     1,     0,     1,     2,     1,     1,     2,     4,
       3,     3,     0,     1,     2,     4,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     0,     5,     3,     3,     1,     2,
       0,     5,     3,     3,     1,     2,     2,     1,     2,     1,
       4,     3,     3,     6,     3,     1,     1,     1,     4,     4,
       2,     2,     4,     2,     2,     1,     3,     3,     0,     2,
       5,     6,     1,     2,     1,     4,     3,     0,     1,     3,
       3,     1,     1,     0,     0,     2,     3,     1,     5,     3,
       3,     3,     1,     2,     0,     4,     2,     2,     1,     1,
       1,     4,     6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   561,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   634,     0,   622,   480,
       0,   486,   487,    19,   512,   610,    70,   488,     0,    52,
       0,     0,     0,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,   473,     0,     0,
       0,     0,   112,     0,     0,     0,   492,   494,   495,   489,
     490,     0,     0,   496,   491,     0,     0,   471,    20,    21,
      22,    24,    23,     0,   493,     0,     0,     0,     0,   497,
       0,    69,    42,   614,   481,     0,     0,     4,    31,    33,
      36,   511,     0,   470,     0,     6,    90,     7,     8,     9,
       0,   275,     0,     0,     0,     0,   273,   339,   338,   347,
     346,     0,   264,   577,   472,     0,   514,   337,     0,   580,
     274,     0,     0,   578,   579,   576,   605,   609,     0,   327,
     513,    10,   473,     0,     0,    31,    90,   674,   274,   673,
       0,   671,   670,   341,     0,     0,   311,   312,   313,   314,
     336,   334,   333,   332,   331,   330,   329,   328,   473,     0,
     687,   472,     0,   294,   292,     0,   638,     0,   521,   263,
     476,     0,   687,   475,     0,   485,   617,   616,   477,     0,
       0,   479,   335,     0,     0,     0,   267,     0,    50,   269,
       0,     0,    56,     0,    58,     0,     0,    60,     0,     0,
     709,   713,     0,     0,    31,   708,     0,   710,     0,    62,
       0,    42,     0,     0,     0,    26,    27,   176,     0,     0,
     175,   114,   113,   181,    90,     0,     0,     0,     0,     0,
     684,   100,   110,   630,   634,   659,     0,   499,     0,     0,
       0,   657,     0,    15,     0,    35,     0,   270,   104,   111,
     379,   354,     0,   678,   275,     0,   273,   274,     0,     0,
     482,     0,   483,     0,     0,     0,    82,     0,     0,    38,
     169,     0,    18,    89,     0,   109,    96,   108,    79,    80,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   622,    78,   613,   613,
     644,     0,     0,     0,    90,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   293,   291,
       0,   581,   566,   613,     0,   572,   169,   613,     0,   615,
     606,   630,     0,     0,     0,   563,   558,   521,     0,     0,
       0,     0,   642,     0,   359,   520,   633,     0,     0,    38,
       0,   169,   262,     0,   618,   566,   574,   478,     0,    42,
     149,     0,    67,     0,     0,   268,     0,     0,     0,     0,
       0,    59,    77,    61,   706,   707,     0,   704,     0,     0,
     688,     0,   683,    63,     0,    76,    28,     0,    17,     0,
       0,   177,     0,    65,     0,     0,     0,    66,   675,     0,
       0,     0,     0,     0,   120,     0,   631,     0,     0,     0,
       0,   498,   658,   512,     0,     0,   656,   517,   655,    34,
       5,    12,    13,    64,   118,     0,     0,   348,     0,   521,
       0,     0,     0,     0,   261,   324,   585,    47,    41,    43,
      44,    45,    46,     0,   340,   515,   516,    32,     0,     0,
       0,   523,   170,     0,   342,    92,   116,   297,   299,   298,
       0,     0,   295,   296,   300,   302,   301,   316,   315,   318,
     317,   319,   321,   322,   320,   310,   309,   304,   305,   303,
     306,   307,   308,   323,   612,     0,     0,   648,     0,   521,
     677,   583,   605,   102,   106,     0,    98,     0,     0,   271,
     277,   290,   289,   288,   287,   286,   285,   284,   283,   282,
     281,   280,     0,   568,   567,     0,     0,     0,     0,     0,
       0,   672,   556,   560,   520,   562,     0,     0,   687,     0,
     637,     0,   636,     0,   621,   620,     0,     0,   568,   567,
     265,   151,   153,   266,     0,    42,   133,    51,   269,     0,
       0,     0,     0,   145,   145,    57,     0,     0,   702,   521,
       0,   693,     0,     0,     0,   519,     0,     0,   471,     0,
      36,   501,   470,   508,     0,   500,    40,   507,    85,     0,
      25,    29,     0,   174,   182,   344,   179,     0,     0,   668,
     669,    11,   697,     0,     0,     0,   630,   627,     0,   358,
     667,   666,   665,     0,   661,     0,   662,   664,     0,     5,
       0,     0,   373,   374,   382,   381,     0,     0,   520,   353,
     357,     0,   679,     0,     0,   582,   566,   573,   611,     0,
     686,   171,   469,   522,   168,     0,   565,     0,     0,   118,
     326,     0,   362,   363,     0,   360,   520,   643,     0,   169,
     120,   118,    94,   116,   622,   278,     0,     0,   169,   570,
     571,   584,   607,   608,     0,     0,     0,   544,   528,   529,
     530,     0,     0,     0,   537,   536,   550,   521,     0,   558,
     641,   640,     0,   619,   566,   575,   484,     0,   155,     0,
       0,    48,     0,     0,     0,     0,   126,   127,   137,     0,
      42,   135,    73,   145,     0,   145,     0,     0,   711,     0,
     520,   703,   705,   692,   691,     0,   689,   502,   503,   527,
       0,   521,   519,     0,     0,   356,     0,   650,     0,    75,
       0,    30,   178,   565,     0,   676,    68,     0,     0,   685,
     119,   121,   184,     0,     0,   628,     0,   660,     0,    16,
       0,   117,   184,     0,     0,   350,     0,   680,     0,     0,
     568,   567,   689,     0,   172,    39,   158,     0,   523,   564,
     717,   565,   115,     0,   325,   647,   646,   169,     0,     0,
       0,     0,   118,   485,   569,   169,     0,     0,   533,   534,
     535,   538,   539,   548,     0,   521,   544,     0,   532,   552,
     520,   555,   557,   559,     0,   635,   569,     0,     0,     0,
       0,   152,    53,     0,   269,   128,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   139,     0,   700,   701,     0,
       0,   715,     0,   505,   520,   518,     0,   510,     0,   521,
       0,   509,   654,     0,   521,     0,     0,     0,   180,   699,
     696,     0,   246,   632,   630,   272,   276,     0,    14,   246,
     385,     0,     0,   387,   380,   383,     0,   378,     0,   681,
       0,     0,   169,   173,   694,   565,   157,   716,     0,     0,
     184,     0,     0,   604,   184,   184,   565,     0,   279,   169,
       0,   598,     0,   541,   520,   543,     0,   531,     0,     0,
     521,   549,   639,     0,    42,     0,   148,   134,     0,   125,
      71,   138,     0,     0,   141,     0,   146,   147,    42,   140,
     712,   690,     0,   526,   525,   504,     0,   520,   355,   506,
       0,   361,   520,   649,     0,    42,   694,     0,   122,     0,
       0,   249,   250,   251,   254,   253,   252,   244,     0,     0,
       0,   101,   183,   185,     0,   243,   247,     0,   246,     0,
     663,   105,   376,     0,     0,   349,   569,   169,     0,     0,
     368,   156,   717,     0,   160,   694,   246,   645,   603,   246,
     246,     0,   184,     0,   597,   547,   546,   540,     0,   542,
     520,   551,    42,   154,    49,    54,     0,   136,   142,    42,
     144,     0,     0,   352,     0,   653,   652,     0,     0,   368,
     698,     0,     0,   123,   213,   211,   471,    24,     0,   207,
       0,   212,   223,     0,   221,   226,     0,   225,     0,   224,
       0,    90,   248,   187,     0,   189,     0,   245,   629,   377,
     375,   386,   384,   169,     0,   601,   695,     0,     0,     0,
     161,     0,     0,    97,   103,   107,   694,   246,   599,     0,
     554,     0,   150,     0,    42,   131,    72,   143,   714,   524,
       0,     0,     0,    86,     0,     0,     0,   197,   201,     0,
       0,   194,   436,   435,   432,   434,   433,   452,   454,   453,
     424,   414,   430,   429,   392,   401,   402,   404,   403,   423,
     407,   405,   406,   408,   409,   410,   411,   412,   413,   415,
     416,   417,   418,   419,   420,   422,   421,   393,   394,   395,
     397,   398,   400,   438,   439,   448,   447,   446,   445,   444,
     443,   431,   449,   440,   441,   442,   425,   426,   427,   428,
     450,   451,   455,   457,   456,   458,   459,   437,   461,   460,
     396,   463,   465,   464,   399,   468,   466,   467,   462,   391,
     218,   388,     0,   195,   239,   240,   238,   231,     0,   232,
     196,   257,     0,     0,     0,     0,    90,     0,   600,     0,
      42,     0,   164,     0,   163,    42,     0,    99,   545,     0,
      42,   129,    55,     0,   351,   651,    42,    42,   260,   124,
       0,     0,   215,   208,     0,     0,     0,   220,   222,     0,
       0,   227,   234,   235,   233,     0,     0,   186,     0,     0,
       0,     0,   602,     0,   371,   523,     0,   165,     0,   162,
       0,    42,   553,     0,     0,     0,     0,   198,    31,     0,
     199,   200,     0,     0,   214,   217,   389,   390,     0,   209,
     236,   237,   229,   230,   228,   258,   255,   190,   188,   259,
       0,   372,   522,     0,   343,     0,   167,    93,     0,     0,
     132,    84,   345,     0,   246,   216,   219,     0,   565,   192,
       0,   369,   367,   166,    95,   130,    88,   205,     0,   245,
     256,     0,   565,   370,     0,    87,    74,     0,     0,   204,
     694,     0,     0,     0,   203,     0,   694,     0,   202,   241,
      42,   191,     0,     0,     0,   193,     0,   242,    42,     0,
      83
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    97,   629,   440,   145,   224,   225,
      99,   100,   101,   102,   103,   104,   268,   458,   459,   384,
     197,  1083,   390,  1016,  1306,   749,   750,  1316,   284,   146,
     460,   657,   801,   461,   476,   673,   424,   670,   462,   444,
     671,   286,   241,   258,   110,   659,   631,   615,   760,  1032,
     837,   716,  1212,  1086,   567,   722,   389,   575,   724,   938,
     562,   708,   711,   829,   787,   788,   470,   471,   229,   230,
     235,   872,   972,  1050,  1194,  1298,  1312,  1220,  1260,  1261,
    1262,  1038,  1039,  1040,  1221,  1227,  1269,  1043,  1044,  1048,
    1187,  1188,  1189,  1331,   973,   974,   975,   976,  1192,   977,
     111,   191,   385,   386,   112,   113,   114,   115,   116,   656,
     753,   448,   449,   859,   450,   860,   117,   118,   119,   593,
     120,   121,  1068,  1245,   122,   445,  1060,   446,   773,   636,
     887,   884,  1180,  1181,   123,   124,   125,   185,   192,   271,
     372,   126,   739,   597,   127,   740,   366,   654,   741,   695,
     811,   813,   814,   815,   697,   919,   920,   698,   543,   357,
     154,   155,   128,   790,   341,   342,   647,   129,   186,   148,
     131,   132,   133,   134,   135,   136,   137,   505,   138,   188,
     189,   427,   177,   178,   508,   509,   863,   864,   250,   251,
     623,   139,   419,   140,   453,   141,   216,   242,   279,   399,
     735,   990,   613,   578,   579,   580,   217,   218,   898
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1107
static const yytype_int16 yypact[] =
{
   -1107,   108, -1107, -1107,  3500,  8714,  8714,   -40,  8714,  8714,
    8714, -1107,  8714,  8714,  8714,  8714,  8714,  8714,  8714,  8714,
    8714,  8714,  8714,  8714,  2133,  2133,  6818,  8714,  2236,   -37,
       3, -1107, -1107, -1107, -1107, -1107, -1107, -1107,  8714, -1107,
       3,   107,   122,   144,     3,     4,  1035,    53, -1107,  1510,
    6976,   130,  8714,   750,    -6,   179,   190,   280,   165,   172,
     195,   215, -1107,  1035,   232,   252, -1107, -1107, -1107, -1107,
   -1107,   723,   737, -1107, -1107,  1035,  7134, -1107, -1107, -1107,
   -1107, -1107, -1107,  1035, -1107,   180,   267,  1035,  1035, -1107,
    8714, -1107, -1107,   257,    31,   236,   236, -1107,   408,   301,
     300, -1107,   277, -1107,    46, -1107,   424, -1107, -1107, -1107,
    1414, -1107,   285,   287,   289,  9468, -1107, -1107,   403, -1107,
     407,   413, -1107,   101,   303,   336, -1107, -1107,   494,    14,
    1147,   111,   318,   133,   135,   320,    96, -1107,    15, -1107,
     432, -1107,   416,   343,   373, -1107,   424, 10038,  1676, 10038,
    8714, 10038, 10038,  9029,   484,  1035, -1107, -1107,   477, -1107,
   -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,  1872,
     367, -1107,   396,   422,   422,  2133,  9698,   358,   537, -1107,
     415,  1872,   367,   423,   426,   375,   137, -1107,   444,   111,
    7292, -1107, -1107,  8714,  2682,    48, 10038,  6660, -1107,  8714,
    8714,  1035, -1107,   395, -1107,   399,   406, -1107,  1510,  1510,
     428, -1107,   398,   547,   571, -1107,   576, -1107,  1035, -1107,
    9509, -1107,  9550,  1035,    54, -1107,    17, -1107,  1827,    55,
   -1107, -1107, -1107,   578,   424,    57,  2133,  2133,  2133,   421,
     434, -1107, -1107,  1657,  6818,    16,   368, -1107,  8872,  2133,
     838, -1107,  1035, -1107,   -29,   301,   431, 10038, -1107, -1107,
   -1107,   518,   586,   511,   436, 10038,   437,  1134,  3658,  8714,
       1,   447,   374,     1,   342,   226, -1107,  1035,  1510,   443,
    7450,  1510, -1107, -1107,   798, -1107, -1107, -1107, -1107, -1107,
   -1107,  8714,  8714,  8714,  7608,  8714,  8714,  8714,  8714,  8714,
    8714,  8714,  8714,  8714,  8714,  8714,  8714,  8714,  8714,  8714,
    8714,  8714,  8714,  8714,  8714,  8714,  2236, -1107,  8714,  8714,
    8714,   690,  1035,  1035,   424,  1414,  2867,  8714,  8714,  8714,
    8714,  8714,  8714,  8714,  8714,  8714,  8714,  8714, -1107, -1107,
     260, -1107,   138,  8714,  8714, -1107,  7450,  8714,  8714,   257,
     145,  1657,   450,  7766,  9591, -1107,   458,   631,  1872,   468,
     -19,   690,   422,  7924, -1107,  8082, -1107,   469,   -18, -1107,
     251,  7450, -1107,   480, -1107,   147, -1107, -1107,  9635, -1107,
   -1107,  8714, -1107,   561,  6028,   638,   473,  9931,   636,    50,
      95, -1107, -1107, -1107, -1107, -1107,  1510,   573,   486,   653,
   -1107,  9643, -1107, -1107,  3816, -1107,    37,   750, -1107,  1035,
    8714,   422,    -6, -1107,  9643,   491,   597, -1107,   422,    52,
      82,    -7,   499,  1035,   553,   502,   422,    85,   503,  1170,
    1035, -1107, -1107,   616,  1563,    -9, -1107, -1107, -1107,   301,
   -1107, -1107, -1107, -1107,   559,   519,    34, -1107,   562,   677,
     521,  1510,  1510,   684,   231,   640,   123, -1107, -1107, -1107,
   -1107, -1107, -1107,  3026, -1107, -1107, -1107, -1107,    74,  2133,
     529,   695, 10038,   691, -1107, -1107,   588, 10078,  3137,  9029,
    8714,  9997, 10100, 10121,  6868,  2540,  6652,  6968,  6968,  6968,
    6968,  2419,  2419,  2419,  2419,  1153,  1153,   574,   574,   574,
     477,   477,   477, -1107, 10038,   526,   532,  9794,   539,   709,
     -31,   546,   145, -1107, -1107,  1035, -1107,  1566,  8714, -1107,
    9029,  9029,  9029,  9029,  9029,  9029,  9029,  9029,  9029,  9029,
    9029,  9029,  8714,   -31,   549,   545,  3100,   557,   555,  9181,
      88, -1107,   950, -1107,  1035, -1107,   436,   231,   367,  2133,
   10038,  2133,  9835,   241,   148, -1107,   564,  8714, -1107, -1107,
   -1107,  5870,    62, 10038,     3, -1107, -1107, -1107,  8714,   679,
    9643,  1035,  6186,   565,   567, -1107,   102,   621, -1107,   730,
     583,  1210,  1510,  9643,  9643,  9643,   577,    13,   618,   590,
     282, -1107,   620, -1107,   589, -1107, -1107, -1107,   663,  1035,
   -1107, -1107,  9222, -1107, -1107, -1107,   754,  2133,   595, -1107,
   -1107, -1107,   693,   103,   969,   606,  1657,  2059,   772, -1107,
   -1107, -1107, -1107,   607, -1107,  8714, -1107, -1107,  3184, -1107,
     969,   612, -1107, -1107, -1107, -1107,   775,  8714,   518, -1107,
   -1107,   615, -1107,  1510,   580, -1107,   150, -1107, -1107,  1510,
   -1107,   422, -1107,  8240, -1107,  9643,   118,   619,   969,   559,
    9954,  8714, -1107, -1107,  8714, -1107,  8714, -1107,   625,  7450,
     553,   559, -1107,   588,  2236,   422,  9263,   626,  7450, -1107,
   -1107,   154, -1107, -1107,   787,   516,   516,   950, -1107, -1107,
   -1107,   633,    21,   639, -1107, -1107, -1107,   805,   641,   458,
     422,   422,  8398, -1107,   160, -1107, -1107,  9304,    76,     3,
    6660, -1107,   644,  3974,   645,  2133,   700,   422, -1107,   802,
   -1107, -1107, -1107, -1107,   409, -1107,   255,  1510, -1107,  1510,
     573, -1107, -1107, -1107,   816,   656,   664, -1107, -1107,   722,
     666,   839,  9643,   711,  1035,   518,  1035,  9643,   678, -1107,
     696, -1107, -1107,   118,  9643,   422, -1107,  1510,  1035, -1107,
     846, -1107, -1107,    89,   686,   422,  8556, -1107,  2391, -1107,
    3342,   846, -1107,   296,   -27, 10038,   741, -1107,   692,  8714,
     -31,   715, -1107,  2133, 10038, -1107, -1107,   710,   860, -1107,
    1510,   118, -1107,   717,  9954, 10038,  9890,  7450,   720,   725,
     731,   733,   559,   375,   746,  7450,   724,  8714, -1107, -1107,
   -1107, -1107, -1107,   773,   734,   911,   950,   786, -1107,   518,
     950, -1107, -1107, -1107,  2133, 10038, -1107,     3,   899,   858,
    6660, -1107, -1107,   762,  8714,   422,   679,   764,  9643,  4132,
     427,   769,  8714,    38,   262, -1107,   777, -1107, -1107,  1266,
     912, -1107,  9643, -1107,  9643, -1107,   789, -1107,   826,   955,
     791, -1107,   847,   792,   965,   969,   803,   806, -1107, -1107,
     891,   969,   766, -1107,  1657, -1107,  9029,   807, -1107,   823,
   -1107,   281,  8714, -1107, -1107, -1107,  8714, -1107,  8714, -1107,
    9345,   810,  7450,   422,   958,   124, -1107, -1107,   297,   812,
   -1107,  8714,   814, -1107, -1107, -1107,   118,   813, -1107,  7450,
     817, -1107,   950, -1107,   950, -1107,   818, -1107,   872,   822,
     988, -1107,   422,   974, -1107,   831, -1107, -1107,   834, -1107,
   -1107, -1107,   843,   844, -1107,  2461, -1107, -1107, -1107, -1107,
   -1107, -1107,  1510, -1107,   903, -1107,  9643,   518, -1107, -1107,
    9643, -1107,  9643, -1107,   940, -1107,   958,  1510, -1107,  1510,
     969, -1107, -1107, -1107, -1107, -1107, -1107, -1107,  2409,   877,
     382, -1107, -1107, -1107,   416,  2043, -1107,    58,  1319,    90,
   -1107, -1107,   882,  9386,  9427, 10038,   859,  7450,   861,  1510,
     928, -1107,  1510,   960,  1022,   958,  1163, 10038, -1107,  1383,
    1684,   864, -1107,   866, -1107, -1107,   919, -1107,   950, -1107,
     518, -1107, -1107,  5870, -1107, -1107,  6344, -1107, -1107, -1107,
    5870,   870,  9643, -1107,   922, -1107,   924,   875,  4290,   928,
   -1107,  1038,    44, -1107, -1107, -1107,    60,   878,    61, -1107,
    9030, -1107, -1107,    68, -1107, -1107,   808, -1107,   881, -1107,
     983,   424, -1107, -1107,  1510, -1107,   416,  1319, -1107, -1107,
   -1107, -1107, -1107,  7450,   884, -1107, -1107,   886,   887,   299,
    1048,  9643,   888, -1107, -1107, -1107,   958,  1821, -1107,   950,
   -1107,   942,  5870,  6502, -1107, -1107, -1107,  5870, -1107, -1107,
    9643,  9643,   892, -1107,   893,  9643,   969, -1107, -1107,  1931,
    2409, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
     121, -1107,   877, -1107, -1107, -1107, -1107, -1107,    43,   425,
   -1107,  1053,    77,  1035,   983,  1056,   424,   897, -1107,   309,
   -1107,   999,  1060,  9643, -1107, -1107,   901, -1107, -1107,   950,
   -1107, -1107, -1107,  4448, -1107, -1107, -1107, -1107, -1107, -1107,
     347,    41, -1107, -1107,  9643,  9030,  9030,  1029, -1107,   808,
     808,   507, -1107, -1107, -1107,  9643,  1008, -1107,   910,    78,
    9643,  1035, -1107,  1011, -1107,  1077,  4606,  1074,  9643, -1107,
    4764, -1107, -1107,  4922,   916,  5080,  5238, -1107,  1001,   954,
   -1107, -1107,  1005,  1931, -1107, -1107, -1107, -1107,   943, -1107,
    1071, -1107, -1107, -1107, -1107, -1107,  1094, -1107, -1107, -1107,
     936, -1107,   311,   939, -1107,  9643, -1107, -1107,  5396,   949,
   -1107, -1107, -1107,  1035,  1319, -1107, -1107,  9643,   118, -1107,
    1052, -1107, -1107, -1107, -1107, -1107,   -15,   971,  1035,   376,
   -1107,   956,   118, -1107,   959, -1107, -1107,   969,   953, -1107,
     958,   957,   969,    86, -1107,   -58,   958,  1062, -1107, -1107,
   -1107, -1107,   -58,   964,  5554, -1107,   963, -1107, -1107,  5712,
   -1107
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1107, -1107, -1107,  -379, -1107, -1107, -1107,    -4, -1107,   735,
     -22,   896,  -238, -1107,  1308, -1107,  -202, -1107,     6, -1107,
   -1107, -1107, -1107, -1107, -1107,  -166, -1107, -1107,  -142,    32,
       7, -1107, -1107,     8, -1107, -1107, -1107, -1107,     9, -1107,
   -1107,   819,   821,   849,  1013,   500,  -558,   476,   548,  -141,
   -1107,   339, -1107, -1107, -1107, -1107, -1107, -1107,  -441,   244,
   -1107, -1107, -1107, -1107,  -750, -1107,  -328, -1107, -1107,   768,
   -1107,  -717, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107,    91, -1107, -1107, -1107, -1107, -1107,    19, -1107,
     220, -1106, -1107,  -140, -1107,  -940,  -934,  -938,    20, -1107,
     -43,   -23,  1165,  -528,  -318, -1107, -1107,  2319,  1118, -1107,
   -1107,  -609, -1107, -1107, -1107, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107,   181, -1107,   438, -1107, -1107, -1107, -1107, -1107,
   -1107, -1107, -1107,  -796, -1107,  1157,  1206,  -277, -1107, -1107,
     410,  -296,  -344, -1107, -1107,   475,  -276,  -772, -1107, -1107,
     533,  -519,   402, -1107, -1107, -1107, -1107, -1107,   525, -1107,
   -1107, -1107,  -623,   325,  -164,  -161,  -111, -1107, -1107,    10,
   -1107, -1107, -1107, -1107,    18,  -116, -1107,  -194, -1107, -1107,
   -1107,  -349,   984, -1107, -1107, -1107, -1107, -1107,   481,   498,
   -1107, -1107,   978, -1107, -1107, -1107,  -272,   -82,  -167,  -230,
   -1107,  -924, -1107,   497, -1107, -1107, -1107,  -183,   237
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -688
static const yytype_int16 yytable[] =
{
      98,   259,   540,   867,   352,   262,   263,   194,   519,   473,
     105,   107,   108,   109,   130,   369,   896,   198,   537,   404,
     345,   202,   350,   696,   374,   394,   395,   375,   287,   776,
     400,   226,  1029,   789,   173,   174,   106,  1052,  1056,   503,
     714,   899,   205,   556,  1057,   214,   187,   264,   468,  1263,
     254,  1229,  1096,   255,   281,   879,   381,   596,   571,   240,
     607,   628,   407,   412,   936,   416,  1054,   227,  -210,  1100,
     604,  1072,  1230,   203,   245,  -589,  1182,   429,   376,   240,
     633,   545,   649,   240,   240,  1236,  1236,  1314,    35,   234,
     607,   743,   415,   617,  1096,   400,   617,   617,   617,   817,
     267,   793,   340,   340,   245,   595,   240,   409,     3,   270,
     727,   758,   340,   800,   340,   277,  1329,  1330,   595,  1052,
     885,   573,   206,  1270,  1271,   506,   359,   599,   709,   710,
     789,   278,   150,   726,  1224,   190,   858,   430,   367,   248,
     249,  -687,   827,   828,   -85,   441,   442,  1225,   886,   535,
     247,   356,  1206,   538,   456,   554,  1001,   347,   818,    11,
     324,   277,  -586,   591,  1226,    11,   610,   627,   789,   248,
     249,   228,   343,   639,   234,   193,   591,   561,   204,   360,
     634,   277,   515,   996,  -593,   362,  -595,   999,  1000,  -589,
      98,   368,   349,    98,  -587,   635,  -588,   388,  -623,  -590,
     380,   406,   475,   383,   130,   512,   347,   130,  -624,  -626,
     918,  -591,   937,   576,   402,  -592,  1231,  1264,  1097,  1098,
     282,  -625,   382,   572,   512,   608,   718,   207,   408,   413,
     439,   417,  1055,   667,  -210,  1101,  -474,   650,   411,   737,
     738,   259,  1183,   287,   907,   512,   418,   418,   421,   978,
     770,  1237,  1278,   426,   512,   609,   978,   512,   618,   435,
    1328,   684,   873,  1058,    98,   728,   759,   763,   641,   642,
     574,   348,   789,   467,   595,   786,  -586,   214,   130,   199,
     240,   991,   840,   789,   844,  1077,   344,   595,   595,   595,
     645,  -159,   343,   646,   200,  -596,   231,  -522,  -593,   245,
     106,   921,   343,   731,   436,   221,   928,   232,  -587,   245,
    -588,   785,  -623,  -590,   273,   546,   201,   510,   240,   240,
     348,   240,  -624,  -626,    35,  -591,   260,   982,   993,  -592,
    1201,    33,   591,    35,   187,  -625,   533,   236,  1024,   511,
    1243,   798,  1300,   668,   237,   591,   591,   591,   842,   843,
     806,   734,   644,   233,  1308,   842,   843,   548,   534,   595,
    1309,   426,   373,   713,   248,   249,   677,   238,   360,   558,
     994,  1052,  1202,   978,   248,   249,   978,   978,    48,   511,
      98,   668,  1244,   645,  1301,   226,   646,   239,   555,   703,
     566,   559,   704,  1005,   130,  1006,  1325,   803,   400,   736,
      98,  1081,  1332,   862,   243,   601,   344,   466,    78,    79,
     868,    80,    81,    82,   130,   245,   344,   591,    33,   612,
     436,   821,  -687,   369,   244,   622,   624,   260,    93,  1266,
    1267,   845,   269,   672,   276,   532,   106,    93,   939,   261,
    -687,   245,   705,   880,   278,   277,   595,   245,   875,   280,
    1232,   595,   436,    33,   978,   283,   881,  -687,   595,   288,
     778,   289,   278,   290,  -364,   855,   782,  1233,   318,   902,
    1234,   882,  -687,  1283,   319,  -687,   321,   910,   320,   651,
     248,   249,   961,   962,   963,   964,   965,   966,   144,  1080,
     346,    75,  -594,  -365,   931,    78,    79,  1045,    80,    81,
      82,   841,   842,   843,   591,   431,   248,   249,   943,   591,
     944,   240,   248,   249,    48,   351,   591,   252,   839,   933,
     842,   843,   465,  1257,   355,   979,   316,   675,   512,   278,
      78,    79,  1272,    80,    81,    82,   361,   364,   694,   915,
     699,   712,   595,   340,   846,   365,   847,   371,  1311,  1273,
    1319,    33,  1274,    35,  1046,  -473,   595,    98,   595,   700,
    1208,   701,  1321,  -472,   988,   373,   370,   719,    98,   391,
     397,   130,   208,   392,   869,   272,   274,   275,   721,   717,
     393,  1003,   130,   948,  -682,   808,   809,    33,   953,   401,
     396,   414,    48,   106,   422,   751,   423,   447,   209,   451,
     591,   452,  1023,    55,    56,   443,  1025,   897,  1026,   454,
     455,    62,   322,   891,   591,   -37,   591,   755,    33,   313,
     314,   315,   474,   316,    98,   464,   426,   765,    78,    79,
     542,    80,    81,    82,   105,   107,   108,   109,   130,   544,
     780,   547,   553,   564,  1011,   398,   381,   568,   323,   570,
     595,    33,   577,    35,   595,   557,   595,    93,   581,  1064,
     106,   582,   781,   605,    78,    79,   782,    80,    81,    82,
     606,   210,   614,   611,   616,   789,   619,   625,  1089,   630,
     632,   810,   810,   694,   637,   638,   830,  1031,   144,   789,
    1252,    75,   187,   211,   640,    78,    79,   643,    80,    81,
      82,  -366,   652,   653,   655,   662,    98,   658,   591,    98,
     715,   663,   591,   212,   591,   665,   831,   666,   669,   213,
     130,   678,  1013,   130,   679,   835,   595,  1204,    78,    79,
     681,    80,    81,    82,   682,  1197,  1020,   706,   730,   723,
     857,   725,   861,   729,   432,   106,  1214,  1215,   438,   742,
      33,  1218,    35,  1028,   870,   779,   732,    93,   744,  1021,
     746,    33,   745,    35,   747,   748,    98,   754,   432,   756,
     438,   432,   438,   438,  1030,   595,   105,   107,   108,   109,
     130,   762,  1195,   757,   591,   766,   767,   772,   774,   777,
     168,   791,  1053,   893,   595,   595,   245,   797,   805,   595,
     807,   246,   106,  1222,   923,   816,  1066,    11,    33,   897,
    1082,   819,   694,   820,   822,   838,   694,  1087,   832,   834,
     144,    33,   836,    75,   849,    77,    98,    78,    79,   850,
      80,    81,    82,   591,   922,    98,   926,   851,    78,    79,
     130,    80,    81,    82,   852,   853,   717,   854,   431,   130,
     865,   169,   591,   591,   871,   866,    93,   591,   874,  1249,
     247,   248,   249,   888,    11,   959,   889,    93,   895,    33,
     960,   106,   961,   962,   963,   964,   965,   966,   967,    33,
    1265,   252,  1213,   894,   426,    78,    79,   892,    80,    81,
      82,  1275,   900,   903,   223,   912,  1279,   911,    78,    79,
     904,    80,    81,    82,  1286,   906,   905,   595,   694,  1193,
     694,   245,   253,   913,   968,   969,   436,   970,   909,   914,
     170,   170,   959,   917,   182,   924,   925,   960,   595,   961,
     962,   963,   964,   965,   966,   967,   927,   930,   942,   595,
     940,  1303,   971,   934,   595,   182,    78,    79,   946,    80,
      81,    82,   595,  1310,  1184,   214,    78,    79,  1185,    80,
      81,    82,   945,   947,   949,   591,  1047,  1295,   951,   950,
     474,   968,   969,   952,   970,   437,   248,   249,   955,   956,
    1046,   957,   987,   980,   989,   995,   591,   998,  1002,   595,
    1004,  1007,   685,   686,  1008,  1009,  1010,   591,  1246,   981,
    1012,   595,   591,  1250,   694,  1014,  1051,  1015,  1253,    98,
     591,   687,    98,  1027,  1255,  1256,    98,  1017,  1018,   688,
     689,    33,  1085,   130,    98,  1022,   130,  1042,  1059,   690,
     130,  1063,  1067,  1070,  1065,  1071,  1179,  1076,   130,  1078,
      33,  1079,  1186,  1088,  1090,   106,  1091,   591,  1092,  1288,
     214,  1095,   106,  1099,  1241,  1190,  1191,  1198,  1199,   591,
     106,  1203,  1200,  1205,  1209,   170,  1235,  1216,  1217,  1240,
    1242,   170,  1247,  1248,   691,   694,  1251,   170,    98,    98,
    1268,  1276,  1277,    98,  1281,  1282,   692,  1285,  1196,  1211,
    1290,  -206,   130,   130,  1293,  1294,  1296,   130,    78,    79,
    1230,    80,    81,    82,   182,   182,    33,  1297,  1299,   182,
     144,  1238,  1302,    75,   106,    77,   693,    78,    79,   106,
      80,    81,    82,  1305,   170,  1313,  1317,  1324,  1334,  1320,
    1326,  1322,   170,   170,   170,  1333,  1339,  1336,  1338,   170,
    1315,   325,   600,   513,   516,   170,   799,   353,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,  1280,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   514,   802,   182,   929,  1323,   182,   771,  1019,
     603,   171,   171,    78,    79,   183,    80,    81,    82,   240,
    1049,  1223,  1335,   338,   339,   310,   311,   312,   313,   314,
     315,  1228,   316,   195,    11,   694,   338,   339,   266,    98,
    1094,   883,   182,   908,  1239,   420,  1258,   856,   916,   812,
     992,  1179,  1179,   130,   823,  1186,  1186,   848,   428,  1069,
     172,   172,     0,     0,   184,   208,     0,   240,     0,     0,
       0,    33,    98,   620,   621,   106,    98,   170,     0,    98,
       0,    98,    98,     0,   170,   340,   130,     0,     0,     0,
     130,   209,   959,   130,     0,   130,   130,   960,   340,   961,
     962,   963,   964,   965,   966,   967,     0,     0,   106,     0,
       0,    33,   106,     0,    98,   106,     0,   106,   106,  1307,
       0,   208,   182,     0,     0,     0,     0,   590,   130,     0,
       0,     0,     0,     0,  1318,     0,     0,   456,     0,     0,
     590,   968,   969,     0,   970,     0,     0,   209,    78,    79,
     106,    80,    81,    82,     0,     0,   171,     0,     0,     0,
      98,     0,   171,     0,   210,    98,     0,    33,   171,  1073,
       0,     0,     0,     0,   130,     0,     0,   182,   182,   130,
       0,   144,     0,     0,    75,     0,   211,   215,    78,    79,
       0,    80,    81,    82,     0,   170,   106,   733,     0,     0,
       0,   106,     0,     0,     0,   172,   212,     0,     0,     0,
       0,   172,   213,     0,     0,   171,     0,   172,     0,     0,
     210,     0,     0,   171,   171,   171,     0,     0,     0,     0,
     171,     0,     0,     0,     0,     0,   171,   144,     0,     0,
      75,     0,   211,   170,    78,    79,     0,    80,    81,    82,
       0,     0,     0,   941,    11,   961,   962,   963,   964,   965,
     966,     0,   212,     0,   172,     0,     0,     0,   213,     0,
       0,     0,   172,   172,   172,   170,     0,   170,     0,   172,
       0,     0,     0,     0,     0,   172,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   170,   590,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,   182,   182,   590,
     590,   590,   959,     0,     0,    33,     0,   960,     0,   961,
     962,   963,   964,   965,   966,   967,     0,     0,     0,     0,
       0,     0,     0,   170,     0,     0,     0,     0,   171,     0,
     182,     0,   170,   170,     0,   171,   215,   215,     0,     0,
       0,   215,   184,     0,     0,     0,   182,     0,     0,     0,
       0,   968,   969,     0,   970,   208,     0,     0,     0,   182,
       0,     0,     0,     0,     0,   182,     0,     0,     0,     0,
       0,   590,     0,     0,   182,     0,     0,   172,   594,  1074,
     285,   209,    78,    79,   172,    80,    81,    82,     0,     0,
     182,   594,   291,   292,   293,     0,     0,     0,     0,     0,
       0,    33,     0,     0,     0,     0,   215,     0,   294,   215,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   170,   316,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   182,     0,   182,   171,     0,     0,   674,
       0,     0,     0,     0,   210,     0,     0,    33,   590,    35,
       0,     0,     0,   590,     0,     0,     0,     0,     0,     0,
     590,   144,     0,   182,    75,     0,   211,     0,    78,    79,
       0,    80,    81,    82,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   171,   172,   212,   168,     0,   170,
       0,     0,   213,     0,     0,     0,   182,     0,     0,   353,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,     0,     0,     0,   215,     0,   171,   144,   171,   592,
      75,     0,    77,     0,    78,    79,     0,    80,    81,    82,
     170,     0,   592,   172,     0,    11,   171,   594,    33,     0,
      35,     0,   170,     0,   590,   338,   339,     0,   169,   626,
     594,   594,   594,    93,     0,   182,     0,     0,   590,     0,
     590,     0,     0,     0,     0,   172,     0,   172,     0,   215,
     215,   182,     0,     0,   171,     0,     0,   182,   168,     0,
     170,   761,     0,   171,   171,   172,     0,     0,     0,     0,
     425,     0,     0,   959,     0,     0,     0,   761,   960,     0,
     961,   962,   963,   964,   965,   966,   967,   340,   144,     0,
       0,    75,     0,    77,     0,    78,    79,     0,    80,    81,
      82,     0,   594,   172,     0,   792,     0,     0,     0,     0,
       0,     0,   172,   172,     0,     0,     0,     0,     0,   169,
       0,   183,   968,   969,    93,   970,     0,     0,   182,     0,
       0,     0,   590,     0,     0,     0,   590,     0,   590,     0,
       0,     0,     0,   182,     0,   182,   182,     0,     0,     0,
    1075,     0,    11,     0,   182,     0,     0,     0,     0,     0,
       0,   182,   171,     0,     0,     0,     0,     0,   592,     0,
     184,     0,     0,     0,     0,   182,     0,     0,   182,   215,
     215,   592,   592,   592,     0,     0,     0,     0,    33,   594,
      35,     0,     0,     0,   594,     0,     0,     0,     0,     0,
       0,   594,     0,     0,     0,     0,     0,     0,   590,     0,
     959,   172,     0,     0,     0,   960,     0,   961,   962,   963,
     964,   965,   966,   967,    27,    28,     0,     0,   168,     0,
     171,     0,     0,    33,     0,    35,     0,     0,     0,     0,
     182,   215,     0,     0,     0,     0,     0,   215,     0,     0,
       0,     0,     0,   592,     0,     0,     0,   590,   144,   968,
     969,    75,   970,    77,     0,    78,    79,     0,    80,    81,
      82,   171,     0,   168,     0,     0,   590,   590,     0,   172,
       0,   590,   182,   171,     0,   594,   182,  1207,     0,   169,
      31,    32,   410,     0,    93,     0,     0,     0,     0,   594,
      37,   594,     0,   144,     0,     0,    75,     0,    77,     0,
      78,    79,   954,    80,    81,    82,     0,     0,   958,     0,
     172,   171,    85,     0,     0,   215,     0,   215,     0,     0,
       0,     0,   172,     0,   358,     0,     0,     0,     0,    93,
     592,     0,     0,     0,     0,   592,    66,    67,    68,    69,
      70,     0,   592,     0,     0,   215,     0,   587,   208,     0,
       0,     0,     0,    73,    74,     0,     0,     0,     0,     0,
     172,     0,     0,     0,     0,     0,     0,    84,     0,     0,
       0,     0,     0,     0,   209,     0,     0,     0,   215,   590,
       0,     0,    89,   594,     0,     0,     0,   594,     0,   594,
       0,     0,     0,     0,    33,     0,   182,  1033,     0,     0,
     590,     0,     0,     0,     0,  1041,     0,     0,     0,     0,
      33,   590,    35,     0,     0,     0,   590,     0,     0,     0,
       0,  -245,     0,     0,   590,     0,   592,     0,     0,   961,
     962,   963,   964,   965,   966,     0,     0,   215,     0,     0,
     592,     0,   592,     0,     0,     0,     0,   210,     0,     0,
     168,     0,     0,     0,     0,     0,     0,     0,     0,   594,
       0,   590,   764,     0,   144,     0,     0,    75,     0,   211,
       0,    78,    79,   590,    80,    81,    82,     0,     0,     0,
     144,     0,     0,    75,    33,    77,    35,    78,    79,   212,
      80,    81,    82,   182,     0,   213,     0,     0,   182,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   594,     0,
       0,   169,     0,     0,     0,     0,    93,     0,     0,     0,
       0,     0,     0,     0,   168,     0,     0,   594,   594,     0,
     215,     0,   594,  1219,   592,     0,     0,  1041,   592,     0,
     592,     0,     0,     0,     0,   215,     0,   215,     0,     0,
       0,     0,     0,     0,   144,     0,     0,    75,     0,    77,
       0,    78,    79,   215,    80,    81,    82,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   215,     0,     0,
     215,     0,     0,     0,     0,   169,     0,    33,     0,    35,
      93,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   147,   149,     0,   151,   152,   153,
     592,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,     0,     0,   176,   179,   180,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   196,     0,     0,
     594,     0,   215,     0,     0,     0,     0,     0,     0,   220,
       0,   222,     0,     0,     0,     0,     0,   144,     0,   592,
      75,   594,    77,     0,    78,    79,     0,    80,    81,    82,
       0,     0,   594,     0,     0,   257,     0,   594,   592,   592,
     291,   292,   293,   592,     0,   594,     0,     0,   181,   265,
       0,     0,     0,    93,     0,     0,   294,     0,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
     316,     0,   594,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   594,  -688,  -688,  -688,  -688,   308,
     309,   310,   311,   312,   313,   314,   315,     0,   316,   354,
     291,   292,   293,     0,  1033,     0,     0,     0,     0,  1327,
      33,     0,     0,     0,     0,     0,   294,   936,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   378,
     316,   592,   378,     0,     0,     0,     0,     0,   196,   387,
       0,  1034,     0,     0,     0,     0,     0,     0,  1259,     0,
       0,     0,   592,  1035,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   592,     0,     0,     0,     0,   592,     0,
     144,     0,     0,    75,     0,  1036,   592,    78,    79,     0,
      80,  1037,    82,   176,     0,     0,     0,   434,     0,     0,
     877,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   463,   316,
       0,     0,     0,   592,     0,     0,     0,     0,     0,   472,
       0,     0,     0,     0,     0,   592,     0,     0,     0,     0,
     477,   478,   479,   481,   482,   483,   484,   485,   486,   487,
     488,   489,   490,   491,   492,   493,   494,   495,   496,   497,
     498,   499,   500,   501,   502,   937,     0,   504,   504,   507,
       0,     0,     0,     0,     0,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,     0,     0,     0,
       0,     0,   504,   536,     0,   472,   504,   539,     0,     0,
       0,     0,   520,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   550,     0,   552,     5,     6,     7,     8,     9,
     472,     0,     0,     0,    10,     0,     0,     0,     0,     0,
     563,     0,     0,     0,     0,     0,     0,     0,   379,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,   602,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,     0,     0,
       0,     0,     0,    57,     0,    58,    59,    60,     0,   660,
       0,     0,     0,     0,     0,    64,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   144,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,     0,   257,    84,     0,
       0,     0,    85,     0,     0,     0,     0,     0,    86,     0,
       0,   676,     0,    89,    90,     0,    91,    92,     0,    93,
      94,     0,    95,    96,     0,     0,     0,     0,     0,     0,
       5,     6,     7,     8,     9,     0,   707,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,   196,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   517,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,   768,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   775,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,   784,     0,     0,     0,     0,     0,   142,     0,
     794,    59,    60,   795,     0,   796,     0,     0,   472,     0,
     143,    65,    66,    67,    68,    69,    70,   472,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,   144,    73,
      74,    75,   518,    77,     0,    78,    79,     0,    80,    81,
      82,   825,     0,    84,     0,     0,     0,    85,     0,     0,
       0,     0,     0,    86,     0,   291,   292,   293,    89,    90,
       0,     0,     0,     0,    93,    94,     0,    95,    96,     0,
       0,   294,     0,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,   316,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   876,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   890,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   291,
     292,   293,     0,     0,     0,     0,   472,     0,     0,     0,
       0,     0,     0,     0,   472,   294,   876,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   293,   316,
       0,     0,     0,   196,     0,     0,     0,     0,     0,     0,
       0,   935,   294,     0,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,   316,     5,     6,     7,
       8,     9,     0,     0,     0,     0,    10,     0,     0,     0,
       0,   983,   648,     0,     0,   984,     0,   985,     0,     0,
       0,   472,     0,     0,     0,     0,     0,     0,     0,     0,
     997,     0,     0,     0,     0,    11,    12,    13,   472,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,   680,     0,     0,    45,
      46,    47,    48,    49,    50,    51,     0,    52,    53,    54,
       0,     0,     0,    55,    56,    57,     0,    58,    59,    60,
      61,    62,    63,     0,     0,     0,   472,    64,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,     0,    78,    79,     0,    80,    81,    82,    83,     0,
      84,     0,     0,     0,    85,     5,     6,     7,     8,     9,
      86,    87,     0,    88,    10,    89,    90,     0,    91,    92,
     769,    93,    94,     0,    95,    96,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   472,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,    49,    50,    51,     0,    52,    53,    54,     0,     0,
       0,    55,    56,    57,     0,    58,    59,    60,    61,    62,
      63,     0,     0,     0,     0,    64,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,    83,     0,    84,     0,
       0,     0,    85,     5,     6,     7,     8,     9,    86,    87,
       0,    88,    10,    89,    90,     0,    91,    92,   878,    93,
      94,     0,    95,    96,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,    49,
      50,    51,     0,    52,    53,    54,     0,     0,     0,    55,
      56,    57,     0,    58,    59,    60,    61,    62,    63,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,    83,     0,    84,     0,     0,     0,
      85,     5,     6,     7,     8,     9,    86,    87,     0,    88,
      10,    89,    90,     0,    91,    92,     0,    93,    94,     0,
      95,    96,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,     0,     0,     0,    55,    56,    57,
       0,    58,    59,    60,     0,    62,    63,     0,     0,     0,
       0,    64,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   144,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,    83,     0,    84,     0,     0,     0,    85,     5,
       6,     7,     8,     9,    86,     0,     0,     0,    10,    89,
      90,     0,    91,    92,   457,    93,    94,     0,    95,    96,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,     0,     0,     0,    55,    56,    57,     0,    58,
      59,    60,     0,    62,    63,     0,     0,     0,     0,    64,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   144,    73,    74,
      75,    76,    77,     0,    78,    79,     0,    80,    81,    82,
      83,     0,    84,     0,     0,     0,    85,     5,     6,     7,
       8,     9,    86,     0,     0,     0,    10,    89,    90,     0,
      91,    92,   598,    93,    94,     0,    95,    96,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,   833,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
       0,     0,     0,    55,    56,    57,     0,    58,    59,    60,
       0,    62,    63,     0,     0,     0,     0,    64,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,   144,    73,    74,    75,    76,
      77,     0,    78,    79,     0,    80,    81,    82,    83,     0,
      84,     0,     0,     0,    85,     5,     6,     7,     8,     9,
      86,     0,     0,     0,    10,    89,    90,     0,    91,    92,
       0,    93,    94,     0,    95,    96,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,   932,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,     0,     0,
       0,    55,    56,    57,     0,    58,    59,    60,     0,    62,
      63,     0,     0,     0,     0,    64,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   144,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,    83,     0,    84,     0,
       0,     0,    85,     5,     6,     7,     8,     9,    86,     0,
       0,     0,    10,    89,    90,     0,    91,    92,     0,    93,
      94,     0,    95,    96,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,     0,     0,     0,    55,
      56,    57,     0,    58,    59,    60,     0,    62,    63,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   144,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,    83,     0,    84,     0,     0,     0,
      85,     5,     6,     7,     8,     9,    86,     0,     0,     0,
      10,    89,    90,     0,    91,    92,  1093,    93,    94,     0,
      95,    96,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,  1254,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,     0,     0,     0,    55,    56,    57,
       0,    58,    59,    60,     0,    62,    63,     0,     0,     0,
       0,    64,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   144,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,    83,     0,    84,     0,     0,     0,    85,     5,
       6,     7,     8,     9,    86,     0,     0,     0,    10,    89,
      90,     0,    91,    92,     0,    93,    94,     0,    95,    96,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,     0,     0,     0,    55,    56,    57,     0,    58,
      59,    60,     0,    62,    63,     0,     0,     0,     0,    64,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   144,    73,    74,
      75,    76,    77,     0,    78,    79,     0,    80,    81,    82,
      83,     0,    84,     0,     0,     0,    85,     5,     6,     7,
       8,     9,    86,     0,     0,     0,    10,    89,    90,     0,
      91,    92,  1284,    93,    94,     0,    95,    96,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
       0,     0,     0,    55,    56,    57,     0,    58,    59,    60,
       0,    62,    63,     0,     0,     0,     0,    64,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,   144,    73,    74,    75,    76,
      77,     0,    78,    79,     0,    80,    81,    82,    83,     0,
      84,     0,     0,     0,    85,     5,     6,     7,     8,     9,
      86,     0,     0,     0,    10,    89,    90,     0,    91,    92,
    1287,    93,    94,     0,    95,    96,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,  1289,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,     0,     0,
       0,    55,    56,    57,     0,    58,    59,    60,     0,    62,
      63,     0,     0,     0,     0,    64,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   144,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,    83,     0,    84,     0,
       0,     0,    85,     5,     6,     7,     8,     9,    86,     0,
       0,     0,    10,    89,    90,     0,    91,    92,     0,    93,
      94,     0,    95,    96,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,     0,     0,     0,    55,
      56,    57,     0,    58,    59,    60,     0,    62,    63,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   144,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,    83,     0,    84,     0,     0,     0,
      85,     5,     6,     7,     8,     9,    86,     0,     0,     0,
      10,    89,    90,     0,    91,    92,  1291,    93,    94,     0,
      95,    96,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,     0,     0,     0,    55,    56,    57,
       0,    58,    59,    60,     0,    62,    63,     0,     0,     0,
       0,    64,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   144,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,    83,     0,    84,     0,     0,     0,    85,     5,
       6,     7,     8,     9,    86,     0,     0,     0,    10,    89,
      90,     0,    91,    92,  1292,    93,    94,     0,    95,    96,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,     0,     0,     0,    55,    56,    57,     0,    58,
      59,    60,     0,    62,    63,     0,     0,     0,     0,    64,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   144,    73,    74,
      75,    76,    77,     0,    78,    79,     0,    80,    81,    82,
      83,     0,    84,     0,     0,     0,    85,     5,     6,     7,
       8,     9,    86,     0,     0,     0,    10,    89,    90,     0,
      91,    92,  1304,    93,    94,     0,    95,    96,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
       0,     0,     0,    55,    56,    57,     0,    58,    59,    60,
       0,    62,    63,     0,     0,     0,     0,    64,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,   144,    73,    74,    75,    76,
      77,     0,    78,    79,     0,    80,    81,    82,    83,     0,
      84,     0,     0,     0,    85,     5,     6,     7,     8,     9,
      86,     0,     0,     0,    10,    89,    90,     0,    91,    92,
    1337,    93,    94,     0,    95,    96,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,     0,     0,
       0,    55,    56,    57,     0,    58,    59,    60,     0,    62,
      63,     0,     0,     0,     0,    64,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   144,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,    83,     0,    84,     0,
       0,     0,    85,     5,     6,     7,     8,     9,    86,     0,
       0,     0,    10,    89,    90,     0,    91,    92,  1340,    93,
      94,     0,    95,    96,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,     0,     0,     0,    55,
      56,    57,     0,    58,    59,    60,     0,    62,    63,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   144,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,    83,     0,    84,     0,     0,     0,
      85,     5,     6,     7,     8,     9,    86,     0,     0,     0,
      10,    89,    90,     0,    91,    92,     0,    93,    94,     0,
      95,    96,     0,     0,   565,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,     0,     0,     0,     0,     0,    57,
       0,    58,    59,    60,     0,     0,     0,     0,     0,     0,
       0,    64,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   144,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,     0,     0,    84,     0,     0,     0,    85,     5,
       6,     7,     8,     9,    86,     0,     0,     0,    10,    89,
      90,     0,    91,    92,     0,    93,    94,     0,    95,    96,
       0,     0,   720,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,     0,     0,     0,     0,     0,    57,     0,    58,
      59,    60,     0,     0,     0,     0,     0,     0,     0,    64,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   144,    73,    74,
      75,    76,    77,     0,    78,    79,     0,    80,    81,    82,
       0,     0,    84,     0,     0,     0,    85,     5,     6,     7,
       8,     9,    86,     0,     0,     0,    10,    89,    90,     0,
      91,    92,     0,    93,    94,     0,    95,    96,     0,     0,
    1084,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
       0,     0,     0,    31,    32,    33,    34,    35,     0,    36,
       0,     0,     0,    37,    38,    39,    40,     0,    41,     0,
      42,     0,    43,     0,     0,    44,     0,     0,     0,    45,
      46,    47,    48,     0,    50,    51,     0,    52,     0,    54,
       0,     0,     0,     0,     0,    57,     0,    58,    59,    60,
       0,     0,     0,     0,     0,     0,     0,    64,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,   144,    73,    74,    75,    76,
      77,     0,    78,    79,     0,    80,    81,    82,     0,     0,
      84,     0,     0,     0,    85,     5,     6,     7,     8,     9,
      86,     0,     0,     0,    10,    89,    90,     0,    91,    92,
       0,    93,    94,     0,    95,    96,     0,     0,  1210,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,     0,     0,
       0,    31,    32,    33,    34,    35,     0,    36,     0,     0,
       0,    37,    38,    39,    40,     0,    41,     0,    42,     0,
      43,     0,     0,    44,     0,     0,     0,    45,    46,    47,
      48,     0,    50,    51,     0,    52,     0,    54,     0,     0,
       0,     0,     0,    57,     0,    58,    59,    60,     0,     0,
       0,     0,     0,     0,     0,    64,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   144,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,     0,     0,    84,     0,
       0,     0,    85,     5,     6,     7,     8,     9,    86,     0,
       0,     0,    10,    89,    90,     0,    91,    92,     0,    93,
      94,     0,    95,    96,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
       0,   316,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,     0,     0,     0,     0,
       0,    57,     0,    58,    59,    60,     0,     0,     0,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   144,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,     0,     0,    84,     0,     0,     0,
      85,     5,     6,     7,     8,     9,    86,     0,     0,     0,
      10,    89,    90,     0,    91,    92,     0,    93,    94,     0,
      95,    96,     0,     0,     0,     0,     0,     0,     0,   175,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,    48,   316,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   142,
       0,     0,    59,    60,     0,     0,     0,     0,     0,     0,
       0,   143,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   144,
      73,    74,    75,     0,    77,     0,    78,    79,     0,    80,
      81,    82,     0,     0,    84,     0,     0,     0,    85,     5,
       6,     7,     8,     9,    86,     0,     0,     0,    10,    89,
      90,     0,     0,     0,     0,    93,    94,     0,    95,    96,
    -688,  -688,  -688,  -688,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,   316,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   142,     0,     0,
      59,    60,     0,     0,     0,     0,     0,     0,     0,   143,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   144,    73,    74,
      75,     0,    77,     0,    78,    79,     0,    80,    81,    82,
       0,     0,    84,     0,     0,     0,    85,     5,     6,     7,
       8,     9,    86,     0,     0,     0,    10,    89,    90,     0,
     219,     0,     0,    93,    94,     0,    95,    96,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   256,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   142,     0,     0,    59,    60,
       0,     0,     0,     0,     0,     0,     0,   143,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,   144,    73,    74,    75,     0,
      77,     0,    78,    79,     0,    80,    81,    82,     0,     0,
      84,     0,     0,     0,    85,     5,     6,     7,     8,     9,
      86,     0,     0,     0,    10,    89,    90,     0,     0,     0,
       0,    93,    94,     0,    95,    96,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   142,     0,     0,    59,    60,     0,     0,
       0,     0,     0,     0,     0,   143,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   144,    73,    74,    75,     0,    77,     0,
      78,    79,     0,    80,    81,    82,     0,     0,    84,     0,
       0,     0,    85,     5,     6,     7,     8,     9,    86,     0,
       0,     0,    10,    89,    90,   377,     0,     0,     0,    93,
      94,     0,    95,    96,     0,     0,     0,     0,     0,     0,
       0,   469,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   142,     0,     0,    59,    60,     0,     0,     0,     0,
       0,     0,     0,   143,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   144,    73,    74,    75,     0,    77,     0,    78,    79,
       0,    80,    81,    82,     0,     0,    84,     0,     0,     0,
      85,     5,     6,     7,     8,     9,    86,     0,     0,     0,
      10,    89,    90,     0,     0,     0,     0,    93,    94,     0,
      95,    96,     0,     0,   480,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   142,
       0,     0,    59,    60,     0,     0,     0,     0,     0,     0,
       0,   143,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   144,
      73,    74,    75,     0,    77,     0,    78,    79,     0,    80,
      81,    82,     0,     0,    84,     0,     0,     0,    85,     5,
       6,     7,     8,     9,    86,     0,     0,     0,    10,    89,
      90,     0,     0,     0,     0,    93,    94,     0,    95,    96,
       0,     0,     0,     0,     0,     0,     0,   517,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   142,     0,     0,
      59,    60,     0,     0,     0,     0,     0,     0,     0,   143,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   144,    73,    74,
      75,     0,    77,     0,    78,    79,     0,    80,    81,    82,
       0,     0,    84,     0,     0,     0,    85,     5,     6,     7,
       8,     9,    86,     0,     0,     0,    10,    89,    90,     0,
       0,     0,     0,    93,    94,     0,    95,    96,     0,     0,
       0,     0,     0,     0,     0,   549,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   142,     0,     0,    59,    60,
       0,     0,     0,     0,     0,     0,     0,   143,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,   144,    73,    74,    75,     0,
      77,     0,    78,    79,     0,    80,    81,    82,     0,     0,
      84,     0,     0,     0,    85,     5,     6,     7,     8,     9,
      86,     0,     0,     0,    10,    89,    90,     0,     0,     0,
       0,    93,    94,     0,    95,    96,     0,     0,     0,     0,
       0,     0,     0,   551,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   142,     0,     0,    59,    60,     0,     0,
       0,     0,     0,     0,     0,   143,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   144,    73,    74,    75,     0,    77,     0,
      78,    79,     0,    80,    81,    82,     0,     0,    84,     0,
       0,     0,    85,     5,     6,     7,     8,     9,    86,     0,
       0,     0,    10,    89,    90,     0,     0,     0,     0,    93,
      94,     0,    95,    96,     0,     0,     0,     0,     0,     0,
       0,   783,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   142,     0,     0,    59,    60,     0,     0,     0,     0,
       0,     0,     0,   143,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   144,    73,    74,    75,     0,    77,     0,    78,    79,
       0,    80,    81,    82,     0,     0,    84,     0,     0,     0,
      85,     5,     6,     7,     8,     9,    86,     0,     0,     0,
      10,    89,    90,     0,     0,     0,     0,    93,    94,     0,
      95,    96,     0,     0,     0,     0,     0,     0,     0,   824,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   142,
       0,     0,    59,    60,     0,     0,     0,     0,     0,     0,
       0,   143,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   144,
      73,    74,    75,     0,    77,     0,    78,    79,     0,    80,
      81,    82,     0,     0,    84,     0,     0,     0,    85,     5,
       6,     7,     8,     9,    86,     0,     0,     0,    10,    89,
      90,     0,     0,     0,     0,    93,    94,     0,    95,    96,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   142,     0,     0,
      59,    60,     0,     0,     0,     0,     0,     0,     0,   143,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   144,    73,    74,
      75,   518,    77,     0,    78,    79,     0,    80,    81,    82,
       0,     0,    84,     0,     0,     0,    85,     5,     6,     7,
       8,     9,    86,     0,     0,     0,    10,    89,    90,     0,
       0,     0,     0,    93,    94,     0,    95,    96,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   142,     0,     0,    59,    60,
       0,     0,     0,     0,     0,     0,     0,   143,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,   144,    73,    74,    75,     0,
      77,     0,    78,    79,     0,    80,    81,    82,     0,     0,
      84,     0,     0,     0,    85,     5,     6,     7,     8,     9,
      86,     0,     0,     0,    10,    89,    90,     0,     0,     0,
       0,    93,    94,     0,    95,    96,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,   433,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   142,     0,     0,    59,    60,     0,     0,
       0,     0,     0,     0,     0,   143,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   144,    73,    74,    75,     0,    77,     0,
      78,    79,     0,    80,    81,    82,     0,     0,    84,     0,
       0,     0,    85,  1102,  1103,  1104,  1105,  1106,    86,  1107,
    1108,  1109,  1110,    89,    90,     0,     0,     0,     0,    93,
      94,     0,    95,    96,   294,     0,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,     0,   316,  1111,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1112,  1113,  1114,  1115,  1116,  1117,  1118,     0,
       0,    33,     0,     0,     0,     0,     0,     0,     0,     0,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
    1129,  1130,  1131,  1132,  1133,  1134,  1135,  1136,  1137,  1138,
    1139,  1140,  1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,
    1149,  1150,  1151,  1152,  1153,  1154,  1155,  1156,  1157,  1158,
    1159,     0,     0,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1167,  1168,  1169,     0,  1170,     0,     0,    78,    79,
       0,    80,    81,    82,  1171,  1172,  1173,     0,     0,  1174,
     291,   292,   293,     0,     0,     0,  1175,  1176,     0,  1177,
       0,  1178,     0,     0,     0,     0,   294,     0,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
     316,   291,   292,   293,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   294,     0,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
       0,   316,   291,   292,   293,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   294,     0,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,     0,   316,   291,   292,   293,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   294,
       0,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,     0,   316,   291,   292,   293,   683,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     294,     0,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,     0,   316,   291,   292,   293,   752,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   294,     0,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,   316,   291,   292,   293,   804,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   294,     0,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,   316,   291,   292,   293,
     826,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   294,     0,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,   316,   291,   292,
     293,   986,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   294,     0,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,     0,   316,   291,
     292,   293,  1061,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   294,     0,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,   316,
     291,   292,   293,  1062,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   294,     0,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
     316,     0,   317,     0,   291,   292,   293,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     294,     0,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,   403,   316,   583,   584,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   585,     0,     0,   291,   292,   293,
       0,     0,    31,    32,    33,     0,     0,     0,     0,     0,
       0,     0,    37,   294,   405,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,   316,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   541,     0,     0,   586,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,   587,
       0,     0,     0,     0,   144,    73,    74,    75,     0,   588,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,     0,     0,   291,   292,   293,     0,     0,   560,   589,
       0,     0,     0,     0,    89,     0,     0,     0,     0,   294,
     363,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,     0,   316,   291,   292,   293,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     294,     0,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,     0,   316,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   291,
     292,   293,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   294,   664,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,   316,
     291,   292,   293,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   294,   702,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
     316,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,     0,   316,     0,     0,   291,   292,   293,     0,
       0,     0,   901,     0,     0,     0,     0,     0,     0,     0,
       0,   569,   294,   661,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,   316,   291,   292,   293,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   294,     0,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,   316,   292,   293,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   294,     0,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,   316,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,   316,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
     316
};

static const yytype_int16 yycheck[] =
{
       4,    83,   351,   753,   146,    87,    88,    30,   326,   281,
       4,     4,     4,     4,     4,   182,   788,    40,   346,   221,
     131,    44,   138,   542,   188,   208,   209,   188,   110,   638,
     213,    53,   956,   656,    24,    25,     4,   975,   978,   316,
     568,   791,    46,   371,   978,    49,    28,    90,   278,     8,
      72,     8,     8,    75,     8,   772,     8,   401,     8,    63,
       8,   440,     8,     8,    26,     8,     8,    73,     8,     8,
     414,   995,    29,    69,    73,    61,     8,    61,   189,    83,
      46,   357,     8,    87,    88,     8,     8,   102,    73,    57,
       8,    78,   234,     8,     8,   278,     8,     8,     8,    78,
      90,   659,   121,   121,    73,   401,   110,    90,     0,    78,
       8,     8,   121,   671,   121,   144,   174,   175,   414,  1057,
     147,    26,    69,  1229,  1230,   319,   169,    90,    66,    67,
     753,   162,   172,   574,    13,   172,   745,   121,   181,   138,
     139,   172,    66,    67,   159,   174,   175,    26,   175,   343,
     137,   155,  1076,   347,   173,   173,   906,    61,   137,    41,
     128,   144,    61,   401,    43,    41,   173,   176,   791,   138,
     139,   177,    61,   449,   142,   172,   414,   379,   174,   169,
     146,   144,   324,   900,    61,   175,   172,   904,   905,   175,
     194,   181,   177,   197,    61,   161,    61,   201,    61,    61,
     194,   223,   284,   197,   194,   321,    61,   197,    61,    61,
     819,    61,   174,   396,   218,    61,   173,   176,   174,   175,
     174,    61,   174,   173,   340,   173,   570,   174,   174,   174,
     252,   174,   174,   509,   174,   174,   140,   163,   228,   583,
     584,   323,   174,   325,   802,   361,   236,   237,   238,   872,
     629,   174,   174,   243,   370,   173,   879,   373,   173,   249,
     174,   173,   173,   173,   268,   163,   163,   616,   451,   452,
     175,   175,   895,   277,   570,   157,   175,   281,   268,   172,
     284,   157,   723,   906,   725,  1002,   175,   583,   584,   585,
     454,   173,    61,   454,   172,   172,   117,   173,   175,    73,
     268,   820,    61,   579,    78,   175,   834,   117,   175,    73,
     175,   655,   175,   175,    78,   358,   172,   321,   322,   323,
     175,   325,   175,   175,    73,   175,   146,    46,    31,   175,
      31,    71,   570,    73,   316,   175,   340,   172,   947,   321,
      31,   669,    31,   510,   172,   583,   584,   585,    93,    94,
     678,   581,   121,    73,  1294,    93,    94,   361,   340,   655,
    1294,   351,   121,   565,   138,   139,   533,   172,   358,   373,
      73,  1309,    73,   996,   138,   139,   999,  1000,    98,   361,
     384,   548,    73,   547,    73,   407,   547,   172,   370,   553,
     384,   373,   553,   912,   384,   914,  1320,   674,   581,   582,
     404,  1010,  1326,   747,   172,   409,   175,   181,   148,   149,
     754,   151,   152,   153,   404,    73,   175,   655,    71,   423,
      78,   697,   140,   590,   172,   429,   430,   146,   177,  1225,
    1226,   176,   175,   515,    26,   175,   404,   177,   176,   172,
     140,    73,   553,   147,   162,   144,   742,    73,   766,   172,
      25,   747,    78,    71,  1077,    31,   160,   175,   754,   174,
     643,   174,   162,   174,    61,   741,   649,    42,    61,   797,
      45,   175,   172,  1245,    61,   175,   140,   805,   175,   469,
     138,   139,   106,   107,   108,   109,   110,   111,   141,  1008,
     172,   144,   172,    61,   838,   148,   149,   115,   151,   152,
     153,    92,    93,    94,   742,   137,   138,   139,   852,   747,
     854,   515,   138,   139,    98,   172,   754,   144,   720,    92,
      93,    94,   180,   176,    40,   874,    49,   517,   644,   162,
     148,   149,    25,   151,   152,   153,   140,   179,   542,   815,
     544,   564,   838,   121,   727,     8,   729,   172,  1298,    42,
     174,    71,    45,    73,   172,   140,   852,   561,   854,   549,
    1079,   551,  1312,   140,   892,   121,   140,   571,   572,   174,
     172,   561,    25,   174,   757,    94,    95,    96,   572,   569,
     174,   909,   572,   859,    13,    69,    70,    71,   864,    13,
     162,    13,    98,   561,   173,   599,   162,    79,    51,    13,
     838,    90,   946,   109,   110,   174,   950,   790,   952,   173,
     173,   117,   118,   780,   852,   172,   854,   607,    71,    45,
      46,    47,   172,    49,   628,   178,   616,   617,   148,   149,
     172,   151,   152,   153,   628,   628,   628,   628,   628,     8,
     644,   173,   173,    82,   920,    98,     8,   174,   154,    13,
     946,    71,    79,    73,   950,   175,   952,   177,   172,   987,
     628,     8,   644,   172,   148,   149,   849,   151,   152,   153,
      73,   124,   119,   174,   172,  1298,   173,    61,  1022,   120,
     161,   685,   686,   687,   122,     8,   709,   959,   141,  1312,
    1209,   144,   674,   146,   173,   148,   149,    13,   151,   152,
     153,    61,   173,     8,    13,   179,   710,   119,   946,   713,
      31,   179,   950,   166,   952,   176,   710,     8,   172,   172,
     710,   172,   924,   713,   179,   715,  1022,  1071,   148,   149,
     173,   151,   152,   153,   179,  1063,   938,   173,     8,   174,
     744,   174,   746,   122,   246,   713,  1090,  1091,   250,   172,
      71,  1095,    73,   955,   758,   175,   173,   177,   140,   942,
     140,    71,   172,    73,   175,   102,   770,    13,   270,   174,
     272,   273,   274,   275,   957,  1071,   770,   770,   770,   770,
     770,   175,  1054,    90,  1022,    13,   179,   175,    13,   174,
     111,   172,   975,   783,  1090,  1091,    73,   172,   172,  1095,
      13,    78,   770,  1099,   827,   172,   989,    41,    71,   992,
    1012,   172,   816,     8,   173,    13,   820,  1019,   174,   174,
     141,    71,   122,   144,     8,   146,   830,   148,   149,   173,
     151,   152,   153,  1071,   824,   839,   830,   173,   148,   149,
     830,   151,   152,   153,   122,   179,   836,     8,   137,   839,
     172,   172,  1090,  1091,     8,   159,   177,  1095,   172,  1203,
     137,   138,   139,   122,    41,    99,   174,   177,     8,    71,
     104,   839,   106,   107,   108,   109,   110,   111,   112,    71,
    1224,   144,  1084,   173,   874,   148,   149,   172,   151,   152,
     153,  1235,   175,   173,   144,   122,  1240,   173,   148,   149,
     175,   151,   152,   153,  1248,   172,   175,  1203,   912,  1051,
     914,    73,   175,   179,   148,   149,    78,   151,   172,     8,
      24,    25,    99,   137,    28,    26,    68,   104,  1224,   106,
     107,   108,   109,   110,   111,   112,   174,   173,    26,  1235,
     163,  1285,   176,   174,  1240,    49,   148,   149,   122,   151,
     152,   153,  1248,  1297,   146,   959,   148,   149,   150,   151,
     152,   153,   173,     8,   173,  1203,   970,  1263,   176,   122,
     172,   148,   149,     8,   151,   137,   138,   139,   175,   173,
     172,    90,   172,   176,    26,   173,  1224,   173,   175,  1285,
     173,   173,    42,    43,   122,   173,     8,  1235,  1200,   176,
      26,  1297,  1240,  1205,  1008,   174,   974,   173,  1210,  1013,
    1248,    61,  1016,    73,  1216,  1217,  1020,   174,   174,    69,
      70,    71,  1016,  1013,  1028,   122,  1016,   150,   146,    79,
    1020,   172,   104,    73,   173,    13,  1040,   173,  1028,   173,
      71,   122,  1046,   173,   122,  1013,   122,  1285,   173,  1251,
    1054,    13,  1020,   175,  1196,   174,    73,   173,   172,  1297,
    1028,    13,   175,   175,   122,   169,    13,   175,   175,    13,
     173,   175,    73,    13,   124,  1079,   175,   181,  1082,  1083,
      51,    73,   172,  1087,    73,     8,   136,    13,  1056,  1083,
     174,    90,  1082,  1083,   140,    90,   153,  1087,   148,   149,
      29,   151,   152,   153,   208,   209,    71,    13,   172,   213,
     141,  1193,   173,   144,  1082,   146,   166,   148,   149,  1087,
     151,   152,   153,   174,   228,    73,   155,   174,  1330,   173,
     173,   172,   236,   237,   238,    73,  1338,   173,   175,   243,
    1306,   128,   407,   322,   325,   249,   670,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,  1241,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,   323,   673,   278,   836,  1317,   281,   630,   935,
     412,    24,    25,   148,   149,    28,   151,   152,   153,  1193,
     970,  1100,  1332,    59,    60,    42,    43,    44,    45,    46,
      47,  1182,    49,    38,    41,  1209,    59,    60,    90,  1213,
    1029,   773,   316,   803,  1194,   237,  1220,   742,   816,   686,
     895,  1225,  1226,  1213,   699,  1229,  1230,   730,   244,   992,
      24,    25,    -1,    -1,    28,    25,    -1,  1241,    -1,    -1,
      -1,    71,  1246,    73,    74,  1213,  1250,   351,    -1,  1253,
      -1,  1255,  1256,    -1,   358,   121,  1246,    -1,    -1,    -1,
    1250,    51,    99,  1253,    -1,  1255,  1256,   104,   121,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,  1246,    -1,
      -1,    71,  1250,    -1,  1288,  1253,    -1,  1255,  1256,  1293,
      -1,    25,   396,    -1,    -1,    -1,    -1,   401,  1288,    -1,
      -1,    -1,    -1,    -1,  1308,    -1,    -1,   173,    -1,    -1,
     414,   148,   149,    -1,   151,    -1,    -1,    51,   148,   149,
    1288,   151,   152,   153,    -1,    -1,   169,    -1,    -1,    -1,
    1334,    -1,   175,    -1,   124,  1339,    -1,    71,   181,   176,
      -1,    -1,    -1,    -1,  1334,    -1,    -1,   451,   452,  1339,
      -1,   141,    -1,    -1,   144,    -1,   146,    49,   148,   149,
      -1,   151,   152,   153,    -1,   469,  1334,   157,    -1,    -1,
      -1,  1339,    -1,    -1,    -1,   169,   166,    -1,    -1,    -1,
      -1,   175,   172,    -1,    -1,   228,    -1,   181,    -1,    -1,
     124,    -1,    -1,   236,   237,   238,    -1,    -1,    -1,    -1,
     243,    -1,    -1,    -1,    -1,    -1,   249,   141,    -1,    -1,
     144,    -1,   146,   517,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,   157,    41,   106,   107,   108,   109,   110,
     111,    -1,   166,    -1,   228,    -1,    -1,    -1,   172,    -1,
      -1,    -1,   236,   237,   238,   549,    -1,   551,    -1,   243,
      -1,    -1,    -1,    -1,    -1,   249,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   569,   570,    -1,    -1,    -1,
      -1,    -1,    -1,   316,    -1,    -1,    -1,   581,   582,   583,
     584,   585,    99,    -1,    -1,    71,    -1,   104,    -1,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   607,    -1,    -1,    -1,    -1,   351,    -1,
     614,    -1,   616,   617,    -1,   358,   208,   209,    -1,    -1,
      -1,   213,   316,    -1,    -1,    -1,   630,    -1,    -1,    -1,
      -1,   148,   149,    -1,   151,    25,    -1,    -1,    -1,   643,
      -1,    -1,    -1,    -1,    -1,   649,    -1,    -1,    -1,    -1,
      -1,   655,    -1,    -1,   658,    -1,    -1,   351,   401,   176,
     146,    51,   148,   149,   358,   151,   152,   153,    -1,    -1,
     674,   414,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    -1,    -1,    -1,    -1,   278,    -1,    25,   281,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,   715,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   727,    -1,   729,   469,    -1,    -1,    63,
      -1,    -1,    -1,    -1,   124,    -1,    -1,    71,   742,    73,
      -1,    -1,    -1,   747,    -1,    -1,    -1,    -1,    -1,    -1,
     754,   141,    -1,   757,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   517,   469,   166,   111,    -1,   783,
      -1,    -1,   172,    -1,    -1,    -1,   790,    -1,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    -1,    -1,    -1,   396,    -1,   549,   141,   551,   401,
     144,    -1,   146,    -1,   148,   149,    -1,   151,   152,   153,
     824,    -1,   414,   517,    -1,    41,   569,   570,    71,    -1,
      73,    -1,   836,    -1,   838,    59,    60,    -1,   172,   176,
     583,   584,   585,   177,    -1,   849,    -1,    -1,   852,    -1,
     854,    -1,    -1,    -1,    -1,   549,    -1,   551,    -1,   451,
     452,   865,    -1,    -1,   607,    -1,    -1,   871,   111,    -1,
     874,   614,    -1,   616,   617,   569,    -1,    -1,    -1,    -1,
     123,    -1,    -1,    99,    -1,    -1,    -1,   630,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,   121,   141,    -1,
      -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,   655,   607,    -1,   658,    -1,    -1,    -1,    -1,
      -1,    -1,   616,   617,    -1,    -1,    -1,    -1,    -1,   172,
      -1,   674,   148,   149,   177,   151,    -1,    -1,   942,    -1,
      -1,    -1,   946,    -1,    -1,    -1,   950,    -1,   952,    -1,
      -1,    -1,    -1,   957,    -1,   959,   960,    -1,    -1,    -1,
     176,    -1,    41,    -1,   968,    -1,    -1,    -1,    -1,    -1,
      -1,   975,   715,    -1,    -1,    -1,    -1,    -1,   570,    -1,
     674,    -1,    -1,    -1,    -1,   989,    -1,    -1,   992,   581,
     582,   583,   584,   585,    -1,    -1,    -1,    -1,    71,   742,
      73,    -1,    -1,    -1,   747,    -1,    -1,    -1,    -1,    -1,
      -1,   754,    -1,    -1,    -1,    -1,    -1,    -1,  1022,    -1,
      99,   715,    -1,    -1,    -1,   104,    -1,   106,   107,   108,
     109,   110,   111,   112,    62,    63,    -1,    -1,   111,    -1,
     783,    -1,    -1,    71,    -1,    73,    -1,    -1,    -1,    -1,
    1054,   643,    -1,    -1,    -1,    -1,    -1,   649,    -1,    -1,
      -1,    -1,    -1,   655,    -1,    -1,    -1,  1071,   141,   148,
     149,   144,   151,   146,    -1,   148,   149,    -1,   151,   152,
     153,   824,    -1,   111,    -1,    -1,  1090,  1091,    -1,   783,
      -1,  1095,  1096,   836,    -1,   838,  1100,   176,    -1,   172,
      69,    70,   175,    -1,   177,    -1,    -1,    -1,    -1,   852,
      79,   854,    -1,   141,    -1,    -1,   144,    -1,   146,    -1,
     148,   149,   865,   151,   152,   153,    -1,    -1,   871,    -1,
     824,   874,   160,    -1,    -1,   727,    -1,   729,    -1,    -1,
      -1,    -1,   836,    -1,   172,    -1,    -1,    -1,    -1,   177,
     742,    -1,    -1,    -1,    -1,   747,   125,   126,   127,   128,
     129,    -1,   754,    -1,    -1,   757,    -1,   136,    25,    -1,
      -1,    -1,    -1,   142,   143,    -1,    -1,    -1,    -1,    -1,
     874,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,    -1,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,   790,  1203,
      -1,    -1,   171,   946,    -1,    -1,    -1,   950,    -1,   952,
      -1,    -1,    -1,    -1,    71,    -1,  1220,   960,    -1,    -1,
    1224,    -1,    -1,    -1,    -1,   968,    -1,    -1,    -1,    -1,
      71,  1235,    73,    -1,    -1,    -1,  1240,    -1,    -1,    -1,
      -1,    98,    -1,    -1,  1248,    -1,   838,    -1,    -1,   106,
     107,   108,   109,   110,   111,    -1,    -1,   849,    -1,    -1,
     852,    -1,   854,    -1,    -1,    -1,    -1,   124,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1022,
      -1,  1285,   123,    -1,   141,    -1,    -1,   144,    -1,   146,
      -1,   148,   149,  1297,   151,   152,   153,    -1,    -1,    -1,
     141,    -1,    -1,   144,    71,   146,    73,   148,   149,   166,
     151,   152,   153,  1317,    -1,   172,    -1,    -1,  1322,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1071,    -1,
      -1,   172,    -1,    -1,    -1,    -1,   177,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,  1090,  1091,    -1,
     942,    -1,  1095,  1096,   946,    -1,    -1,  1100,   950,    -1,
     952,    -1,    -1,    -1,    -1,   957,    -1,   959,    -1,    -1,
      -1,    -1,    -1,    -1,   141,    -1,    -1,   144,    -1,   146,
      -1,   148,   149,   975,   151,   152,   153,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   989,    -1,    -1,
     992,    -1,    -1,    -1,    -1,   172,    -1,    71,    -1,    73,
     177,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     5,     6,    -1,     8,     9,    10,
    1022,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,   111,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,
    1203,    -1,  1054,    -1,    -1,    -1,    -1,    -1,    -1,    50,
      -1,    52,    -1,    -1,    -1,    -1,    -1,   141,    -1,  1071,
     144,  1224,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,  1235,    -1,    -1,    76,    -1,  1240,  1090,  1091,
       9,    10,    11,  1095,    -1,  1248,    -1,    -1,   172,    90,
      -1,    -1,    -1,   177,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,  1285,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1297,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,   150,
       9,    10,    11,    -1,  1317,    -1,    -1,    -1,    -1,  1322,
      71,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,   190,
      49,  1203,   193,    -1,    -1,    -1,    -1,    -1,   199,   200,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,  1220,    -1,
      -1,    -1,  1224,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1235,    -1,    -1,    -1,    -1,  1240,    -1,
     141,    -1,    -1,   144,    -1,   146,  1248,   148,   149,    -1,
     151,   152,   153,   244,    -1,    -1,    -1,   248,    -1,    -1,
     179,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   269,    49,
      -1,    -1,    -1,  1285,    -1,    -1,    -1,    -1,    -1,   280,
      -1,    -1,    -1,    -1,    -1,  1297,    -1,    -1,    -1,    -1,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,   174,    -1,   318,   319,   320,
      -1,    -1,    -1,    -1,    -1,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   337,    -1,    -1,    -1,
      -1,    -1,   343,   344,    -1,   346,   347,   348,    -1,    -1,
      -1,    -1,   353,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   363,    -1,   365,     3,     4,     5,     6,     7,
     371,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
     381,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,   410,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,    -1,    -1,   111,    -1,   113,   114,   115,    -1,   480,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,   518,   156,    -1,
      -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,   532,    -1,   171,   172,    -1,   174,   175,    -1,   177,
     178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,   557,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   568,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,   625,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   637,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,   653,    -1,    -1,    -1,    -1,    -1,   111,    -1,
     661,   114,   115,   664,    -1,   666,    -1,    -1,   669,    -1,
     123,   124,   125,   126,   127,   128,   129,   678,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   702,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,     9,    10,    11,   171,   172,
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   766,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   779,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,   797,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   805,    25,   807,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    11,    49,
      -1,    -1,    -1,   834,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   842,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      -1,   882,   176,    -1,    -1,   886,    -1,   888,    -1,    -1,
      -1,   892,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     901,    -1,    -1,    -1,    -1,    41,    42,    43,   909,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,   176,    -1,    -1,    95,
      96,    97,    98,    99,   100,   101,    -1,   103,   104,   105,
      -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,   115,
     116,   117,   118,    -1,    -1,    -1,   987,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,   167,    -1,   169,    12,   171,   172,    -1,   174,   175,
     176,   177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1063,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,   167,
      -1,   169,    12,   171,   172,    -1,   174,   175,   176,   177,
     178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,   113,   114,   115,   116,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,   167,    -1,   169,
      12,   171,   172,    -1,   174,   175,    -1,   177,   178,    -1,
     180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,    -1,   174,   175,   176,   177,   178,    -1,   180,   181,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,
     174,   175,   176,   177,   178,    -1,   180,   181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    83,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
      -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,
      -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    89,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,   177,
     178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,    -1,   174,   175,   176,   177,   178,    -1,
     180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    87,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,    -1,   174,   175,    -1,   177,   178,    -1,   180,   181,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,
     174,   175,   176,   177,   178,    -1,   180,   181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
      -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,
     176,   177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    85,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,   177,
     178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,    -1,   174,   175,   176,   177,   178,    -1,
     180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,    -1,   174,   175,   176,   177,   178,    -1,   180,   181,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,
     174,   175,   176,   177,   178,    -1,   180,   181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
      -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,   115,
      -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,   154,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,
     176,   177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,    -1,   174,   175,   176,   177,
     178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,    -1,   174,   175,    -1,   177,   178,    -1,
     180,   181,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,   111,
      -1,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,    -1,   174,   175,    -1,   177,   178,    -1,   180,   181,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,    -1,    -1,    -1,    -1,    -1,   111,    -1,   113,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,
     174,   175,    -1,   177,   178,    -1,   180,   181,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
      -1,    -1,    -1,    -1,    -1,   111,    -1,   113,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,
      -1,   177,   178,    -1,   180,   181,    -1,    -1,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,    -1,    -1,   111,    -1,   113,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,   177,
     178,    -1,   180,   181,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,    -1,
      -1,   111,    -1,   113,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,    -1,   174,   175,    -1,   177,   178,    -1,
     180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    98,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,    -1,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,
     174,    -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,
      -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,   173,    -1,    -1,    -1,   177,
     178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,
     180,   181,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,    -1,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,
      -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,    -1,
      -1,    -1,    12,   171,   172,    -1,    -1,    -1,    -1,   177,
     178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,    -1,    -1,
      12,   171,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,
     180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,
       4,     5,     6,     7,   166,    -1,    -1,    -1,    12,   171,
     172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,
      -1,    -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    -1,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,
      -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     156,    -1,    -1,    -1,   160,     3,     4,     5,     6,     7,
     166,    -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,
      -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,     3,     4,     5,     6,     7,   166,     9,
      10,    11,    12,   171,   172,    -1,    -1,    -1,    -1,   177,
     178,    -1,   180,   181,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    -1,
      -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,    -1,   145,    -1,    -1,   148,   149,
      -1,   151,   152,   153,   154,   155,   156,    -1,    -1,   159,
       9,    10,    11,    -1,    -1,    -1,   166,   167,    -1,   169,
      -1,   171,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
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
      46,    47,    -1,    49,     9,    10,    11,   176,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,   176,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,   176,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,   176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,   176,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,   174,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,   174,    49,    42,    43,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    61,    -1,    -1,     9,    10,    11,
      -1,    -1,    69,    70,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    79,    25,   174,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   173,    -1,    -1,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,   173,   166,
      -1,    -1,    -1,    -1,   171,    -1,    -1,    -1,    -1,    25,
     122,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   122,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   122,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    90,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   183,   184,     0,   185,     3,     4,     5,     6,     7,
      12,    41,    42,    43,    48,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    69,    70,    71,    72,    73,    75,    79,    80,    81,
      82,    84,    86,    88,    91,    95,    96,    97,    98,    99,
     100,   101,   103,   104,   105,   109,   110,   111,   113,   114,
     115,   116,   117,   118,   123,   124,   125,   126,   127,   128,
     129,   136,   141,   142,   143,   144,   145,   146,   148,   149,
     151,   152,   153,   154,   156,   160,   166,   167,   169,   171,
     172,   174,   175,   177,   178,   180,   181,   186,   189,   192,
     193,   194,   195,   196,   197,   200,   211,   212,   215,   220,
     226,   282,   286,   287,   288,   289,   290,   298,   299,   300,
     302,   303,   306,   316,   317,   318,   323,   326,   344,   349,
     351,   352,   353,   354,   355,   356,   357,   358,   360,   373,
     375,   377,   111,   123,   141,   189,   211,   289,   351,   289,
     172,   289,   289,   289,   342,   343,   289,   289,   289,   289,
     289,   289,   289,   289,   289,   289,   289,   289,   111,   172,
     193,   317,   318,   351,   351,    31,   289,   364,   365,   289,
     111,   172,   193,   317,   318,   319,   350,   356,   361,   362,
     172,   283,   320,   172,   283,   284,   289,   202,   283,   172,
     172,   172,   283,    69,   174,   189,    69,   174,    25,    51,
     124,   146,   166,   172,   189,   196,   378,   388,   389,   174,
     289,   175,   289,   144,   190,   191,   192,    73,   177,   250,
     251,   117,   117,    73,   211,   252,   172,   172,   172,   172,
     189,   224,   379,   172,   172,    73,    78,   137,   138,   139,
     370,   371,   144,   175,   192,   192,    95,   289,   225,   379,
     146,   172,   379,   379,   282,   289,   290,   351,   198,   175,
      78,   321,   370,    78,   370,   370,    26,   144,   162,   380,
     172,     8,   174,    31,   210,   146,   223,   379,   174,   174,
     174,     9,    10,    11,    25,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    49,   174,    61,    61,
     175,   140,   118,   154,   211,   226,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    59,    60,
     121,   346,   347,    61,   175,   348,   172,    61,   175,   177,
     357,   172,   210,    13,   289,    40,   189,   341,   172,   282,
     351,   140,   351,   122,   179,     8,   328,   282,   351,   380,
     140,   172,   322,   121,   346,   347,   348,   173,   289,    26,
     200,     8,   174,   200,   201,   284,   285,   289,   189,   238,
     204,   174,   174,   174,   389,   389,   162,   172,    98,   381,
     389,    13,   189,   174,   198,   174,   192,     8,   174,    90,
     175,   351,     8,   174,    13,   210,     8,   174,   351,   374,
     374,   351,   173,   162,   218,   123,   351,   363,   364,    61,
     121,   137,   371,    72,   289,   351,    78,   137,   371,   192,
     188,   174,   175,   174,   221,   307,   309,    79,   293,   294,
     296,    13,    90,   376,   173,   173,   173,   176,   199,   200,
     212,   215,   220,   289,   178,   180,   181,   189,   381,    31,
     248,   249,   289,   378,   172,   379,   216,   289,   289,   289,
      26,   289,   289,   289,   289,   289,   289,   289,   289,   289,
     289,   289,   289,   289,   289,   289,   289,   289,   289,   289,
     289,   289,   289,   319,   289,   359,   359,   289,   366,   367,
     189,   356,   357,   224,   225,   210,   223,    31,   145,   286,
     289,   289,   289,   289,   289,   289,   289,   289,   289,   289,
     289,   289,   175,   189,   356,   359,   289,   248,   359,   289,
     363,   173,   172,   340,     8,   328,   282,   173,   189,    31,
     289,    31,   289,   173,   173,   356,   248,   175,   189,   356,
     173,   198,   242,   289,    82,    26,   200,   236,   174,    90,
      13,     8,   173,    26,   175,   239,   389,    79,   385,   386,
     387,   172,     8,    42,    43,    61,   124,   136,   146,   166,
     193,   194,   196,   301,   317,   323,   324,   325,   176,    90,
     191,   189,   289,   251,   324,   172,    73,     8,   173,   173,
     173,   174,   189,   384,   119,   229,   172,     8,   173,   173,
      73,    74,   189,   372,   189,    61,   176,   176,   185,   187,
     120,   228,   161,    46,   146,   161,   311,   122,     8,   328,
     173,   389,   389,    13,   121,   346,   347,   348,   176,     8,
     163,   351,   173,     8,   329,    13,   291,   213,   119,   227,
     289,    26,   179,   179,   122,   176,     8,   328,   380,   172,
     219,   222,   379,   217,    63,   351,   289,   380,   172,   179,
     176,   173,   179,   176,   173,    42,    43,    61,    69,    70,
      79,   124,   136,   166,   189,   331,   333,   336,   339,   189,
     351,   351,   122,   346,   347,   348,   173,   289,   243,    66,
      67,   244,   283,   198,   285,    31,   233,   351,   324,   189,
      26,   200,   237,   174,   240,   174,   240,     8,   163,   122,
       8,   328,   173,   157,   381,   382,   389,   324,   324,   324,
     327,   330,   172,    78,   140,   172,   140,   175,   102,   207,
     208,   189,   176,   292,    13,   351,   174,    90,     8,   163,
     230,   317,   175,   363,   123,   351,    13,   179,   289,   176,
     185,   230,   175,   310,    13,   289,   293,   174,   389,   175,
     189,   356,   389,    31,   289,   324,   157,   246,   247,   344,
     345,   172,   317,   228,   289,   289,   289,   172,   248,   229,
     228,   214,   227,   319,   176,   172,   248,    13,    69,    70,
     189,   332,   332,   333,   334,   335,   172,    78,   137,   172,
       8,   328,   173,   340,    31,   289,   176,    66,    67,   245,
     283,   200,   174,    83,   174,   351,   122,   232,    13,   198,
     240,    92,    93,    94,   240,   176,   389,   389,   385,     8,
     173,   173,   122,   179,     8,   328,   327,   189,   293,   295,
     297,   189,   324,   368,   369,   172,   159,   246,   324,   389,
     189,     8,   253,   173,   172,   286,   289,   179,   176,   253,
     147,   160,   175,   306,   313,   147,   175,   312,   122,   174,
     289,   380,   172,   351,   173,     8,   329,   389,   390,   246,
     175,   122,   248,   173,   175,   175,   172,   228,   322,   172,
     248,   173,   122,   179,     8,   328,   334,   137,   293,   337,
     338,   333,   351,   283,    26,    68,   200,   174,   285,   233,
     173,   324,    89,    92,   174,   289,    26,   174,   241,   176,
     163,   157,    26,   324,   324,   173,   122,     8,   328,   173,
     122,   176,     8,   328,   317,   175,   173,    90,   317,    99,
     104,   106,   107,   108,   109,   110,   111,   112,   148,   149,
     151,   176,   254,   276,   277,   278,   279,   281,   344,   363,
     176,   176,    46,   289,   289,   289,   176,   172,   248,    26,
     383,   157,   345,    31,    73,   173,   253,   289,   173,   253,
     253,   246,   175,   248,   173,   333,   333,   173,   122,   173,
       8,   328,    26,   198,   174,   173,   205,   174,   174,   241,
     198,   389,   122,   324,   293,   324,   324,    73,   198,   383,
     389,   378,   231,   317,   112,   124,   146,   152,   263,   264,
     265,   317,   150,   269,   270,   115,   172,   189,   271,   272,
     255,   211,   279,   389,     8,   174,   277,   278,   173,   146,
     308,   176,   176,   172,   248,   173,   389,   104,   304,   390,
      73,    13,   383,   176,   176,   176,   173,   253,   173,   122,
     333,   293,   198,   203,    26,   200,   235,   198,   173,   324,
     122,   122,   173,   176,   304,    13,     8,   174,   175,   175,
       8,   174,     3,     4,     5,     6,     7,     9,    10,    11,
      12,    49,    62,    63,    64,    65,    66,    67,    68,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     123,   124,   125,   126,   127,   128,   129,   141,   142,   143,
     145,   154,   155,   156,   159,   166,   167,   169,   171,   189,
     314,   315,     8,   174,   146,   150,   189,   272,   273,   274,
     174,    73,   280,   210,   256,   378,   211,   248,   173,   172,
     175,    31,    73,    13,   324,   175,   383,   176,   333,   122,
      26,   200,   234,   198,   324,   324,   175,   175,   324,   317,
     259,   266,   323,   264,    13,    26,    43,   267,   270,     8,
      29,   173,    25,    42,    45,    13,     8,   174,   379,   280,
      13,   210,   173,    31,    73,   305,   198,    73,    13,   324,
     198,   175,   333,   198,    87,   198,   198,   176,   189,   196,
     260,   261,   262,     8,   176,   324,   315,   315,    51,   268,
     273,   273,    25,    42,    45,   324,    73,   172,   174,   324,
     379,    73,     8,   329,   176,    13,   324,   176,   198,    85,
     174,   176,   176,   140,    90,   323,   153,    13,   257,   172,
      31,    73,   173,   324,   176,   174,   206,   189,   277,   278,
     324,   246,   258,    73,   102,   207,   209,   155,   189,   174,
     173,   246,   172,   231,   174,   383,   173,   317,   174,   174,
     175,   275,   383,    73,   198,   275,   173,   176,   175,   198,
     176
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
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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
#line 804 "hphp.y"
    { _p->initParseTree(); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 804 "hphp.y"
    { _p->popLabelInfo();
                                                  _p->finiParseTree();;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 810 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 811 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 816 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 817 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 819 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 850 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 852 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 863 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 864 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 867 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 881 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 898 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 917 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 924 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 931 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->onBreak((yyval), NULL);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->onBreak((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->onContinue((yyval), NULL);;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 936 "hphp.y"
    { _p->onContinue((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 964 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 965 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 969 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 972 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 985 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 986 "hphp.y"
    { (yyval).reset();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 990 "hphp.y"
    { finally_statement(_p);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 992 "hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (5)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { (yyval).reset();;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { (yyval).reset();;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1006 "hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(8) - (11)]),(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1020 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1025 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(9) - (12)]),(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1032 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
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
                                         _p->popTypeScope();;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 1050 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1053 "hphp.y"
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
                                         _p->popTypeScope();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1067 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1070 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1075 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,t_imp,
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,t_imp,
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1107 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1108 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1112 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1115 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1120 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1124 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { (yyval).reset();;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval).reset();;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1132 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyval).reset();;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { (yyval).reset();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1156 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1163 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1184 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1185 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1186 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1191 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval).reset();;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval).reset();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1198 "hphp.y"
    { (yyval).reset();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyval).reset();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1210 "hphp.y"
    { (yyval).reset();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyval).reset();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1217 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { (yyval).reset();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset(); ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { (yyval).reset();;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,NULL,&(yyvsp[(1) - (3)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,NULL,&(yyvsp[(1) - (4)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,&(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1240 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,&(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,NULL,&(yyvsp[(3) - (5)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,NULL,&(yyvsp[(3) - (6)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1250 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,&(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,&(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1260 "hphp.y"
    { (yyval).reset();;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1264 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1266 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1276 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1278 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1292 "hphp.y"
    { (yyval).reset();;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1299 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1359 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1375 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1382 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1390 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1394 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1406 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1413 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1418 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1423 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1426 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1431 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1453 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1457 "hphp.y"
    { (yyval).reset();;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { (yyval).reset();;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1465 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { (yyval).reset();;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1479 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1491 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1499 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1512 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { (yyval).reset();;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1526 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1530 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1620 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),1);
                                         _p->popLabelInfo();;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY); ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1689 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1691 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1695 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1697 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval).reset();;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1710 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1711 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1712 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         (yyval).setText("");;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1736 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1737 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1742 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1743 "hphp.y"
    { (yyval).reset();;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval).reset();;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1774 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1781 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1866 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { (yyval).reset();;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval).reset();;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval).reset();;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { (yyval).reset();;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { (yyval).reset();;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval).reset();;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1961 "hphp.y"
    { (yyval).reset();;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval).reset();;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { (yyval).reset();;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2016 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2021 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2037 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2046 "hphp.y"
    { (yyval).reset();;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2062 "hphp.y"
    { user_attribute_check(_p);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2071 "hphp.y"
    { (yyval).reset();;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2100 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2105 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval).reset();;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval)++;;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { (yyval).reset();;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2326 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2340 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2358 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]);
                                         only_in_hh_syntax(_p); ;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2374 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2385 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    {;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyval).setText("array"); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10433 "new_hphp.tab.cpp"
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
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 2472 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

