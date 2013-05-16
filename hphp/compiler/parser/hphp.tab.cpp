
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

static void on_constant(Parser *_p, Token &out, Token *stmts,
                        Token &name, Token &value) {
  Token sname;   _p->onScalar(sname, T_CONSTANT_ENCAPSED_STRING, name);

  Token fname;   fname.setText("define");
  Token params1; _p->onCallParam(params1, NULL, sname, 0);
  Token params2; _p->onCallParam(params2, &params1, value, 0);
  Token call;    _p->onCall(call, 0, fname, params2, 0);
  Token scall;   _p->onExpStatement(scall, call);

  Token stmts0;
  if (!stmts) {
    _p->onStatementListStart(stmts0);
    stmts = &stmts0;
  }
  _p->addStatement(out, *stmts, scall);
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
                      const char *clsname, Token *modifiers, bool getArgs,
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

    if (getArgs) {
      Token cname;   cname.setText("func_get_args");
      Token empty;
      Token call;    _p->onCall(call, false, cname, empty, NULL);
                     _p->onCallParam(param1, &param1, call, false);
    }

    Token cname0;  cname0.setText("hphp_create_continuation");
    Token call;    _p->onCall(call, false, cname0, param1, NULL, true);
    Token ret;     _p->onReturn(ret, &call);

    Token stmts0;  _p->onStatementListStart(stmts0);
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
#line 756 "new_hphp.tab.cpp"

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
     T_COMPILER_HALT_OFFSET = 405
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
#line 961 "new_hphp.tab.cpp"

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
#define YYLAST   11086

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  180
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  207
/* YYNRULES -- Number of rules.  */
#define YYNRULES  711
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1330

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   405

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   178,     2,   175,    47,    31,   179,
     170,   171,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   172,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   177,    30,     2,   176,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   173,    29,   174,    50,     2,     2,     2,
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
     164,   165,   166,   167,   168,   169
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
     906,   911,   915,   919,   923,   927,   929,   931,   932,   935,
     939,   946,   948,   950,   952,   959,   963,   968,   975,   978,
     982,   986,   990,   994,   998,  1002,  1006,  1010,  1014,  1018,
    1022,  1025,  1028,  1031,  1034,  1038,  1042,  1046,  1050,  1054,
    1058,  1062,  1066,  1070,  1074,  1078,  1082,  1086,  1090,  1094,
    1098,  1101,  1104,  1107,  1110,  1114,  1118,  1122,  1126,  1130,
    1134,  1138,  1142,  1146,  1150,  1156,  1161,  1163,  1166,  1169,
    1172,  1175,  1178,  1181,  1184,  1187,  1190,  1192,  1194,  1196,
    1200,  1203,  1204,  1216,  1217,  1230,  1232,  1234,  1236,  1242,
    1246,  1252,  1256,  1259,  1260,  1263,  1264,  1269,  1274,  1278,
    1283,  1288,  1293,  1298,  1300,  1302,  1306,  1312,  1313,  1317,
    1322,  1324,  1327,  1332,  1335,  1342,  1343,  1345,  1350,  1351,
    1354,  1355,  1357,  1359,  1363,  1365,  1369,  1371,  1373,  1377,
    1381,  1383,  1385,  1387,  1389,  1391,  1393,  1395,  1397,  1399,
    1401,  1403,  1405,  1407,  1409,  1411,  1413,  1415,  1417,  1419,
    1421,  1423,  1425,  1427,  1429,  1431,  1433,  1435,  1437,  1439,
    1441,  1443,  1445,  1447,  1449,  1451,  1453,  1455,  1457,  1459,
    1461,  1463,  1465,  1467,  1469,  1471,  1473,  1475,  1477,  1479,
    1481,  1483,  1485,  1487,  1489,  1491,  1493,  1495,  1497,  1499,
    1501,  1503,  1505,  1507,  1509,  1511,  1513,  1515,  1517,  1519,
    1521,  1523,  1525,  1527,  1529,  1531,  1536,  1538,  1540,  1542,
    1544,  1546,  1548,  1550,  1552,  1555,  1557,  1558,  1559,  1561,
    1563,  1567,  1568,  1570,  1572,  1574,  1576,  1578,  1580,  1582,
    1584,  1586,  1588,  1590,  1592,  1596,  1599,  1601,  1603,  1606,
    1609,  1614,  1619,  1621,  1623,  1627,  1631,  1633,  1635,  1637,
    1639,  1643,  1647,  1651,  1654,  1655,  1657,  1658,  1660,  1661,
    1667,  1671,  1675,  1677,  1679,  1681,  1683,  1687,  1690,  1692,
    1694,  1696,  1698,  1700,  1703,  1706,  1711,  1715,  1720,  1723,
    1724,  1730,  1734,  1738,  1740,  1744,  1746,  1749,  1750,  1756,
    1760,  1763,  1764,  1768,  1769,  1774,  1777,  1778,  1782,  1786,
    1788,  1789,  1791,  1794,  1797,  1802,  1806,  1810,  1813,  1818,
    1821,  1826,  1828,  1830,  1832,  1834,  1836,  1839,  1844,  1848,
    1853,  1857,  1859,  1861,  1863,  1865,  1868,  1873,  1878,  1882,
    1884,  1886,  1890,  1898,  1905,  1914,  1924,  1933,  1944,  1952,
    1959,  1961,  1964,  1969,  1974,  1976,  1978,  1983,  1985,  1986,
    1988,  1991,  1993,  1995,  1998,  2003,  2007,  2011,  2012,  2014,
    2017,  2022,  2026,  2029,  2033,  2040,  2041,  2043,  2048,  2051,
    2052,  2058,  2062,  2066,  2068,  2075,  2080,  2085,  2088,  2091,
    2092,  2098,  2102,  2106,  2108,  2111,  2112,  2118,  2122,  2126,
    2128,  2131,  2134,  2136,  2139,  2141,  2146,  2150,  2154,  2161,
    2165,  2167,  2169,  2171,  2176,  2181,  2184,  2187,  2192,  2195,
    2198,  2200,  2204,  2208,  2214,  2216,  2219,  2221,  2226,  2230,
    2231,  2233,  2237,  2241,  2243,  2245,  2246,  2247,  2250,  2254,
    2256,  2261,  2267,  2271,  2275,  2279,  2283,  2285,  2288,  2289,
    2294,  2297,  2300,  2303,  2305,  2307,  2312,  2319,  2321,  2330,
    2336,  2338
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     181,     0,    -1,    -1,   182,   183,    -1,   183,   184,    -1,
      -1,   198,    -1,   210,    -1,   213,    -1,   218,    -1,   373,
      -1,   116,   170,   171,   172,    -1,   141,   190,   172,    -1,
      -1,   141,   190,   173,   185,   183,   174,    -1,    -1,   141,
     173,   186,   183,   174,    -1,   104,   188,   172,    -1,   195,
     172,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   188,     8,   189,    -1,   189,    -1,
     190,    -1,   144,   190,    -1,   190,    90,   187,    -1,   144,
     190,    90,   187,    -1,   187,    -1,   190,   144,   187,    -1,
     190,    -1,   141,   144,   190,    -1,   144,   190,    -1,   191,
      -1,   191,   376,    -1,   191,   376,    -1,   195,     8,   374,
      13,   321,    -1,    99,   374,    13,   321,    -1,   196,   197,
      -1,    -1,   198,    -1,   210,    -1,   213,    -1,   218,    -1,
     173,   196,   174,    -1,    65,   281,   198,   240,   242,    -1,
      65,   281,    26,   196,   241,   243,    68,   172,    -1,    -1,
      82,   281,   199,   234,    -1,    -1,    81,   200,   198,    82,
     281,   172,    -1,    -1,    84,   170,   283,   172,   283,   172,
     283,   171,   201,   232,    -1,    -1,    91,   281,   202,   237,
      -1,    95,   172,    -1,    95,   287,   172,    -1,    97,   172,
      -1,    97,   287,   172,    -1,   100,   172,    -1,   100,   287,
     172,    -1,   145,    95,   172,    -1,   105,   248,   172,    -1,
     111,   250,   172,    -1,    80,   282,   172,    -1,   113,   170,
     371,   171,   172,    -1,   172,    -1,    75,    -1,    -1,    86,
     170,   287,    90,   231,   230,   171,   203,   233,    -1,    88,
     170,   236,   171,   235,    -1,   101,   173,   196,   174,   102,
     170,   314,    73,   171,   173,   196,   174,   204,   207,    -1,
     101,   173,   196,   174,   205,    -1,   103,   287,   172,    -1,
      96,   187,   172,    -1,   287,   172,    -1,   284,   172,    -1,
     285,   172,    -1,   286,   172,    -1,   187,    26,    -1,   204,
     102,   170,   314,    73,   171,   173,   196,   174,    -1,    -1,
      -1,   206,   159,   173,   196,   174,    -1,   205,    -1,    -1,
      31,    -1,    -1,    98,    -1,    -1,   209,   208,   375,   211,
     170,   244,   171,   379,   173,   196,   174,    -1,    -1,   341,
     209,   208,   375,   212,   170,   244,   171,   379,   173,   196,
     174,    -1,    -1,   224,   221,   214,   225,   226,   173,   251,
     174,    -1,    -1,   341,   224,   221,   215,   225,   226,   173,
     251,   174,    -1,    -1,   118,   222,   216,   227,   173,   251,
     174,    -1,    -1,   341,   118,   222,   217,   227,   173,   251,
     174,    -1,    -1,   154,   223,   219,   226,   173,   251,   174,
      -1,    -1,   341,   154,   223,   220,   226,   173,   251,   174,
      -1,   375,    -1,   146,    -1,   375,    -1,   375,    -1,   117,
      -1,   110,   117,    -1,   109,   117,    -1,   119,   314,    -1,
      -1,   120,   228,    -1,    -1,   119,   228,    -1,    -1,   314,
      -1,   228,     8,   314,    -1,   314,    -1,   229,     8,   314,
      -1,   122,   231,    -1,    -1,   348,    -1,    31,   348,    -1,
     198,    -1,    26,   196,    85,   172,    -1,   198,    -1,    26,
     196,    87,   172,    -1,   198,    -1,    26,   196,    83,   172,
      -1,   198,    -1,    26,   196,    89,   172,    -1,   187,    13,
     321,    -1,   236,     8,   187,    13,   321,    -1,   173,   238,
     174,    -1,   173,   172,   238,   174,    -1,    26,   238,    92,
     172,    -1,    26,   172,   238,    92,   172,    -1,   238,    93,
     287,   239,   196,    -1,   238,    94,   239,   196,    -1,    -1,
      26,    -1,   172,    -1,   240,    66,   281,   198,    -1,    -1,
     241,    66,   281,    26,   196,    -1,    -1,    67,   198,    -1,
      -1,    67,    26,   196,    -1,    -1,   245,     8,   157,    -1,
     245,   326,    -1,   157,    -1,    -1,   342,   386,    73,    -1,
     342,   386,    31,    73,    -1,   342,   386,    31,    73,    13,
     321,    -1,   342,   386,    73,    13,   321,    -1,   245,     8,
     342,   386,    73,    -1,   245,     8,   342,   386,    31,    73,
      -1,   245,     8,   342,   386,    31,    73,    13,   321,    -1,
     245,     8,   342,   386,    73,    13,   321,    -1,   247,   326,
      -1,    -1,   287,    -1,    31,   348,    -1,   247,     8,   287,
      -1,   247,     8,    31,   348,    -1,   248,     8,   249,    -1,
     249,    -1,    73,    -1,   175,   348,    -1,   175,   173,   287,
     174,    -1,   250,     8,    73,    -1,   250,     8,    73,    13,
     321,    -1,    73,    -1,    73,    13,   321,    -1,   251,   252,
      -1,    -1,    -1,   274,   253,   278,   172,    -1,    -1,   276,
     385,   254,   278,   172,    -1,   279,   172,    -1,    -1,   275,
     209,   208,   375,   170,   255,   244,   171,   379,   273,    -1,
      -1,   341,   275,   209,   208,   375,   170,   256,   244,   171,
     379,   273,    -1,   148,   261,   172,    -1,   149,   267,   172,
      -1,   151,   269,   172,    -1,   104,   229,   172,    -1,   104,
     229,   173,   257,   174,    -1,   257,   258,    -1,   257,   259,
      -1,    -1,   194,   140,   187,   155,   229,   172,    -1,   260,
      90,   275,   187,   172,    -1,   260,    90,   276,   172,    -1,
     194,   140,   187,    -1,   187,    -1,   262,    -1,   261,     8,
     262,    -1,   263,   311,   265,   266,    -1,   146,    -1,   124,
      -1,   314,    -1,   112,    -1,   152,   173,   264,   174,    -1,
     320,    -1,   264,     8,   320,    -1,    13,   321,    -1,    -1,
      51,   153,    -1,    -1,   268,    -1,   267,     8,   268,    -1,
     150,    -1,   270,    -1,   187,    -1,   115,    -1,   170,   271,
     171,    -1,   170,   271,   171,    45,    -1,   170,   271,   171,
      25,    -1,   170,   271,   171,    42,    -1,   270,    -1,   272,
      -1,   272,    45,    -1,   272,    25,    -1,   272,    42,    -1,
     271,     8,   271,    -1,   271,    29,   271,    -1,   187,    -1,
     146,    -1,   150,    -1,   172,    -1,   173,   196,   174,    -1,
     276,    -1,   112,    -1,   276,    -1,    -1,   277,    -1,   276,
     277,    -1,   106,    -1,   107,    -1,   108,    -1,   111,    -1,
     110,    -1,   109,    -1,   278,     8,    73,    -1,   278,     8,
      73,    13,   321,    -1,    73,    -1,    73,    13,   321,    -1,
     279,     8,   374,    13,   321,    -1,    99,   374,    13,   321,
      -1,    63,   316,   319,    -1,   170,   280,   171,    -1,   170,
     287,   171,    -1,   282,     8,   287,    -1,   287,    -1,   282,
      -1,    -1,   145,   287,    -1,   348,    13,   284,    -1,   123,
     170,   360,   171,    13,   284,    -1,   288,    -1,   348,    -1,
     280,    -1,   123,   170,   360,   171,    13,   287,    -1,   348,
      13,   287,    -1,   348,    13,    31,   348,    -1,   348,    13,
      31,    63,   316,   319,    -1,    62,   287,    -1,   348,    24,
     287,    -1,   348,    23,   287,    -1,   348,    22,   287,    -1,
     348,    21,   287,    -1,   348,    20,   287,    -1,   348,    19,
     287,    -1,   348,    18,   287,    -1,   348,    17,   287,    -1,
     348,    16,   287,    -1,   348,    15,   287,    -1,   348,    14,
     287,    -1,   348,    60,    -1,    60,   348,    -1,   348,    59,
      -1,    59,   348,    -1,   287,    27,   287,    -1,   287,    28,
     287,    -1,   287,     9,   287,    -1,   287,    11,   287,    -1,
     287,    10,   287,    -1,   287,    29,   287,    -1,   287,    31,
     287,    -1,   287,    30,   287,    -1,   287,    44,   287,    -1,
     287,    42,   287,    -1,   287,    43,   287,    -1,   287,    45,
     287,    -1,   287,    46,   287,    -1,   287,    47,   287,    -1,
     287,    41,   287,    -1,   287,    40,   287,    -1,    42,   287,
      -1,    43,   287,    -1,    48,   287,    -1,    50,   287,    -1,
     287,    33,   287,    -1,   287,    32,   287,    -1,   287,    35,
     287,    -1,   287,    34,   287,    -1,   287,    36,   287,    -1,
     287,    39,   287,    -1,   287,    37,   287,    -1,   287,    38,
     287,    -1,   287,    49,   316,    -1,   170,   288,   171,    -1,
     287,    25,   287,    26,   287,    -1,   287,    25,    26,   287,
      -1,   370,    -1,    58,   287,    -1,    57,   287,    -1,    56,
     287,    -1,    55,   287,    -1,    54,   287,    -1,    53,   287,
      -1,    52,   287,    -1,    64,   317,    -1,    51,   287,    -1,
     323,    -1,   296,    -1,   295,    -1,   176,   318,   176,    -1,
      12,   287,    -1,    -1,   209,   208,   170,   289,   244,   171,
     379,   301,   173,   196,   174,    -1,    -1,   111,   209,   208,
     170,   290,   244,   171,   379,   301,   173,   196,   174,    -1,
     303,    -1,   299,    -1,   297,    -1,   291,     8,    79,   122,
     287,    -1,    79,   122,   287,    -1,   292,     8,    79,   122,
     321,    -1,    79,   122,   321,    -1,   291,   325,    -1,    -1,
     292,   325,    -1,    -1,   166,   170,   293,   171,    -1,   124,
     170,   361,   171,    -1,    61,   361,   177,    -1,   314,   173,
     363,   174,    -1,   314,   173,   365,   174,    -1,   299,    61,
     356,   177,    -1,   300,    61,   356,   177,    -1,   296,    -1,
     372,    -1,   170,   288,   171,    -1,   104,   170,   302,   326,
     171,    -1,    -1,   302,     8,    73,    -1,   302,     8,    31,
      73,    -1,    73,    -1,    31,    73,    -1,   160,   146,   304,
     161,    -1,   306,    46,    -1,   306,   161,   307,   160,    46,
     305,    -1,    -1,   146,    -1,   306,   308,    13,   309,    -1,
      -1,   307,   310,    -1,    -1,   146,    -1,   147,    -1,   173,
     287,   174,    -1,   147,    -1,   173,   287,   174,    -1,   303,
      -1,   312,    -1,   311,    26,   312,    -1,   311,    43,   312,
      -1,   187,    -1,    64,    -1,    98,    -1,    99,    -1,   100,
      -1,   145,    -1,   101,    -1,   102,    -1,   159,    -1,   103,
      -1,    65,    -1,    66,    -1,    68,    -1,    67,    -1,    82,
      -1,    83,    -1,    81,    -1,    84,    -1,    85,    -1,    86,
      -1,    87,    -1,    88,    -1,    89,    -1,    49,    -1,    90,
      -1,    91,    -1,    92,    -1,    93,    -1,    94,    -1,    95,
      -1,    97,    -1,    96,    -1,    80,    -1,    12,    -1,   117,
      -1,   118,    -1,   119,    -1,   120,    -1,    63,    -1,    62,
      -1,   112,    -1,     5,    -1,     7,    -1,     6,    -1,     4,
      -1,     3,    -1,   141,    -1,   104,    -1,   105,    -1,   114,
      -1,   115,    -1,   116,    -1,   111,    -1,   110,    -1,   109,
      -1,   108,    -1,   107,    -1,   106,    -1,   113,    -1,   123,
      -1,   124,    -1,     9,    -1,    11,    -1,    10,    -1,   125,
      -1,   127,    -1,   126,    -1,   128,    -1,   129,    -1,   143,
      -1,   142,    -1,   169,    -1,   154,    -1,   156,    -1,   167,
      -1,   193,   170,   246,   171,    -1,   194,    -1,   146,    -1,
     314,    -1,   111,    -1,   354,    -1,   314,    -1,   111,    -1,
     358,    -1,   170,   171,    -1,   281,    -1,    -1,    -1,    78,
      -1,   367,    -1,   170,   246,   171,    -1,    -1,    69,    -1,
      70,    -1,    79,    -1,   128,    -1,   129,    -1,   143,    -1,
     125,    -1,   156,    -1,   126,    -1,   127,    -1,   142,    -1,
     169,    -1,   136,    78,   137,    -1,   136,   137,    -1,   320,
      -1,   192,    -1,    42,   321,    -1,    43,   321,    -1,   124,
     170,   324,   171,    -1,   166,   170,   294,   171,    -1,   322,
      -1,   298,    -1,   194,   140,   187,    -1,   146,   140,   187,
      -1,   192,    -1,    72,    -1,   372,    -1,   320,    -1,   178,
     367,   178,    -1,   179,   367,   179,    -1,   136,   367,   137,
      -1,   327,   325,    -1,    -1,     8,    -1,    -1,     8,    -1,
      -1,   327,     8,   321,   122,   321,    -1,   327,     8,   321,
      -1,   321,   122,   321,    -1,   321,    -1,    69,    -1,    70,
      -1,    79,    -1,   136,    78,   137,    -1,   136,   137,    -1,
      69,    -1,    70,    -1,   187,    -1,   328,    -1,   187,    -1,
      42,   329,    -1,    43,   329,    -1,   124,   170,   331,   171,
      -1,    61,   331,   177,    -1,   166,   170,   334,   171,    -1,
     332,   325,    -1,    -1,   332,     8,   330,   122,   330,    -1,
     332,     8,   330,    -1,   330,   122,   330,    -1,   330,    -1,
     333,     8,   330,    -1,   330,    -1,   335,   325,    -1,    -1,
     335,     8,    79,   122,   330,    -1,    79,   122,   330,    -1,
     333,   325,    -1,    -1,   170,   336,   171,    -1,    -1,   338,
       8,   187,   337,    -1,   187,   337,    -1,    -1,   340,   338,
     325,    -1,    41,   339,    40,    -1,   341,    -1,    -1,   344,
      -1,   121,   353,    -1,   121,   187,    -1,   121,   173,   287,
     174,    -1,    61,   356,   177,    -1,   173,   287,   174,    -1,
     349,   345,    -1,   170,   280,   171,   345,    -1,   359,   345,
      -1,   170,   280,   171,   345,    -1,   353,    -1,   313,    -1,
     351,    -1,   352,    -1,   346,    -1,   348,   343,    -1,   170,
     280,   171,   343,    -1,   315,   140,   353,    -1,   350,   170,
     246,   171,    -1,   170,   348,   171,    -1,   313,    -1,   351,
      -1,   352,    -1,   346,    -1,   348,   344,    -1,   170,   280,
     171,   344,    -1,   350,   170,   246,   171,    -1,   170,   348,
     171,    -1,   353,    -1,   346,    -1,   170,   348,   171,    -1,
     348,   121,   187,   376,   170,   246,   171,    -1,   348,   121,
     353,   170,   246,   171,    -1,   348,   121,   173,   287,   174,
     170,   246,   171,    -1,   170,   280,   171,   121,   187,   376,
     170,   246,   171,    -1,   170,   280,   171,   121,   353,   170,
     246,   171,    -1,   170,   280,   171,   121,   173,   287,   174,
     170,   246,   171,    -1,   315,   140,   187,   376,   170,   246,
     171,    -1,   315,   140,   353,   170,   246,   171,    -1,   354,
      -1,   357,   354,    -1,   354,    61,   356,   177,    -1,   354,
     173,   287,   174,    -1,   355,    -1,    73,    -1,   175,   173,
     287,   174,    -1,   287,    -1,    -1,   175,    -1,   357,   175,
      -1,   353,    -1,   347,    -1,   358,   343,    -1,   170,   280,
     171,   343,    -1,   315,   140,   353,    -1,   170,   348,   171,
      -1,    -1,   347,    -1,   358,   344,    -1,   170,   280,   171,
     344,    -1,   170,   348,   171,    -1,   360,     8,    -1,   360,
       8,   348,    -1,   360,     8,   123,   170,   360,   171,    -1,
      -1,   348,    -1,   123,   170,   360,   171,    -1,   362,   325,
      -1,    -1,   362,     8,   287,   122,   287,    -1,   362,     8,
     287,    -1,   287,   122,   287,    -1,   287,    -1,   362,     8,
     287,   122,    31,   348,    -1,   362,     8,    31,   348,    -1,
     287,   122,    31,   348,    -1,    31,   348,    -1,   364,   325,
      -1,    -1,   364,     8,   287,   122,   287,    -1,   364,     8,
     287,    -1,   287,   122,   287,    -1,   287,    -1,   366,   325,
      -1,    -1,   366,     8,   321,   122,   321,    -1,   366,     8,
     321,    -1,   321,   122,   321,    -1,   321,    -1,   367,   368,
      -1,   367,    78,    -1,   368,    -1,    78,   368,    -1,    73,
      -1,    73,    61,   369,   177,    -1,    73,   121,   187,    -1,
     138,   287,   174,    -1,   138,    72,    61,   287,   177,   174,
      -1,   139,   348,   174,    -1,   187,    -1,    74,    -1,    73,
      -1,   114,   170,   371,   171,    -1,   115,   170,   348,   171,
      -1,     7,   287,    -1,     6,   287,    -1,     5,   170,   287,
     171,    -1,     4,   287,    -1,     3,   287,    -1,   348,    -1,
     371,     8,   348,    -1,   315,   140,   187,    -1,   167,   375,
      13,   385,   172,    -1,   187,    -1,   385,   187,    -1,   187,
      -1,   187,   162,   380,   163,    -1,   162,   377,   163,    -1,
      -1,   385,    -1,   377,     8,   385,    -1,   377,     8,   157,
      -1,   377,    -1,   157,    -1,    -1,    -1,    26,   385,    -1,
     192,     8,   380,    -1,   192,    -1,   192,    90,   384,   380,
      -1,   192,    90,   192,     8,   380,    -1,   192,    90,   192,
      -1,   192,    90,   384,    -1,    79,   122,   385,    -1,   382,
       8,   381,    -1,   381,    -1,   382,   325,    -1,    -1,   166,
     170,   383,   171,    -1,    25,   385,    -1,    51,   385,    -1,
     192,   376,    -1,   124,    -1,   384,    -1,   124,   162,   385,
     163,    -1,   124,   162,   385,     8,   385,   163,    -1,   146,
      -1,   170,    98,   170,   378,   171,    26,   385,   171,    -1,
     170,   377,     8,   385,   171,    -1,   385,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   815,   815,   815,   820,   822,   825,   826,   827,   828,
     829,   830,   833,   835,   835,   837,   837,   839,   840,   845,
     846,   847,   848,   849,   850,   854,   856,   859,   860,   861,
     862,   867,   868,   872,   873,   875,   878,   884,   891,   898,
     902,   908,   910,   913,   914,   915,   916,   919,   920,   924,
     929,   929,   933,   933,   938,   937,   941,   941,   944,   945,
     946,   947,   948,   949,   950,   951,   952,   953,   954,   955,
     956,   959,   957,   962,   964,   972,   975,   976,   980,   981,
     982,   983,   984,   991,   997,  1001,  1001,  1007,  1008,  1012,
    1013,  1017,  1022,  1021,  1031,  1030,  1043,  1042,  1061,  1059,
    1078,  1077,  1086,  1084,  1096,  1095,  1107,  1105,  1118,  1119,
    1123,  1126,  1129,  1130,  1131,  1134,  1136,  1139,  1140,  1143,
    1144,  1147,  1148,  1152,  1153,  1158,  1159,  1162,  1163,  1167,
    1168,  1172,  1173,  1177,  1178,  1182,  1183,  1188,  1189,  1194,
    1195,  1196,  1197,  1200,  1203,  1205,  1208,  1209,  1213,  1215,
    1218,  1221,  1224,  1225,  1228,  1229,  1233,  1235,  1237,  1238,
    1242,  1244,  1246,  1249,  1252,  1255,  1258,  1262,  1269,  1271,
    1274,  1275,  1276,  1278,  1283,  1284,  1287,  1288,  1289,  1293,
    1294,  1296,  1297,  1301,  1303,  1306,  1306,  1310,  1309,  1313,
    1317,  1315,  1328,  1325,  1336,  1338,  1340,  1342,  1344,  1348,
    1349,  1350,  1353,  1359,  1362,  1368,  1371,  1376,  1378,  1383,
    1388,  1392,  1393,  1399,  1400,  1405,  1406,  1411,  1412,  1416,
    1417,  1421,  1423,  1429,  1434,  1435,  1437,  1441,  1442,  1443,
    1444,  1448,  1449,  1450,  1451,  1452,  1453,  1455,  1460,  1463,
    1464,  1468,  1469,  1472,  1473,  1476,  1477,  1480,  1481,  1485,
    1486,  1487,  1488,  1489,  1490,  1493,  1495,  1497,  1498,  1501,
    1503,  1507,  1509,  1513,  1517,  1518,  1522,  1523,  1527,  1531,
    1535,  1540,  1541,  1542,  1545,  1547,  1548,  1549,  1552,  1553,
    1554,  1555,  1556,  1557,  1558,  1559,  1560,  1561,  1562,  1563,
    1564,  1565,  1566,  1567,  1568,  1569,  1570,  1571,  1572,  1573,
    1574,  1575,  1576,  1577,  1578,  1579,  1580,  1581,  1582,  1583,
    1584,  1585,  1586,  1587,  1588,  1589,  1590,  1591,  1592,  1593,
    1595,  1596,  1598,  1600,  1601,  1602,  1603,  1604,  1605,  1606,
    1607,  1608,  1609,  1610,  1611,  1612,  1613,  1614,  1615,  1616,
    1617,  1619,  1618,  1627,  1626,  1634,  1635,  1636,  1640,  1645,
    1652,  1657,  1664,  1666,  1670,  1672,  1676,  1681,  1682,  1686,
    1693,  1700,  1702,  1707,  1708,  1709,  1713,  1717,  1721,  1722,
    1723,  1724,  1728,  1734,  1739,  1748,  1749,  1752,  1755,  1758,
    1759,  1762,  1766,  1769,  1772,  1779,  1780,  1784,  1785,  1787,
    1791,  1792,  1793,  1794,  1795,  1796,  1797,  1798,  1799,  1800,
    1801,  1802,  1803,  1804,  1805,  1806,  1807,  1808,  1809,  1810,
    1811,  1812,  1813,  1814,  1815,  1816,  1817,  1818,  1819,  1820,
    1821,  1822,  1823,  1824,  1825,  1826,  1827,  1828,  1829,  1830,
    1831,  1832,  1833,  1834,  1835,  1836,  1837,  1838,  1839,  1840,
    1841,  1842,  1843,  1844,  1845,  1846,  1847,  1848,  1849,  1850,
    1851,  1852,  1853,  1854,  1855,  1856,  1857,  1858,  1859,  1860,
    1861,  1862,  1863,  1864,  1865,  1869,  1874,  1875,  1878,  1879,
    1880,  1884,  1885,  1886,  1890,  1891,  1892,  1896,  1897,  1898,
    1901,  1903,  1907,  1908,  1909,  1911,  1912,  1913,  1914,  1915,
    1916,  1917,  1918,  1919,  1920,  1923,  1928,  1929,  1930,  1931,
    1932,  1934,  1937,  1938,  1942,  1945,  1951,  1952,  1953,  1954,
    1955,  1956,  1957,  1962,  1964,  1968,  1969,  1972,  1973,  1977,
    1980,  1982,  1984,  1988,  1989,  1990,  1992,  1995,  1999,  2000,
    2001,  2004,  2005,  2006,  2007,  2008,  2010,  2011,  2017,  2019,
    2022,  2025,  2027,  2029,  2032,  2034,  2038,  2040,  2043,  2047,
    2054,  2056,  2059,  2060,  2065,  2068,  2072,  2072,  2077,  2080,
    2081,  2085,  2086,  2091,  2092,  2096,  2097,  2101,  2102,  2106,
    2108,  2112,  2113,  2114,  2115,  2116,  2117,  2118,  2119,  2122,
    2124,  2128,  2129,  2130,  2131,  2132,  2134,  2136,  2138,  2142,
    2143,  2144,  2148,  2151,  2154,  2157,  2160,  2163,  2169,  2173,
    2180,  2181,  2186,  2188,  2189,  2192,  2193,  2196,  2197,  2201,
    2202,  2206,  2207,  2208,  2209,  2210,  2213,  2216,  2217,  2218,
    2220,  2222,  2226,  2227,  2228,  2230,  2231,  2232,  2236,  2238,
    2241,  2243,  2244,  2245,  2246,  2249,  2251,  2252,  2256,  2258,
    2261,  2263,  2264,  2265,  2269,  2271,  2274,  2277,  2279,  2281,
    2285,  2286,  2288,  2289,  2295,  2296,  2298,  2300,  2302,  2304,
    2307,  2308,  2309,  2313,  2314,  2315,  2316,  2317,  2318,  2319,
    2323,  2324,  2328,  2337,  2344,  2345,  2351,  2352,  2360,  2363,
    2367,  2370,  2375,  2376,  2377,  2378,  2382,  2383,  2387,  2389,
    2390,  2392,  2395,  2397,  2402,  2408,  2410,  2414,  2417,  2420,
    2429,  2432,  2435,  2436,  2439,  2440,  2444,  2449,  2453,  2459,
    2467,  2468
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
  "T_UNRESOLVED_TYPE", "T_COMPILER_HALT_OFFSET", "'('", "')'", "';'",
  "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept", "start",
  "$@1", "top_statement_list", "top_statement", "$@2", "$@3", "ident",
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
  "class_variable_declaration", "class_constant_declaration", "new_expr",
  "parenthesis_expr", "expr_list", "for_expr", "yield_expr",
  "yield_assign_expr", "yield_list_assign_expr", "expr",
  "expr_no_variable", "$@22", "$@23", "non_empty_shape_pair_list",
  "non_empty_static_shape_pair_list", "shape_pair_list",
  "static_shape_pair_list", "shape_literal", "array_literal",
  "collection_literal", "static_collection_literal", "dim_expr",
  "dim_expr_base", "lexical_vars", "lexical_var_list", "xhp_tag",
  "xhp_tag_body", "xhp_opt_end_label", "xhp_attributes", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label_ws",
  "xhp_bareword", "simple_function_call", "fully_qualified_class_name",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_scalar",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "possible_comma_in_hphp_syntax",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_scalar_ae",
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
  "class_constant", "hh_typedef_statement", "hh_name_with_type",
  "hh_name_with_typevar", "hh_typeargs_opt", "hh_type_list",
  "hh_func_type_list", "hh_opt_return_type", "hh_typevar_list",
  "hh_shape_member_type", "hh_non_empty_shape_member_list",
  "hh_shape_member_list", "hh_shape_type", "hh_type", "hh_type_opt", 0
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
      40,    41,    59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   180,   182,   181,   183,   183,   184,   184,   184,   184,
     184,   184,   184,   185,   184,   186,   184,   184,   184,   187,
     187,   187,   187,   187,   187,   188,   188,   189,   189,   189,
     189,   190,   190,   191,   191,   191,   192,   193,   194,   195,
     195,   196,   196,   197,   197,   197,   197,   198,   198,   198,
     199,   198,   200,   198,   201,   198,   202,   198,   198,   198,
     198,   198,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   203,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   198,   198,   204,   204,   206,   205,   207,   207,   208,
     208,   209,   211,   210,   212,   210,   214,   213,   215,   213,
     216,   213,   217,   213,   219,   218,   220,   218,   221,   221,
     222,   223,   224,   224,   224,   225,   225,   226,   226,   227,
     227,   228,   228,   229,   229,   230,   230,   231,   231,   232,
     232,   233,   233,   234,   234,   235,   235,   236,   236,   237,
     237,   237,   237,   238,   238,   238,   239,   239,   240,   240,
     241,   241,   242,   242,   243,   243,   244,   244,   244,   244,
     245,   245,   245,   245,   245,   245,   245,   245,   246,   246,
     247,   247,   247,   247,   248,   248,   249,   249,   249,   250,
     250,   250,   250,   251,   251,   253,   252,   254,   252,   252,
     255,   252,   256,   252,   252,   252,   252,   252,   252,   257,
     257,   257,   258,   259,   259,   260,   260,   261,   261,   262,
     262,   263,   263,   263,   263,   264,   264,   265,   265,   266,
     266,   267,   267,   268,   269,   269,   269,   270,   270,   270,
     270,   271,   271,   271,   271,   271,   271,   271,   272,   272,
     272,   273,   273,   274,   274,   275,   275,   276,   276,   277,
     277,   277,   277,   277,   277,   278,   278,   278,   278,   279,
     279,   280,   280,   281,   282,   282,   283,   283,   284,   285,
     286,   287,   287,   287,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   289,   288,   290,   288,   288,   288,   288,   291,   291,
     292,   292,   293,   293,   294,   294,   295,   296,   296,   297,
     298,   299,   299,   300,   300,   300,   301,   301,   302,   302,
     302,   302,   303,   304,   304,   305,   305,   306,   306,   307,
     307,   308,   309,   309,   310,   310,   310,   311,   311,   311,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   312,   312,   312,
     312,   312,   312,   312,   312,   313,   314,   314,   315,   315,
     315,   316,   316,   316,   317,   317,   317,   318,   318,   318,
     319,   319,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   321,   321,   321,   321,
     321,   321,   321,   321,   322,   322,   323,   323,   323,   323,
     323,   323,   323,   324,   324,   325,   325,   326,   326,   327,
     327,   327,   327,   328,   328,   328,   328,   328,   329,   329,
     329,   330,   330,   330,   330,   330,   330,   330,   331,   331,
     332,   332,   332,   332,   333,   333,   334,   334,   335,   335,
     336,   336,   337,   337,   338,   338,   340,   339,   341,   342,
     342,   343,   343,   344,   344,   345,   345,   346,   346,   347,
     347,   348,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   349,   349,   349,   349,   349,   349,   349,   349,   350,
     350,   350,   351,   351,   351,   351,   351,   351,   352,   352,
     353,   353,   354,   354,   354,   355,   355,   356,   356,   357,
     357,   358,   358,   358,   358,   358,   358,   359,   359,   359,
     359,   359,   360,   360,   360,   360,   360,   360,   361,   361,
     362,   362,   362,   362,   362,   362,   362,   362,   363,   363,
     364,   364,   364,   364,   365,   365,   366,   366,   366,   366,
     367,   367,   367,   367,   368,   368,   368,   368,   368,   368,
     369,   369,   369,   370,   370,   370,   370,   370,   370,   370,
     371,   371,   372,   373,   374,   374,   375,   375,   376,   376,
     377,   377,   378,   378,   378,   378,   379,   379,   380,   380,
     380,   380,   380,   380,   381,   382,   382,   383,   383,   384,
     385,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     386,   386
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
       4,     3,     3,     3,     3,     1,     1,     0,     2,     3,
       6,     1,     1,     1,     6,     3,     4,     6,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     4,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     3,
       2,     0,    11,     0,    12,     1,     1,     1,     5,     3,
       5,     3,     2,     0,     2,     0,     4,     4,     3,     4,
       4,     4,     4,     1,     1,     3,     5,     0,     3,     4,
       1,     2,     4,     2,     6,     0,     1,     4,     0,     2,
       0,     1,     1,     3,     1,     3,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     0,     0,     1,     1,
       3,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     2,     2,
       4,     4,     1,     1,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     2,     2,     4,     3,     4,     2,     0,
       5,     3,     3,     1,     3,     1,     2,     0,     5,     3,
       2,     0,     3,     0,     4,     2,     0,     3,     3,     1,
       0,     1,     2,     2,     4,     3,     3,     2,     4,     2,
       4,     1,     1,     1,     1,     1,     2,     4,     3,     4,
       3,     1,     1,     1,     1,     2,     4,     4,     3,     1,
       1,     3,     7,     6,     8,     9,     8,    10,     7,     6,
       1,     2,     4,     4,     1,     1,     4,     1,     0,     1,
       2,     1,     1,     2,     4,     3,     3,     0,     1,     2,
       4,     3,     2,     3,     6,     0,     1,     4,     2,     0,
       5,     3,     3,     1,     6,     4,     4,     2,     2,     0,
       5,     3,     3,     1,     2,     0,     5,     3,     3,     1,
       2,     2,     1,     2,     1,     4,     3,     3,     6,     3,
       1,     1,     1,     4,     4,     2,     2,     4,     2,     2,
       1,     3,     3,     5,     1,     2,     1,     4,     3,     0,
       1,     3,     3,     1,     1,     0,     0,     2,     3,     1,
       4,     5,     3,     3,     3,     3,     1,     2,     0,     4,
       2,     2,     2,     1,     1,     4,     6,     1,     8,     5,
       1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   556,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   629,     0,   617,   476,
       0,   482,   483,    19,   507,   605,    70,   484,     0,    52,
       0,     0,     0,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,   469,     0,     0,
       0,     0,   112,     0,     0,     0,   488,   490,   491,   485,
     486,     0,     0,   492,   487,     0,     0,   467,    20,    21,
      22,    24,    23,     0,   489,     0,     0,     0,   493,     0,
      69,    42,   609,   477,     0,     0,     4,    31,    33,    36,
     506,     0,   466,     0,     6,    90,     7,     8,     9,     0,
     273,     0,     0,     0,     0,   271,   338,   337,   347,   346,
       0,   345,   572,   468,     0,   509,   336,     0,   575,   272,
       0,     0,   573,   574,   571,   600,   604,     0,   326,   508,
      10,   469,     0,     0,    31,    90,   669,   272,   668,     0,
     666,   665,   340,     0,     0,   310,   311,   312,   313,   335,
     333,   332,   331,   330,   329,   328,   327,   469,     0,   679,
     468,     0,   293,   291,     0,   633,     0,   516,   278,   472,
       0,   679,   471,     0,   481,   612,   611,   473,     0,     0,
     475,   334,     0,     0,     0,   265,     0,    50,   267,     0,
       0,    56,    58,     0,     0,    60,     0,     0,     0,   703,
     707,     0,     0,    31,    36,   679,     0,   704,     0,    62,
       0,    42,     0,     0,     0,    26,    27,   176,     0,     0,
     175,   114,   113,   181,    90,     0,     0,     0,     0,     0,
     676,   100,   110,   625,   629,   654,     0,   495,     0,     0,
       0,   652,     0,    15,     0,    35,     0,   268,   104,   111,
     378,   353,     0,   273,     0,   271,   272,     0,     0,   478,
       0,   479,     0,     0,     0,    82,     0,     0,    38,   169,
       0,    18,    89,     0,   109,    96,   108,    79,    80,    81,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   617,    78,   608,   608,   639,
       0,     0,     0,    90,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   292,   290,     0,
     576,   561,   608,     0,   567,   169,   608,     0,   610,   601,
     625,     0,     0,     0,   558,   553,   516,     0,     0,     0,
       0,   637,     0,   358,   515,   628,     0,     0,    38,     0,
     169,   261,     0,   613,   561,   569,   474,     0,    42,   149,
       0,    67,     0,     0,   266,     0,     0,     0,     0,     0,
      59,    77,    61,   700,   701,     0,   698,     0,     0,   680,
     702,     0,   675,    63,     0,    76,    28,     0,    17,     0,
       0,   177,     0,    65,     0,     0,     0,    66,   670,     0,
       0,     0,     0,     0,   120,     0,   626,     0,     0,     0,
       0,   494,   653,   507,     0,     0,   651,   512,   650,    34,
       5,    12,    13,    64,   118,     0,     0,     0,   516,     0,
       0,   262,   323,   580,    47,    41,    43,    44,    45,    46,
       0,   339,   510,   511,    32,     0,     0,     0,   518,   170,
       0,   341,    92,   116,   296,   298,   297,     0,     0,   294,
     295,   299,   301,   300,   315,   314,   317,   316,   318,   320,
     321,   319,   309,   308,   303,   304,   302,   305,   306,   307,
     322,   607,     0,     0,   643,     0,   516,   672,   578,   600,
     102,   106,     0,    98,     0,     0,   269,   275,   289,   288,
     287,   286,   285,   284,   283,   282,   281,   280,   279,     0,
     563,   562,     0,     0,     0,     0,     0,     0,   667,   551,
     555,   515,   557,     0,     0,   679,     0,   632,     0,   631,
       0,   616,   615,     0,     0,   563,   562,   263,   151,   153,
     264,     0,    42,   133,    51,   267,     0,     0,     0,     0,
     145,   145,    57,     0,     0,   696,   516,     0,   685,     0,
       0,     0,     0,     0,   467,     0,    36,   497,   466,   503,
       0,   496,    40,   502,    85,     0,    25,    29,     0,   174,
     182,   343,   179,     0,     0,   663,   664,    11,   689,     0,
       0,     0,   625,   622,     0,   357,   662,   661,   660,     0,
     656,     0,   657,   659,     0,     5,     0,     0,   372,   373,
     381,   380,     0,     0,   515,   352,   356,     0,     0,   577,
     561,   568,   606,     0,   678,   171,   465,   517,   168,     0,
     560,     0,     0,   118,   325,     0,   361,   362,     0,   359,
     515,   638,     0,   169,   120,   118,    94,   116,   617,   276,
       0,     0,   169,   565,   566,   579,   602,   603,     0,     0,
       0,   539,   523,   524,   525,     0,     0,     0,   532,   531,
     545,   516,     0,   553,   636,   635,     0,   614,   561,   570,
     480,     0,   155,     0,     0,    48,     0,     0,     0,     0,
     126,   127,   137,     0,    42,   135,    73,   145,     0,   145,
       0,     0,   705,     0,   515,   697,   699,   684,   683,     0,
     681,   498,   499,   514,     0,     0,   355,     0,   645,     0,
      75,     0,    30,   178,   560,     0,   671,    68,     0,     0,
     677,   119,   121,   184,     0,     0,   623,     0,   655,     0,
      16,     0,   117,   184,     0,     0,   349,     0,   673,     0,
     563,   562,   681,     0,   172,    39,   158,     0,   518,   559,
     711,   560,   115,     0,   324,   642,   641,   169,     0,     0,
       0,     0,   118,   481,   564,   169,     0,     0,   528,   529,
     530,   533,   534,   543,     0,   516,   539,     0,   527,   547,
     515,   550,   552,   554,     0,   630,   564,     0,     0,     0,
       0,   152,    53,     0,   267,   128,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   139,     0,   694,   695,     0,
       0,   709,   522,     0,   516,   505,     0,   516,     0,   504,
     649,     0,   516,     0,     0,     0,   180,   688,   692,   693,
       0,   246,   627,   625,   270,   274,     0,    14,   246,   384,
       0,     0,   386,   379,   382,     0,   377,     0,     0,     0,
     169,   173,   686,   560,   157,   710,     0,     0,   184,     0,
       0,   599,   184,   184,   560,     0,   277,   169,     0,   593,
       0,   536,   515,   538,     0,   526,     0,     0,   516,   544,
     634,     0,    42,     0,   148,   134,     0,   125,    71,   138,
       0,     0,   141,     0,   146,   147,    42,   140,   706,   682,
       0,     0,   500,   515,   513,     0,   515,   354,   501,     0,
     360,   515,   644,     0,    42,   686,     0,   690,   122,     0,
       0,   249,   250,   251,   254,   253,   252,   244,     0,     0,
       0,   101,   183,   185,     0,   243,   247,     0,   246,     0,
     658,   105,   375,     0,     0,   348,   564,   169,     0,     0,
     367,   156,   711,     0,   160,   686,   246,   640,   598,   246,
     246,     0,   184,     0,   592,   542,   541,   535,     0,   537,
     515,   546,    42,   154,    49,    54,     0,   136,   142,    42,
     144,     0,   521,   520,   351,     0,   648,   647,     0,     0,
     367,   691,     0,     0,   123,   213,   211,   467,    24,     0,
     207,     0,   212,   223,     0,   221,   226,     0,   225,     0,
     224,     0,    90,   248,   187,     0,   189,     0,   245,   624,
     376,   374,   385,   383,   169,     0,   596,   687,     0,     0,
       0,   161,     0,     0,    97,   103,   107,   686,   246,   594,
       0,   549,     0,   150,     0,    42,   131,    72,   143,   708,
       0,     0,     0,     0,    86,     0,     0,     0,   197,   201,
       0,     0,   194,   435,   434,   431,   433,   432,   451,   453,
     452,   423,   413,   429,   428,   391,   400,   401,   403,   402,
     422,   406,   404,   405,   407,   408,   409,   410,   411,   412,
     414,   415,   416,   417,   418,   419,   421,   420,   392,   393,
     394,   396,   397,   399,   437,   438,   447,   446,   445,   444,
     443,   442,   430,   448,   439,   440,   441,   424,   425,   426,
     427,   449,   450,   454,   456,   455,   457,   458,   436,   460,
     459,   395,   462,   463,   398,   464,   461,   390,   218,   387,
       0,   195,   239,   240,   238,   231,     0,   232,   196,   257,
       0,     0,     0,     0,    90,     0,   595,     0,    42,     0,
     164,     0,   163,    42,     0,    99,   540,     0,    42,   129,
      55,     0,   519,   350,   646,    42,    42,   260,   124,     0,
       0,   215,   208,     0,     0,     0,   220,   222,     0,     0,
     227,   234,   235,   233,     0,     0,   186,     0,     0,     0,
       0,   597,     0,   370,   518,     0,   165,     0,   162,     0,
      42,   548,     0,     0,     0,     0,   198,    31,     0,   199,
     200,     0,     0,   214,   217,   388,   389,     0,   209,   236,
     237,   229,   230,   228,   258,   255,   190,   188,   259,     0,
     371,   517,     0,   342,     0,   167,    93,     0,     0,   132,
      84,   344,     0,   246,   216,   219,     0,   560,   192,     0,
     368,   366,   166,    95,   130,    88,   205,     0,   245,   256,
       0,   560,   369,     0,    87,    74,     0,     0,   204,   686,
       0,     0,     0,   203,     0,   686,     0,   202,   241,    42,
     191,     0,     0,     0,   193,     0,   242,    42,     0,    83
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    96,   625,   440,   144,   224,   225,
      98,    99,   100,   101,   102,   103,   267,   455,   456,   383,
     196,  1074,   389,  1006,  1295,   740,   741,  1305,   283,   145,
     457,   651,   791,   458,   473,   667,   424,   664,   459,   444,
     665,   285,   241,   258,   109,   653,   627,   611,   751,  1023,
     827,   710,  1200,  1077,   564,   716,   388,   572,   718,   926,
     559,   702,   705,   819,   777,   778,   467,   468,   229,   230,
     235,   861,   962,  1041,  1182,  1287,  1301,  1209,  1249,  1250,
    1251,  1029,  1030,  1031,  1210,  1216,  1258,  1034,  1035,  1039,
    1175,  1176,  1177,  1320,   963,   964,   965,   966,  1180,   967,
     110,   190,   384,   385,   111,   112,   113,   114,   115,   650,
     744,   448,   847,   449,   848,   116,   117,   118,   589,   119,
     120,  1059,  1234,   121,   445,  1051,   446,   764,   632,   876,
     873,  1168,  1169,   122,   123,   124,   184,   191,   270,   371,
     125,   592,   593,   126,   843,   365,   648,   844,   689,   801,
     803,   804,   805,   691,   907,   908,   692,   540,   356,   153,
     154,   127,   780,   340,   341,   641,   128,   185,   147,   130,
     131,   132,   133,   134,   135,   136,   502,   137,   187,   188,
     427,   176,   177,   505,   506,   851,   852,   250,   251,   619,
     138,   419,   139,   140,   216,   242,   278,   398,   729,   980,
     609,   575,   576,   577,   217,   218,   886
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -940
static const yytype_int16 yypact[] =
{
    -940,   120,  -940,  -940,  3517,  9712,  9712,   -46,  9712,  9712,
    9712,  -940,  9712,  9712,  9712,  9712,  9712,  9712,  9712,  9712,
    9712,  9712,  9712,  9712,  2824,  2824,  2983,  9712,  6395,   -27,
     132,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  9712,  -940,
     132,   169,   171,   193,   132,  7411,   310,  7588,  -940,  1368,
    7765,   -26,  9712,   901,   -12,   177,   249,   207,   202,   218,
     221,   224,  -940,   310,   229,   237,  -940,  -940,  -940,  -940,
    -940,   480,   367,  -940,  -940,   310,  7942,  -940,  -940,  -940,
    -940,  -940,  -940,   310,  -940,    11,   238,   310,  -940,  9712,
    -940,  -940,    18,   289,   336,   336,  -940,   384,   233,   -45,
    -940,   242,  -940,    33,  -940,   389,  -940,  -940,  -940,   868,
    -940,   251,   252,   272,  8105,  -940,  -940,   360,  -940,   392,
     399,  -940,    94,   291,   305,  -940,  -940,   580,    85,  1864,
     113,   300,   114,   119,   322,    30,  -940,    91,  -940,   436,
    -940,   407,   337,   368,  -940,   389, 11037,  2125, 11037,  9712,
   11037, 11037,  3680,   482,   310,  -940,  -940,   474,  -940,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  1939,   366,
    -940,   396,   411,   411,  2824, 10730,   362,   533,  -940,   403,
    1939,   366,   405,   412,   393,   124,  -940,   441,   113,  8119,
    -940,  -940,  9712,  6349,    43, 11037,  7234,  -940,  9712,  9712,
     310,  -940,  -940,  9521,   409,  -940,  9698,  1368,  1368,   406,
    -940,   413,   782,   566,  -940,   366,   576,  -940,   310,  -940,
    9875,  -940, 10584,   310,    44,  -940,     0,  -940,  1055,    46,
    -940,  -940,  -940,   579,   389,    47,  2824,  2824,  2824,   425,
     435,  -940,  -940,  2336,  2983,    17,   504,  -940,  9889,  2824,
     529,  -940,   310,  -940,   -33,   233,   428, 11037,  -940,  -940,
    -940,   524,   592,   438, 11037,   439,   467,  3694,  9712,   302,
     440,   456,   302,   265,   258,  -940,   310,  1368,   445,  8296,
    1368,  -940,  -940,   423,  -940,  -940,  -940,  -940,  -940,  -940,
    9712,  9712,  9712,  8473,  9712,  9712,  9712,  9712,  9712,  9712,
    9712,  9712,  9712,  9712,  9712,  9712,  9712,  9712,  9712,  9712,
    9712,  9712,  9712,  9712,  9712,  6395,  -940,  9712,  9712,  9712,
     620,   310,   310,   389,   868,  2688,  9712,  9712,  9712,  9712,
    9712,  9712,  9712,  9712,  9712,  9712,  9712,  -940,  -940,   281,
    -940,   125,  9712,  9712,  -940,  8296,  9712,  9712,    18,   127,
    2336,   455,  8650, 10625,  -940,   457,   618,  1939,   460,   -13,
     620,   411,  8827,  -940,  9004,  -940,   461,     8,  -940,    99,
    8296,  -940,   298,  -940,   134,  -940,  -940, 10689,  -940,  -940,
    9712,  -940,   551,  6526,   628,   465, 10930,   627,    56,    21,
    -940,  -940,  -940,  -940,  -940,  1368,   567,   478,   641,  -940,
    -940,  2300,  -940,  -940,  3871,  -940,    25,   901,  -940,   310,
    9712,   411,   -12,  -940,  2300,   481,   577,  -940,   411,    68,
      74,    12,   486,   856,   534,   491,   411,    77,   494,   756,
     310,  -940,  -940,   608,  2874,   -37,  -940,  -940,  -940,   233,
    -940,  -940,  -940,  -940,   552,   512,    20,   560,   676,   514,
    1368,    31,   626,    92,  -940,  -940,  -940,  -940,  -940,  -940,
   10168,  -940,  -940,  -940,  -940,    36,  2824,   521,   688, 11037,
     686,  -940,  -940,   583,  7574,  3506,  3680,  9712, 10996,  4033,
    4209,  3331,  4384,  4560,  4737,  4737,  4737,  4737,  2212,  2212,
    2212,  2212,  1269,  1269,   502,   502,   502,   474,   474,   474,
    -940, 11037,   527,   528, 10772,   532,   699,   -40,   539,   127,
    -940,  -940,   310,  -940,  2026,  9712,  -940,  3680,  3680,  3680,
    3680,  3680,  3680,  3680,  3680,  3680,  3680,  3680,  3680,  9712,
     -40,   540,   535, 10209,   542,   538, 10250,    78,  -940,  1769,
    -940,   310,  -940,   438,    31,   366,  2824, 11037,  2824, 10834,
      40,   136,  -940,   545,  9712,  -940,  -940,  -940,  6172,    69,
   11037,   132,  -940,  -940,  -940,  9712,   511,  2300,   310,  6703,
     546,   548,  -940,    54,   589,  -940,   709,   554,   816,  1368,
    2300,  2300,   553,    22,   587,   562,    14,  -940,   590,  -940,
     565,  -940,  -940,  -940,   649,   310,  -940,  -940, 10291,  -940,
    -940,  -940,   727,  2824,   571,  -940,  -940,  -940,   108,   593,
    1633,   588,  2336,  2389,   739,  -940,  -940,  -940,  -940,   585,
    -940,  9712,  -940,  -940,  3163,  -940,  1633,   591,  -940,  -940,
    -940,  -940,   747,  9712,   695,  -940,  -940,   603,   648,  -940,
     159,  -940,  -940,  1368,  -940,   411,  -940,  9181,  -940,  2300,
      93,   607,  1633,   552,  3857,  9712,  -940,  -940,  9712,  -940,
    9712,  -940,   609,  8296,   534,   552,  -940,   583,  6395,   411,
   10335,   610,  8296,  -940,  -940,   200,  -940,  -940,   769,   408,
     408,  1769,  -940,  -940,  -940,   611,    28,   613,  -940,  -940,
    -940,   777,   617,   457,   411,   411,  9358,  -940,   222,  -940,
    -940, 10376,   263,   132,  7234,  -940,   631,  4048,   632,  2824,
     668,   411,  -940,   779,  -940,  -940,  -940,  -940,   220,  -940,
      10,  1368,  -940,  1368,   567,  -940,  -940,  -940,   790,   637,
     639,  -940,  -940,  2300,   674,   310,   735,   310,  2300,   645,
    -940,   659,  -940,  -940,    93,  2300,   411,  -940,   856,  1032,
    -940,   811,  -940,  -940,    80,   652,   411,  9535,  -940,  2253,
    -940,  3340,   811,  -940,   164,   131, 11037,   702,  -940,  9712,
     -40,   656,  -940,  2824, 11037,  -940,  -940,   657,   823,  -940,
    1368,    93,  -940,   665,  3857, 11037, 10875,  8296,   669,   670,
     672,   677,   552,   393,   678,  8296,   671,  9712,  -940,  -940,
    -940,  -940,  -940,   728,   679,   841,  1769,   714,  -940,   775,
    1769,  -940,  -940,  -940,  2824, 11037,  -940,   132,   833,   792,
    7234,  -940,  -940,   690,  9712,   411,   511,   692,  2300,  4225,
     234,   693,  9712,    49,   196,  -940,   703,  -940,  -940,  1256,
     842,  -940,   748,   700,   866,  -940,   753,   869,   705,  -940,
     757,   704,   874,  1633,   711,   717,  -940,  -940,   882,   856,
    1633,   638,  -940,  2336,  -940,  3680,   719,  -940,   969,  -940,
      86,  9712,  -940,  -940,  -940,  9712,  -940,  9712, 10417,   722,
    8296,   411,   873,   100,  -940,  -940,   277,   723,  -940,  9712,
     729,  -940,  -940,  -940,    93,   730,  -940,  8296,   742,  -940,
    1769,  -940,  1769,  -940,   751,  -940,   807,   761,   916,  -940,
     411,   911,  -940,   770,  -940,  -940,   773,  -940,  -940,  -940,
     786,   789,  -940, 10543,  -940,  -940,  -940,  -940,  -940,  -940,
    1368,  2300,  -940,  2300,  -940,  2300,   862,  -940,  -940,  2300,
    -940,  2300,  -940,   876,  -940,   873,   856,  -940,  -940,  1368,
    1633,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  1517,   813,
     351,  -940,  -940,  -940,   407,  1279,  -940,    50,  1111,    81,
    -940,  -940,   820, 10458, 10502, 11037,   801,  8296,   803,  1368,
     877,  -940,  1368,   904,   965,   873,  1454, 11037,  -940,  1470,
    1574,   812,  -940,   814,  -940,  -940,   867,  -940,  1769,  -940,
     905,  -940,  -940,  6172,  -940,  -940,  6880,  -940,  -940,  -940,
    6172,   819,  -940,   870,  -940,   871,  -940,   879,   824,  4402,
     877,  -940,   978,    38,  -940,  -940,  -940,    57,   825,    59,
    -940, 10017,  -940,  -940,    61,  -940,  -940,   768,  -940,   839,
    -940,   923,   389,  -940,  -940,  1368,  -940,   407,  1111,  -940,
    -940,  -940,  -940,  -940,  8296,   852,  -940,  -940,   843,   851,
     284,  1013,  2300,   855,  -940,  -940,  -940,   873,  1657,  -940,
    1769,  -940,   907,  6172,  7057,  -940,  -940,  -940,  6172,  -940,
    2300,  2300,  2300,   864,  -940,   875,  2300,  1633,  -940,  -940,
    1834,  1517,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,   105,  -940,
     813,  -940,  -940,  -940,  -940,  -940,    52,   410,  -940,  1026,
      62,   310,   923,  1027,   389,   885,  -940,   288,  -940,   971,
    1033,  2300,  -940,  -940,   884,  -940,  -940,  1769,  -940,  -940,
    -940,  4579,  -940,  -940,  -940,  -940,  -940,  -940,  -940,   802,
      35,  -940,  -940,  2300, 10017, 10017,   996,  -940,   768,   768,
     468,  -940,  -940,  -940,  2300,   985,  -940,   889,    63,  2300,
     310,  -940,   987,  -940,  1056,  4756,  1050,  2300,  -940,  4933,
    -940,  -940,  5110,   893,  5287,  5464,  -940,   982,   949,  -940,
    -940,  1000,  1834,  -940,  -940,  -940,  -940,   938,  -940,  1063,
    -940,  -940,  -940,  -940,  -940,  1083,  -940,  -940,  -940,   927,
    -940,   292,   931,  -940,  2300,  -940,  -940,  5641,   926,  -940,
    -940,  -940,   310,  1111,  -940,  -940,  2300,    93,  -940,  1031,
    -940,  -940,  -940,  -940,  -940,    -8,   952,   310,   976,  -940,
     937,    93,  -940,   941,  -940,  -940,  1633,   940,  -940,   873,
     942,  1633,    65,  -940,   -60,   873,  1041,  -940,  -940,  -940,
    -940,   -60,   945,  5818,  -940,   946,  -940,  -940,  5995,  -940
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -940,  -940,  -940,  -401,  -940,  -940,  -940,    -4,  -940,   715,
     -22,   887,  1295,  -940,   -50,  -940,  -218,  -940,    -3,  -940,
    -940,  -940,  -940,  -940,  -940,  -174,  -940,  -940,  -141,    15,
       4,  -940,  -940,     5,  -940,  -940,  -940,  -940,     6,  -940,
    -940,   805,   806,   809,  1006,   470,  -546,   471,   508,  -168,
    -940,   313,  -940,  -940,  -940,  -940,  -940,  -940,  -475,   217,
    -940,  -940,  -940,  -940,  -732,  -940,  -330,  -940,  -940,   732,
    -940,  -715,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,    51,  -940,  -940,  -940,  -940,  -940,   -25,  -940,
     186,  -870,  -940,  -169,  -940,  -938,  -936,  -939,   -35,  -940,
     -54,   -23,  1115,  -528,  -309,  -940,  -940,  2306,  1066,  -940,
    -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,   137,  -940,   394,  -940,  -940,  -940,  -940,  -940,  -940,
    -940,  -940,  -813,  -940,  1126,  1244,  -290,  -940,  -940,   363,
    -327,   718,  -940,  -940,  -940,  -338,  -758,  -940,  -940,   485,
    -526,   354,  -940,  -940,  -940,  -940,  -940,   469,  -940,  -940,
    -940,  -573,   278,  -160,  -158,  -108,  -940,  -940,    32,  -940,
    -940,  -940,  -940,   -14,  -113,  -940,  -219,  -940,  -940,  -940,
    -339,   924,  -940,  -940,  -940,  -940,  -940,   492,   563,  -940,
    -940,   932,  -940,  -940,  -275,   -81,  -147,  -244,  -940,  -922,
    -710,   446,  -940,  -940,   422,  -110,   190
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -680
static const yytype_int16 yytable[] =
{
      97,   104,   259,   404,   351,   470,   262,   193,   106,   107,
     108,   537,   855,   690,   186,   534,   516,   197,   542,   105,
     884,   201,   344,  1020,   349,   500,  1043,   373,   286,   374,
    1047,   226,  1048,   465,   368,   263,   129,   708,   857,   624,
     553,   280,   204,  1252,   643,   213,  1087,   570,   868,   887,
     254,   380,   407,   255,   412,   416,   172,   173,  1045,   240,
    1218,   227,   721,  1063,   568,  -210,   629,  1091,   400,  1170,
    1225,  1225,   234,  1087,   591,   924,   603,   779,   429,   240,
     375,  1219,   603,   240,   339,   613,   613,   591,   613,   613,
     409,   346,   342,   415,  1303,  -679,   720,   393,   394,   503,
     734,   342,   399,   832,   833,   240,   807,   783,   339,  1043,
     635,   276,  1318,  1319,   358,   595,   748,   277,  1213,   790,
       3,   266,   277,   532,   149,  -679,   366,   535,  -679,   339,
    -679,  1214,   972,   339,    11,   703,   704,   623,   430,   441,
     442,    11,   323,   189,   276,  1194,  -584,   221,  1215,   947,
     355,   -85,   638,  -588,  -679,  -581,   234,   260,   453,   247,
     558,   372,   991,   228,    35,   808,   630,   399,   661,   276,
    -470,   779,    35,   986,   342,  -582,   277,   989,   990,   551,
    -583,   631,   512,   606,   835,  -618,  -585,  -679,   346,    97,
     379,   268,    97,   382,   571,  -619,   387,  -621,   749,   644,
     359,   406,   472,   347,   343,   281,   361,   509,   779,  1253,
    1088,  1089,   367,   343,   402,   381,   408,   722,   413,   417,
    -586,   925,  1046,  1220,   761,   129,   509,   569,   129,  -210,
     439,  1092,   260,  1171,  1226,  1267,  1021,  1317,   725,   604,
     591,   259,   830,   286,   834,   605,   895,   509,   614,   678,
     776,   862,  1049,   591,   591,  -590,   509,   981,  -584,   509,
     411,  -587,  -591,    97,  -159,  -588,   348,  -581,   418,   418,
     421,  -517,   464,   754,    92,   426,   213,  1068,   874,   240,
     233,   435,   105,  -620,   909,   573,   343,  -582,   968,   832,
     833,   639,  -583,   640,   231,   968,   916,  -618,  -585,   129,
     347,   186,   192,   543,   875,    48,   508,  -619,   983,  -621,
     779,   869,   831,   832,   833,  1189,   507,   240,   240,  1232,
     240,   779,   591,  1289,   870,   531,   921,   832,   833,   817,
     818,   245,  -586,   788,   728,   530,   436,   871,   245,   198,
     637,   199,   796,   436,   707,  1297,   508,  1298,  1259,  1260,
     984,   588,    33,   811,    35,   552,   545,  1190,   556,  1043,
     662,  1233,   245,   200,   588,  1290,   232,   269,   555,    33,
     927,    35,   236,  -587,   995,   245,   996,   276,   793,    97,
     563,    33,   426,   671,   639,   226,   640,  1314,   237,   359,
     697,   238,   698,  1321,   239,  -620,   248,   249,   662,   243,
      97,  1255,  1256,   248,   249,   597,   591,   244,   261,   245,
     275,   591,   279,   968,   272,   129,   968,   968,   591,   105,
     282,  -363,    33,   287,   288,   618,   620,   248,   249,    78,
      79,   666,    80,    81,    82,  1221,   129,   463,    33,   368,
     248,   249,   699,   462,   289,   320,    78,    79,   864,    80,
      81,    82,  1222,   317,   529,  1223,    92,   890,    78,    79,
     318,    80,    81,    82,   319,   898,  1036,   903,   399,   730,
     345,   554,  1071,    92,   248,   249,  1272,   798,   799,    33,
     352,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,  -589,  1261,    33,   968,   829,  -364,   645,    78,
      79,   591,    80,    81,    82,    48,   934,   350,   240,   937,
    1262,   252,   252,  1263,   942,    78,    79,   588,    80,    81,
      82,  1037,   354,   315,   969,   509,   337,   338,   277,   245,
     588,   588,   339,   772,   436,   688,   360,   693,   706,   363,
     253,   364,   709,  -469,  1196,  -468,   669,   312,   313,   314,
     978,   315,   369,   245,    97,  1300,    78,    79,   246,    80,
      81,    82,   372,   370,   713,    97,   715,   993,   395,  1310,
    1001,    78,    79,   105,    80,    81,    82,   245,   694,  -674,
     695,   391,    33,   396,    35,   271,   273,   274,   339,   401,
     129,   742,   414,   471,   248,   249,   422,   423,   711,   588,
     443,   129,   245,   447,   591,   450,   591,   436,   591,   451,
     452,   836,   591,   837,   591,   -37,   461,   247,   248,   249,
      97,   104,   167,   879,   771,   471,   541,   539,   106,   107,
     108,   544,   550,   561,   770,   746,   380,   565,   453,   105,
     567,   431,   248,   249,   426,   756,   574,  1055,   578,   579,
     602,   601,   143,   610,   186,    75,   129,    77,   607,    78,
      79,   612,    80,    81,    82,   615,   437,   248,   249,   621,
     885,  1241,   626,   628,  1022,   800,   800,   688,    48,    11,
     820,   168,   633,   588,   634,   636,    92,  -365,   588,    55,
      56,    33,   646,    35,  1003,   588,   647,    62,   321,   649,
      97,   821,   652,    97,   656,   657,   659,   660,  1010,   663,
     672,   723,   673,   675,   779,   676,   700,   724,   717,    33,
     719,    35,   105,   733,  1185,   726,  1019,   735,   779,   772,
     737,   845,   736,   849,   322,   591,   129,   949,   738,   129,
     745,   825,   950,   747,   951,   952,   953,   954,   955,   956,
     957,   739,   757,   591,   591,   591,   750,    97,   104,   591,
     765,   753,   758,  1211,   763,   106,   107,   108,    78,    79,
    1183,    80,    81,    82,   767,   768,   105,   781,   588,   787,
     795,   806,   797,   809,  1073,   810,   958,   959,   812,   960,
     826,  1078,   828,   129,   911,    92,    78,    79,   839,    80,
      81,    82,   688,   822,   824,   881,   688,   207,   840,   432,
     841,   431,   961,   438,   846,   853,    97,   914,   854,   860,
    1011,   769,   863,    92,   877,    97,   880,    33,   882,   616,
     617,   883,   432,   208,   438,   432,   438,   438,   888,    33,
     891,   207,   899,   892,   105,   893,   910,   894,   897,   902,
     900,   905,   129,    33,   906,  1044,   901,  1201,   711,   912,
     913,   129,   915,   918,   591,   922,   928,   208,   930,  1057,
     931,   932,   885,    33,   933,   935,   938,   936,   940,   939,
     397,   588,   941,   588,   944,   588,   591,    33,   945,   588,
     946,   588,   977,   970,   985,   426,   688,   591,   688,   979,
     988,  1181,   591,   992,    78,    79,   209,    80,    81,    82,
     591,   169,   169,   994,  1172,   181,    78,    79,  1173,    80,
      81,    82,   997,   143,  1000,  1284,    75,    33,   210,   998,
      78,    79,   999,    80,    81,    82,   214,  1002,  1037,    33,
     209,  1015,  1004,   143,  1005,   213,    75,   591,   211,  1018,
      78,    79,   212,    80,    81,    82,  1038,   143,  1007,   591,
      75,  1008,   210,  1033,    78,    79,  1050,    80,    81,    82,
    1235,  1054,    33,   727,  1056,  1239,  1246,  1061,  1062,  1042,
    1242,  1058,   211,  1067,  1072,  1069,   212,  1244,  1245,  1070,
    1079,  1086,  1080,  1081,   688,  1083,  1179,   143,  1090,    97,
      75,  1082,    97,  1076,    78,    79,    97,    80,    81,    82,
      11,  1178,   588,  1187,   284,    97,    78,    79,   105,    80,
      81,    82,  1277,  1186,  1188,   105,  1191,  1167,  1193,  1197,
     588,   588,   588,  1174,   105,   129,   588,  1205,   129,  1224,
    1229,   213,   129,  1230,  1236,   223,  1237,  1257,  1206,    78,
      79,   129,    80,    81,    82,   169,  1231,  1240,  1265,  1266,
    1270,   169,  1184,  1274,  1271,  1279,   688,   169,   949,    97,
      97,  1199,  -206,   950,    97,   951,   952,   953,   954,   955,
     956,   957,   951,   952,   953,   954,   955,   956,   105,  1282,
    1283,  1285,  1219,   105,   214,   214,  1286,  1288,  1294,   214,
    1227,  1323,  1291,    33,  1302,   129,   129,  1306,  1309,  1328,
     129,  1311,  1313,  1315,  1322,   169,  1325,   958,   959,  1327,
     960,  1304,   596,   169,   169,   169,    33,   510,    35,   513,
     169,   511,   600,   324,   762,   789,   169,   792,  1312,   917,
    1009,   588,  1212,   971,   599,  1217,  1040,  1228,  1308,  1269,
     170,   170,  1324,   194,   182,   265,   896,  1085,   872,  1248,
     904,   982,   813,   588,   214,   802,   167,   214,   428,   420,
     838,   859,  1060,   143,   588,     0,    75,   240,     0,   588,
      78,    79,     0,    80,    81,    82,     0,   588,     0,     0,
       0,     0,     0,   688,     0,     0,   143,    97,   211,    75,
       0,    77,   181,    78,    79,  1247,    80,    81,    82,     0,
    1167,  1167,     0,     0,  1174,  1174,   105,   951,   952,   953,
     954,   955,   956,     0,   588,   168,   240,     0,   410,     0,
      92,    97,     0,   129,     0,    97,   588,   169,    97,     0,
      97,    97,     0,     0,   169,     0,     0,     0,     0,     0,
     105,     0,     0,     0,   105,     0,     0,   105,     0,   105,
     105,     0,     0,     0,     0,     0,     0,   129,   171,   171,
       0,   129,   183,    97,   129,     0,   129,   129,  1296,     0,
       0,   207,   214,     0,     0,   712,     0,     0,   586,     0,
       0,     0,   105,  1307,   170,     0,     0,     0,   731,   732,
     170,   586,     0,     0,   207,     0,   170,   208,     0,   129,
     214,   309,   310,   311,   312,   313,   314,     0,   315,    97,
       0,     0,     0,     0,    97,     0,     0,    33,     0,     0,
     208,     0,     0,     0,     0,     0,     0,   214,   105,     0,
       0,     0,     0,   105,   215,     0,     0,     0,     0,     0,
      33,     0,     0,   169,   170,   129,     0,     0,     0,     0,
     129,     0,   170,   170,   170,     0,     0,   775,     0,   170,
       0,     0,     0,     0,     0,   170,     0,  -245,     0,     0,
     209,     0,     0,     0,     0,   951,   952,   953,   954,   955,
     956,     0,     0,   207,     0,     0,     0,   143,     0,     0,
      75,   169,   210,   209,    78,    79,     0,    80,    81,    82,
       0,     0,   171,   929,     0,     0,     0,     0,   171,   208,
     143,     0,   211,    75,   171,   210,   212,    78,    79,     0,
      80,    81,    82,   169,     0,   169,     0,     0,     0,    33,
       0,   182,     0,     0,     0,   211,     0,     0,     0,   212,
       0,   842,     0,   169,   586,     0,   850,     0,     0,     0,
       0,     0,     0,   856,     0,   214,   214,   586,   586,     0,
       0,     0,   171,     0,     0,     0,   170,     0,     0,     0,
     171,   171,   171,   170,     0,     0,     0,   171,     0,     0,
     169,     0,   209,   171,     0,    11,     0,   181,     0,   169,
     169,     0,   215,   215,     0,     0,     0,   215,     0,   143,
       0,    11,    75,   181,   210,     0,    78,    79,     0,    80,
      81,    82,     0,     0,     0,     0,     0,   590,     0,     0,
     214,     0,     0,     0,   211,     0,   586,     0,   212,   181,
     590,     0,     0,     0,     0,     0,   919,     0,     0,     0,
       0,     0,     0,   949,     0,   181,     0,     0,   950,   183,
     951,   952,   953,   954,   955,   956,   957,     0,     0,   949,
       0,     0,   215,     0,   950,   215,   951,   952,   953,   954,
     955,   956,   957,     0,     0,     0,     0,     0,    33,     0,
       0,     0,   170,     0,   171,     0,   169,     0,     0,     0,
       0,   171,   958,   959,     0,   960,     0,     0,   214,     0,
     214,     0,     0,     0,     0,    11,     0,     0,   958,   959,
     586,   960,     0,     0,     0,   586,     0,     0,  1064,  1025,
       0,     0,   586,     0,     0,   214,   214,     0,     0,     0,
     170,  1026,     0,     0,  1065,     0,     0,     0,     0,  1012,
       0,  1013,     0,  1014,     0,     0,     0,  1016,   143,  1017,
     169,    75,     0,  1027,     0,    78,    79,   214,    80,  1028,
      82,     0,   170,   949,   170,     0,     0,     0,   950,     0,
     951,   952,   953,   954,   955,   956,   957,     0,     0,     0,
     215,     0,   170,   590,     0,     0,   587,     0,    11,     0,
       0,   169,     0,     0,    33,     0,   590,   590,     0,   587,
     171,     0,     0,   169,     0,   586,     0,     0,   608,     0,
       0,     0,   958,   959,     0,   960,   214,     0,     0,   170,
       0,     0,     0,     0,     0,     0,   752,     0,   170,   170,
     181,     0,     0,     0,     0,   215,   214,   181,  1066,     0,
     169,     0,   752,     0,     0,     0,   949,     0,   171,     0,
       0,   950,     0,   951,   952,   953,   954,   955,   956,   957,
       0,     0,     0,     0,   143,   590,     0,    75,   782,    77,
    1192,    78,    79,     0,    80,    81,    82,     0,     0,     0,
     171,     0,   171,     0,   182,     0,     0,     0,  1202,  1203,
    1204,     0,     0,     0,  1207,   958,   959,     0,   960,     0,
     171,   679,   680,     0,     0,     0,     0,   214,   586,     0,
     586,     0,   586,     0,     0,     0,   586,     0,   586,     0,
     681,  1195,     0,   214,     0,   170,   214,   181,   682,   683,
      33,     0,     0,     0,     0,   181,     0,   171,   684,     0,
       0,     0,   214,     0,     0,     0,   171,   171,     0,   590,
       0,     0,   587,     0,   590,     0,   214,     0,     0,   214,
       0,   590,     0,   215,   215,   587,   587,   325,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,     0,
       0,     0,     0,   685,     0,     0,     0,     0,     0,   170,
       0,     0,     0,    31,    32,   686,     0,     0,     0,  1238,
       0,     0,   183,    37,     0,     0,     0,    78,    79,     0,
      80,    81,    82,   337,   338,     0,     0,     0,     0,     0,
       0,  1254,   214,     0,     0,   687,     0,     0,   215,     0,
     170,     0,  1264,     0,   587,     0,     0,  1268,     0,   586,
       0,     0,   170,   171,   590,  1275,     0,     0,     0,    66,
      67,    68,    69,    70,     0,     0,     0,   586,   586,   586,
     583,     0,     0,   586,   181,     0,    73,    74,   181,   943,
       0,     0,     0,     0,     0,   339,   948,     0,     0,   170,
      84,     0,  1292,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    28,    88,  1299,     0,     0,     0,     0,     0,
      33,     0,    35,     0,     0,     0,   215,   171,   215,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   587,     0,
       0,     0,     0,   587,     0,     0,     0,     0,     0,     0,
     587,     0,     0,   608,   858,     0,     0,     0,     0,     0,
     167,     0,     0,     0,     0,     0,     0,   590,   171,   590,
       0,   590,     0,     0,     0,   590,     0,   590,     0,     0,
     171,     0,     0,     0,     0,   215,  1024,     0,   586,     0,
     143,     0,     0,    75,  1032,    77,     0,    78,    79,   668,
      80,    81,    82,     0,     0,     0,   181,    33,     0,    35,
     586,     0,     0,     0,     0,     0,     0,   171,     0,   357,
       0,   586,     0,     0,    92,     0,   586,     0,     0,     0,
       0,     0,     0,   587,   586,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   215,     0,     0,   167,   352,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
       0,     0,     0,     0,   608,     0,     0,     0,     0,     0,
       0,   586,     0,     0,     0,     0,     0,   143,     0,     0,
      75,     0,    77,   586,    78,    79,     0,    80,    81,    82,
       0,     0,     0,     0,   337,   338,     0,     0,   590,     0,
       0,     0,     0,   181,     0,     0,   168,     0,   181,     0,
       0,    92,     0,     0,     0,     0,   590,   590,   590,     0,
       0,     0,   590,  1208,     0,     0,     0,  1032,     0,     0,
       0,     0,     0,     0,     0,   215,   587,     0,   587,     0,
     587,     0,     0,     0,   587,     0,   587,     0,     0,     0,
       0,   608,     0,     0,   215,     0,   339,     0,  -680,  -680,
    -680,  -680,   307,   308,   309,   310,   311,   312,   313,   314,
     215,   315,   290,   291,   292,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   215,     0,     0,   215,   293,     0,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,     0,   315,     0,     0,     0,     0,     0,     0,     0,
       0,   146,   148,     0,   150,   151,   152,   590,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
       0,     0,   175,   178,     0,     0,     0,     0,     0,   590,
     215,     0,   580,   581,   195,     0,     0,     0,     0,     0,
     590,   203,     0,   206,     0,   590,   220,   587,   222,     0,
       0,     0,     0,   590,     0,     0,     0,     0,     0,    31,
      32,    33,     0,     0,     0,   587,   587,   587,     0,    37,
       0,   587,   257,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   264,     0,     0,     0,     0,
     590,     0,     0,     0,     0,     0,     0,    33,     0,    35,
       0,     0,   590,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   582,    66,    67,    68,    69,    70,
     866,     0,  1024,     0,     0,     0,   583,  1316,     0,     0,
       0,   143,    73,    74,    75,     0,   584,   167,    78,    79,
       0,    80,    81,    82,     0,   353,    84,     0,     0,   425,
      33,     0,    35,     0,     0,     0,   585,     0,     0,    88,
       0,     0,     0,     0,     0,     0,     0,   143,     0,     0,
      75,     0,    77,     0,    78,    79,   587,    80,    81,    82,
       0,     0,     0,     0,     0,   377,     0,     0,   377,     0,
     167,     0,     0,     0,   195,   386,   168,     0,   587,     0,
       0,    92,   755,     0,     0,     0,     0,     0,     0,   587,
       0,     0,     0,     0,   587,     0,     0,     0,     0,     0,
     143,     0,   587,    75,     0,    77,     0,    78,    79,     0,
      80,    81,    82,     0,     0,     0,     0,     0,     0,     0,
     175,     0,     0,     0,   434,     0,     0,     0,     0,   168,
       0,     0,     0,     0,    92,     0,     0,     0,     0,   587,
       0,     0,     0,     0,   460,     0,     0,     0,     0,     0,
       0,   587,     0,     0,     0,   469,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   474,   475,   476,   478,
     479,   480,   481,   482,   483,   484,   485,   486,   487,   488,
     489,   490,   491,   492,   493,   494,   495,   496,   497,   498,
     499,     0,     0,   501,   501,   504,     0,     0,     0,     0,
       0,   517,   518,   519,   520,   521,   522,   523,   524,   525,
     526,   527,   528,     0,     0,     0,     0,     0,   501,   533,
       0,   469,   501,   536,     0,     0,     0,     0,   517,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   547,     0,
     549,     0,     0,     0,     0,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   560,     0,     0,     0,
       0,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   598,     0,     0,   514,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   654,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,    59,    60,     0,     0,     0,     0,     0,     0,
       0,   142,    65,    66,    67,    68,    69,    70,     0,     0,
       0,   257,     0,     0,    71,     0,     0,     0,     0,   143,
      73,    74,    75,   515,    77,   670,    78,    79,     0,    80,
      81,    82,     0,     0,    84,     0,     0,     0,    85,     0,
       0,     0,     0,     0,    86,     0,     0,    88,    89,     0,
     701,     0,     0,    92,    93,     0,    94,    95,     0,     0,
       0,   195,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   290,   291,   292,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,     0,    35,     0,   293,
       0,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,     0,   315,     0,     0,     0,   759,     0,     0,
       0,     0,     0,     0,     0,   167,     0,     0,     0,   766,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   774,     0,     0,     0,     0,     0,     0,
       0,   784,     0,     0,   785,   143,   786,     0,    75,   469,
      77,     0,    78,    79,     0,    80,    81,    82,   469,     0,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,     0,     0,     0,   168,    10,     0,     0,     0,    92,
       0,     0,   815,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   174,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,   622,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,   865,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   878,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   469,   141,     0,     0,    59,    60,     0,
       0,   469,     0,   865,     0,     0,   142,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   143,    73,    74,    75,     0,    77,
     195,    78,    79,     0,    80,    81,    82,     0,   923,    84,
       0,     0,     0,    85,     0,     0,     0,     0,     0,    86,
       0,     0,    88,    89,     0,     0,     0,     0,    92,    93,
       0,    94,    95,     0,     0,     0,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,   973,     0,     0,
       0,   974,     0,   975,     0,     0,   469,     0,     0,     0,
       0,     0,     0,     0,     0,   987,     0,     0,     0,     0,
       0,     0,     0,   469,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,    49,    50,    51,     0,    52,    53,    54,     0,
       0,     0,    55,    56,    57,     0,    58,    59,    60,    61,
      62,    63,     0,   469,     0,     0,    64,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,    72,    73,    74,    75,    76,    77,
       0,    78,    79,     0,    80,    81,    82,    83,     0,    84,
       0,     0,     0,    85,     0,     0,     0,     0,     0,    86,
      87,     0,    88,    89,     0,    90,    91,   760,    92,    93,
       0,    94,    95,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
     469,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,     0,
     315,    11,    12,    13,     0,     0,     0,     0,    14,     0,
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
      85,     0,     0,     0,     0,     0,    86,    87,     0,    88,
      89,     0,    90,    91,   867,    92,    93,   292,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,   293,     0,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,     0,   315,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,    49,    50,    51,     0,
      52,    53,    54,     0,     0,     0,    55,    56,    57,     0,
      58,    59,    60,    61,    62,    63,     0,     0,     0,     0,
      64,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,     0,    78,    79,     0,    80,    81,
      82,    83,     0,    84,     0,     0,     0,    85,     0,     0,
       0,     0,     0,    86,    87,     0,    88,    89,     0,    90,
      91,     0,    92,    93,     0,    94,    95,     5,     6,     7,
       8,     9,     0,     0,     0,   293,    10,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,     0,   315,
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
      71,     0,     0,     0,     0,   143,    73,    74,    75,    76,
      77,     0,    78,    79,     0,    80,    81,    82,    83,     0,
      84,     0,     0,     0,    85,     0,     0,     0,     0,     0,
      86,     0,     0,    88,    89,     0,    90,    91,   454,    92,
      93,     0,    94,    95,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,     0,   315,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,     0,     0,     0,
      55,    56,    57,     0,    58,    59,    60,     0,    62,    63,
       0,     0,     0,     0,    64,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   143,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,    83,     0,    84,     0,     0,
       0,    85,     0,     0,     0,     0,     0,    86,     0,     0,
      88,    89,     0,    90,    91,   594,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,     0,   315,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,   823,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,     0,     0,     0,    55,    56,    57,
       0,    58,    59,    60,     0,    62,    63,     0,     0,     0,
       0,    64,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   143,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,    83,     0,    84,     0,     0,     0,    85,     0,
       0,     0,     0,     0,    86,     0,     0,    88,    89,     0,
      90,    91,     0,    92,    93,     0,    94,    95,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,     0,   315,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,   920,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,     0,     0,     0,    55,    56,    57,     0,    58,    59,
      60,     0,    62,    63,     0,     0,     0,     0,    64,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   143,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,    83,
       0,    84,     0,     0,     0,    85,     0,     0,     0,     0,
       0,    86,     0,     0,    88,    89,     0,    90,    91,     0,
      92,    93,     0,    94,    95,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,     0,   315,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   143,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,    83,     0,    84,     0,
       0,     0,    85,     0,     0,     0,     0,     0,    86,     0,
       0,    88,    89,     0,    90,    91,  1084,    92,    93,     0,
      94,    95,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,     0,   315,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,  1243,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,     0,     0,     0,    55,    56,
      57,     0,    58,    59,    60,     0,    62,    63,     0,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,    83,     0,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,    90,    91,     0,    92,    93,     0,    94,    95,     5,
       6,     7,     8,     9,     0,     0,     0,     0,    10,  -680,
    -680,  -680,  -680,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,     0,   315,     0,     0,     0,
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
       0,     0,    71,     0,     0,     0,     0,   143,    73,    74,
      75,    76,    77,     0,    78,    79,     0,    80,    81,    82,
      83,     0,    84,     0,     0,     0,    85,     0,     0,     0,
       0,     0,    86,     0,     0,    88,    89,     0,    90,    91,
    1273,    92,    93,     0,    94,    95,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,     0,
       0,     0,    55,    56,    57,     0,    58,    59,    60,     0,
      62,    63,     0,     0,     0,     0,    64,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   143,    73,    74,    75,    76,    77,
       0,    78,    79,     0,    80,    81,    82,    83,     0,    84,
       0,     0,     0,    85,     0,     0,     0,     0,     0,    86,
       0,     0,    88,    89,     0,    90,    91,  1276,    92,    93,
       0,    94,    95,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,  1278,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,     0,     0,     0,    55,
      56,    57,     0,    58,    59,    60,     0,    62,    63,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   143,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,    83,     0,    84,     0,     0,     0,
      85,     0,     0,     0,     0,     0,    86,     0,     0,    88,
      89,     0,    90,    91,     0,    92,    93,     0,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,     0,     0,     0,    55,    56,    57,     0,
      58,    59,    60,     0,    62,    63,     0,     0,     0,     0,
      64,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,   143,    73,
      74,    75,    76,    77,     0,    78,    79,     0,    80,    81,
      82,    83,     0,    84,     0,     0,     0,    85,     0,     0,
       0,     0,     0,    86,     0,     0,    88,    89,     0,    90,
      91,  1280,    92,    93,     0,    94,    95,     5,     6,     7,
       8,     9,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      71,     0,     0,     0,     0,   143,    73,    74,    75,    76,
      77,     0,    78,    79,     0,    80,    81,    82,    83,     0,
      84,     0,     0,     0,    85,     0,     0,     0,     0,     0,
      86,     0,     0,    88,    89,     0,    90,    91,  1281,    92,
      93,     0,    94,    95,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,     0,     0,     0,
      55,    56,    57,     0,    58,    59,    60,     0,    62,    63,
       0,     0,     0,     0,    64,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   143,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,    83,     0,    84,     0,     0,
       0,    85,     0,     0,     0,     0,     0,    86,     0,     0,
      88,    89,     0,    90,    91,  1293,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    71,     0,     0,     0,     0,   143,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,    83,     0,    84,     0,     0,     0,    85,     0,
       0,     0,     0,     0,    86,     0,     0,    88,    89,     0,
      90,    91,  1326,    92,    93,     0,    94,    95,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,     0,     0,     0,    55,    56,    57,     0,    58,    59,
      60,     0,    62,    63,     0,     0,     0,     0,    64,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   143,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,    83,
       0,    84,     0,     0,     0,    85,     0,     0,     0,     0,
       0,    86,     0,     0,    88,    89,     0,    90,    91,  1329,
      92,    93,     0,    94,    95,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   143,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,    83,     0,    84,     0,
       0,     0,    85,     0,     0,     0,     0,     0,    86,     0,
       0,    88,    89,     0,    90,    91,     0,    92,    93,     0,
      94,    95,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   378,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,     0,     0,     0,     0,     0,
      57,     0,    58,    59,    60,     0,    33,     0,    35,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,     0,     0,    84,   179,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,    90,    91,     0,    92,    93,     0,    94,    95,     5,
       6,     7,     8,     9,     0,     0,   143,     0,    10,    75,
       0,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,     0,   562,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   180,     0,     0,    12,    13,
      92,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,     0,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,     0,     0,     0,     0,     0,    57,     0,    58,
      59,    60,     0,     0,     0,     0,     0,     0,     0,    64,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   143,    73,    74,
      75,    76,    77,     0,    78,    79,     0,    80,    81,    82,
       0,     0,    84,     0,     0,     0,    85,     0,     0,     0,
       0,     0,    86,     0,     0,    88,    89,     0,    90,    91,
       0,    92,    93,     0,    94,    95,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   714,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,     0,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,     0,
       0,     0,     0,     0,    57,     0,    58,    59,    60,     0,
       0,     0,     0,     0,     0,     0,    64,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   143,    73,    74,    75,    76,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,     0,     0,    85,     0,     0,     0,     0,     0,    86,
       0,     0,    88,    89,     0,    90,    91,     0,    92,    93,
       0,    94,    95,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1075,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     0,     0,     0,    31,
      32,    33,    34,    35,     0,    36,     0,     0,     0,    37,
      38,    39,    40,     0,    41,     0,    42,     0,    43,     0,
       0,    44,     0,     0,     0,    45,    46,    47,    48,     0,
      50,    51,     0,    52,     0,    54,     0,     0,     0,     0,
       0,    57,     0,    58,    59,    60,     0,     0,     0,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   143,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,     0,     0,    84,     0,     0,     0,
      85,     0,     0,     0,     0,     0,    86,     0,     0,    88,
      89,     0,    90,    91,     0,    92,    93,     0,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1198,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,     0,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,     0,     0,     0,     0,     0,    57,     0,
      58,    59,    60,     0,     0,     0,     0,     0,     0,     0,
      64,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,   143,    73,
      74,    75,    76,    77,     0,    78,    79,     0,    80,    81,
      82,     0,     0,    84,     0,     0,     0,    85,     0,     0,
       0,     0,     0,    86,     0,     0,    88,    89,     0,    90,
      91,     0,    92,    93,     0,    94,    95,     5,     6,     7,
       8,     9,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      71,     0,     0,     0,     0,   143,    73,    74,    75,    76,
      77,     0,    78,    79,     0,    80,    81,    82,     0,     0,
      84,     0,     0,     0,    85,     0,     0,     0,     0,     0,
      86,     0,     0,    88,    89,     0,    90,    91,     0,    92,
      93,     0,    94,    95,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   141,     0,     0,    59,    60,     0,     0,     0,
       0,     0,     0,     0,   142,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   143,    73,    74,    75,     0,    77,     0,    78,
      79,     0,    80,    81,    82,     0,     0,    84,     0,     0,
       0,    85,     0,     0,     0,     0,     0,    86,     0,     0,
      88,    89,     0,   202,   291,   292,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,   293,
      10,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,     0,   315,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,    59,    60,     0,     0,     0,     0,     0,     0,
       0,   142,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   143,
      73,    74,    75,     0,    77,     0,    78,    79,     0,    80,
      81,    82,     0,     0,    84,     0,     0,     0,    85,     0,
       0,     0,     0,     0,    86,     0,     0,    88,    89,     0,
     205,     0,     0,    92,    93,     0,    94,    95,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   141,     0,     0,    59,
      60,     0,     0,     0,     0,     0,     0,     0,   142,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   143,    73,    74,    75,
       0,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,     0,     0,    85,     0,     0,     0,     0,
       0,    86,     0,     0,    88,    89,     0,   219,     0,     0,
      92,    93,     0,    94,    95,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   256,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   141,     0,     0,    59,    60,     0,     0,
       0,     0,     0,     0,     0,   142,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   143,    73,    74,    75,     0,    77,     0,
      78,    79,     0,    80,    81,    82,     0,     0,    84,     0,
       0,     0,    85,     0,     0,     0,     0,     0,    86,     0,
       0,    88,    89,     0,   290,   291,   292,    92,    93,     0,
      94,    95,     5,     6,     7,     8,     9,     0,     0,     0,
     293,    10,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,     0,   315,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     141,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,   142,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,     0,    77,     0,    78,    79,     0,
      80,    81,    82,     0,     0,    84,     0,   316,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
     376,     0,     0,     0,    92,    93,     0,    94,    95,     5,
       6,     7,     8,     9,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   466,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,     0,    31,    32,    33,    34,    35,
       0,     0,     0,     0,     0,    37,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    48,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   141,     0,     0,
      59,    60,     0,     0,     0,     0,     0,     0,     0,   142,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   143,    73,    74,
      75,     0,    77,     0,    78,    79,     0,    80,    81,    82,
       0,     0,    84,     0,     0,     0,    85,     0,     0,     0,
       0,     0,    86,     0,     0,    88,    89,     0,     0,     0,
       0,    92,    93,     0,    94,    95,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   477,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   141,     0,     0,    59,    60,     0,
       0,     0,     0,     0,     0,     0,   142,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   143,    73,    74,    75,     0,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,     0,     0,    85,     0,     0,     0,     0,     0,    86,
       0,     0,    88,    89,     0,     0,     0,     0,    92,    93,
       0,    94,    95,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   514,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   141,     0,     0,    59,    60,     0,     0,     0,     0,
       0,     0,     0,   142,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   143,    73,    74,    75,     0,    77,     0,    78,    79,
       0,    80,    81,    82,     0,     0,    84,     0,     0,     0,
      85,     0,     0,     0,     0,     0,    86,     0,     0,    88,
      89,     0,     0,     0,     0,    92,    93,     0,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   546,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   141,     0,
       0,    59,    60,     0,     0,     0,     0,     0,     0,     0,
     142,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,   143,    73,
      74,    75,     0,    77,     0,    78,    79,     0,    80,    81,
      82,     0,     0,    84,     0,     0,     0,    85,     0,     0,
       0,     0,     0,    86,     0,     0,    88,    89,     0,     0,
       0,     0,    92,    93,     0,    94,    95,     5,     6,     7,
       8,     9,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   548,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,     0,    31,    32,    33,    34,    35,     0,     0,
       0,     0,     0,    37,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   141,     0,     0,    59,    60,
       0,     0,     0,     0,     0,     0,     0,   142,    65,    66,
      67,    68,    69,    70,     0,     0,     0,     0,     0,     0,
      71,     0,     0,     0,     0,   143,    73,    74,    75,     0,
      77,     0,    78,    79,     0,    80,    81,    82,     0,     0,
      84,     0,     0,     0,    85,     0,     0,     0,     0,     0,
      86,     0,     0,    88,    89,     0,     0,     0,     0,    92,
      93,     0,    94,    95,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   773,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   141,     0,     0,    59,    60,     0,     0,     0,
       0,     0,     0,     0,   142,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   143,    73,    74,    75,     0,    77,     0,    78,
      79,     0,    80,    81,    82,     0,     0,    84,     0,     0,
       0,    85,     0,     0,     0,     0,     0,    86,     0,     0,
      88,    89,     0,     0,     0,     0,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   814,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,     0,    31,    32,    33,
      34,    35,     0,     0,     0,     0,     0,    37,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    48,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,    59,    60,     0,     0,     0,     0,     0,     0,
       0,   142,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   143,
      73,    74,    75,     0,    77,     0,    78,    79,     0,    80,
      81,    82,     0,     0,    84,     0,     0,     0,    85,     0,
       0,     0,     0,     0,    86,     0,     0,    88,    89,     0,
     290,   291,   292,    92,    93,     0,    94,    95,     5,     6,
       7,     8,     9,     0,     0,     0,   293,    10,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,     0,
     315,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   141,     0,     0,    59,
      60,     0,     0,     0,     0,     0,     0,     0,   142,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   143,    73,    74,    75,
     515,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,   390,     0,    85,     0,     0,     0,     0,
       0,    86,     0,     0,    88,    89,     0,   290,   291,   292,
      92,    93,     0,    94,    95,     5,     6,     7,     8,     9,
       0,     0,     0,   293,    10,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,     0,   315,     0,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
       0,    31,    32,    33,    34,    35,     0,     0,     0,     0,
       0,    37,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      48,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   141,     0,     0,    59,    60,     0,     0,
       0,     0,     0,     0,     0,   142,    65,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,   143,    73,    74,    75,     0,    77,     0,
      78,    79,     0,    80,    81,    82,     0,     0,    84,     0,
     392,     0,    85,     0,     0,     0,     0,     0,    86,     0,
       0,    88,    89,     0,   290,   291,   292,    92,    93,     0,
      94,    95,     5,     6,     7,     8,     9,     0,     0,     0,
     293,    10,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,     0,   315,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,   433,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     141,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,   142,    65,    66,    67,    68,    69,    70,     0,
    1093,  1094,  1095,  1096,  1097,    71,  1098,  1099,  1100,  1101,
     143,    73,    74,    75,     0,    77,     0,    78,    79,     0,
      80,    81,    82,     0,     0,    84,     0,   403,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,     0,     0,     0,    92,    93,  1102,    94,    95,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1103,
    1104,  1105,  1106,  1107,  1108,  1109,     0,     0,    33,     0,
       0,     0,     0,     0,     0,     0,     0,  1110,  1111,  1112,
    1113,  1114,  1115,  1116,  1117,  1118,  1119,  1120,  1121,  1122,
    1123,  1124,  1125,  1126,  1127,  1128,  1129,  1130,  1131,  1132,
    1133,  1134,  1135,  1136,  1137,  1138,  1139,  1140,  1141,  1142,
    1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,     0,     0,
    1151,  1152,  1153,  1154,  1155,  1156,  1157,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,  1158,  1159,
    1160,     0,  1161,     0,     0,    78,    79,     0,    80,    81,
      82,  1162,     0,  1163,     0,     0,  1164,   290,   291,   292,
       0,     0,     0,     0,  1165,     0,  1166,     0,     0,     0,
       0,     0,     0,   293,     0,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,     0,   315,   290,   291,
     292,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   293,     0,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,     0,   315,   290,
     291,   292,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   293,     0,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,     0,   315,
     290,   291,   292,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   293,     0,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,     0,
     315,     0,   642,     0,   290,   291,   292,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     293,     0,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   674,   315,   290,   291,   292,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   293,     0,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   677,   315,   290,   291,   292,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   293,     0,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   743,   315,   290,   291,   292,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   293,     0,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,     0,   315,     0,   794,
       0,   290,   291,   292,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   293,     0,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     816,   315,   290,   291,   292,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   293,   924,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   976,   315,   290,   291,   292,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   293,
       0,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,  1052,   315,   290,   291,   292,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     293,     0,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,     0,   315,     0,  1053,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   290,   291,
     292,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   293,   925,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,     0,   315,   290,
     291,   292,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   293,   405,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,     0,   315,
       0,   290,   291,   292,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   538,   293,     0,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
       0,   315,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   290,   291,   292,     0,     0,     0,     0,
       0,     0,   362,     0,     0,     0,     0,     0,     0,   293,
     557,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,     0,   315,   290,   291,   292,     0,     0,     0,
       0,     0,     0,     0,   658,     0,     0,     0,     0,     0,
     293,     0,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,     0,   315,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   290,
     291,   292,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   293,   696,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,     0,   315,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   889,     0,     0,
       0,     0,     0,     0,     0,   290,   291,   292,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     566,   293,   655,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,     0,   315,   290,   291,   292,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   293,     0,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,     0,   315
};

static const yytype_int16 yycheck[] =
{
       4,     4,    83,   221,   145,   280,    87,    30,     4,     4,
       4,   350,   744,   539,    28,   345,   325,    40,   356,     4,
     778,    44,   130,   945,   137,   315,   965,   187,   109,   187,
     968,    53,   968,   277,   181,    89,     4,   565,   748,   440,
     370,     8,    46,     8,     8,    49,     8,    26,   763,   781,
      72,     8,     8,    75,     8,     8,    24,    25,     8,    63,
       8,    73,     8,   985,     8,     8,    46,     8,   215,     8,
       8,     8,    57,     8,   401,    26,     8,   650,    61,    83,
     188,    29,     8,    87,   121,     8,     8,   414,     8,     8,
      90,    61,    61,   234,   102,   140,   571,   207,   208,   318,
      78,    61,   212,    93,    94,   109,    78,   653,   121,  1048,
     448,   144,   172,   173,   168,    90,     8,   162,    13,   665,
       0,    89,   162,   342,   170,   170,   180,   346,   173,   121,
     170,    26,    46,   121,    41,    66,    67,   174,   121,   172,
     173,    41,   127,   170,   144,  1067,    61,   173,    43,   859,
     154,   159,   121,    61,   140,    61,   141,   146,   171,   137,
     378,   121,   894,   175,    73,   137,   146,   277,   506,   144,
     140,   744,    73,   888,    61,    61,   162,   892,   893,   171,
      61,   161,   323,   171,   174,    61,    61,   173,    61,   193,
     193,   173,   196,   196,   173,    61,   200,    61,    90,   163,
     168,   223,   283,   173,   173,   172,   174,   320,   781,   174,
     172,   173,   180,   173,   218,   172,   172,   163,   172,   172,
      61,   172,   172,   171,   625,   193,   339,   171,   196,   172,
     252,   172,   146,   172,   172,   172,   946,   172,   576,   171,
     567,   322,   717,   324,   719,   171,   792,   360,   171,   171,
     157,   171,   171,   580,   581,   170,   369,   157,   173,   372,
     228,    61,   170,   267,   171,   173,   175,   173,   236,   237,
     238,   171,   276,   612,   175,   243,   280,   992,   147,   283,
      73,   249,   267,    61,   810,   395,   173,   173,   861,    93,
      94,   451,   173,   451,   117,   868,   824,   173,   173,   267,
     173,   315,   170,   357,   173,    98,   320,   173,    31,   173,
     883,   147,    92,    93,    94,    31,   320,   321,   322,    31,
     324,   894,   649,    31,   160,   339,    92,    93,    94,    66,
      67,    73,   173,   663,   578,   339,    78,   173,    73,   170,
     450,   170,   672,    78,   562,  1283,   360,  1283,  1218,  1219,
      73,   401,    71,   691,    73,   369,   360,    73,   372,  1298,
     507,    73,    73,   170,   414,    73,   117,    78,   372,    71,
     174,    73,   170,   173,   900,    73,   902,   144,   668,   383,
     383,    71,   350,   530,   544,   407,   544,  1309,   170,   357,
     550,   170,   550,  1315,   170,   173,   138,   139,   545,   170,
     404,  1214,  1215,   138,   139,   409,   733,   170,   170,    73,
      26,   738,   170,   986,    78,   383,   989,   990,   745,   404,
      31,    61,    71,   172,   172,   429,   430,   138,   139,   148,
     149,   512,   151,   152,   153,    25,   404,   179,    71,   586,
     138,   139,   550,   178,   172,   140,   148,   149,   757,   151,
     152,   153,    42,    61,   173,    45,   175,   787,   148,   149,
      61,   151,   152,   153,   173,   795,   115,   805,   578,   579,
     170,   173,   998,   175,   138,   139,  1234,    69,    70,    71,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,   170,    25,    71,  1068,   714,    61,   466,   148,
     149,   828,   151,   152,   153,    98,   844,   170,   512,   847,
      42,   144,   144,    45,   852,   148,   149,   567,   151,   152,
     153,   170,    40,    49,   863,   638,    59,    60,   162,    73,
     580,   581,   121,   643,    78,   539,   140,   541,   561,   177,
     173,     8,    31,   140,  1070,   140,   514,    45,    46,    47,
     880,    49,   140,    73,   558,  1287,   148,   149,    78,   151,
     152,   153,   121,   170,   568,   569,   569,   897,   162,  1301,
     908,   148,   149,   558,   151,   152,   153,    73,   546,    13,
     548,   172,    71,   170,    73,    93,    94,    95,   121,    13,
     558,   595,    13,   170,   138,   139,   171,   162,   566,   649,
     172,   569,    73,    79,   931,    13,   933,    78,   935,   171,
     171,   721,   939,   723,   941,   170,   176,   137,   138,   139,
     624,   624,   111,   770,   638,   170,     8,   170,   624,   624,
     624,   171,   171,    82,   638,   603,     8,   172,   171,   624,
      13,   137,   138,   139,   612,   613,    79,   977,   170,     8,
      73,   170,   141,   119,   668,   144,   624,   146,   172,   148,
     149,   170,   151,   152,   153,   171,   137,   138,   139,    61,
     780,  1197,   120,   161,   949,   679,   680,   681,    98,    41,
     703,   170,   122,   733,     8,   171,   175,    61,   738,   109,
     110,    71,   171,    73,   912,   745,     8,   117,   118,    13,
     704,   704,   119,   707,   177,   177,   174,     8,   926,   170,
     170,   122,   177,   171,  1287,   177,   171,     8,   172,    71,
     172,    73,   707,   170,  1054,   171,   944,   140,  1301,   839,
     140,   735,   170,   737,   154,  1062,   704,    99,   173,   707,
      13,   709,   104,   172,   106,   107,   108,   109,   110,   111,
     112,   102,    13,  1080,  1081,  1082,   163,   761,   761,  1086,
      13,   173,   177,  1090,   173,   761,   761,   761,   148,   149,
    1045,   151,   152,   153,    79,   172,   761,   170,   828,   170,
     170,   170,    13,   170,  1002,     8,   148,   149,   171,   151,
     122,  1009,    13,   761,   817,   175,   148,   149,     8,   151,
     152,   153,   806,   172,   172,   773,   810,    25,   171,   246,
     171,   137,   174,   250,    79,   170,   820,   820,   159,     8,
     930,   173,   170,   175,   122,   829,   170,    71,   171,    73,
      74,     8,   269,    51,   271,   272,   273,   274,   173,    71,
     171,    25,   171,   173,   829,   173,   814,   170,   170,     8,
     122,   137,   820,    71,    79,   965,   177,  1075,   826,    26,
      68,   829,   172,   171,  1191,   172,   163,    51,    26,   979,
     122,   171,   982,    71,     8,   122,   171,     8,   174,   122,
      98,   931,     8,   933,   173,   935,  1213,    71,   171,   939,
       8,   941,   170,   174,   171,   863,   900,  1224,   902,    26,
     171,  1042,  1229,   173,   148,   149,   124,   151,   152,   153,
    1237,    24,    25,   171,   146,    28,   148,   149,   150,   151,
     152,   153,   171,   141,     8,  1252,   144,    71,   146,   122,
     148,   149,   171,   151,   152,   153,    49,    26,   170,    71,
     124,    79,   172,   141,   171,   949,   144,  1274,   166,    73,
     148,   149,   170,   151,   152,   153,   960,   141,   172,  1286,
     144,   172,   146,   150,   148,   149,   146,   151,   152,   153,
    1188,   170,    71,   157,   171,  1193,   174,    73,    13,   964,
    1198,   104,   166,   171,    79,   171,   170,  1205,  1206,   122,
     171,    13,   122,   122,   998,   171,    73,   141,   173,  1003,
     144,   122,  1006,  1006,   148,   149,  1010,   151,   152,   153,
      41,   172,  1062,   170,   146,  1019,   148,   149,  1003,   151,
     152,   153,  1240,   171,   173,  1010,    13,  1031,   173,   122,
    1080,  1081,  1082,  1037,  1019,  1003,  1086,   173,  1006,    13,
      13,  1045,  1010,  1184,    73,   144,    13,    51,   173,   148,
     149,  1019,   151,   152,   153,   168,   171,   173,    73,   170,
      73,   174,  1047,    13,     8,   172,  1070,   180,    99,  1073,
    1074,  1074,    90,   104,  1078,   106,   107,   108,   109,   110,
     111,   112,   106,   107,   108,   109,   110,   111,  1073,   140,
      90,   153,    29,  1078,   207,   208,    13,   170,   172,   212,
    1181,  1319,   171,    71,    73,  1073,  1074,   155,   171,  1327,
    1078,   170,   172,   171,    73,   228,   171,   148,   149,   173,
     151,  1295,   407,   236,   237,   238,    71,   321,    73,   324,
     243,   322,   414,   127,   626,   664,   249,   667,  1306,   826,
     923,  1191,  1091,   174,   412,  1170,   960,  1182,   172,  1230,
      24,    25,  1321,    38,    28,    89,   793,  1020,   764,  1209,
     806,   883,   693,  1213,   277,   680,   111,   280,   244,   237,
     724,   749,   982,   141,  1224,    -1,   144,  1181,    -1,  1229,
     148,   149,    -1,   151,   152,   153,    -1,  1237,    -1,    -1,
      -1,    -1,    -1,  1197,    -1,    -1,   141,  1201,   166,   144,
      -1,   146,   315,   148,   149,  1209,   151,   152,   153,    -1,
    1214,  1215,    -1,    -1,  1218,  1219,  1201,   106,   107,   108,
     109,   110,   111,    -1,  1274,   170,  1230,    -1,   173,    -1,
     175,  1235,    -1,  1201,    -1,  1239,  1286,   350,  1242,    -1,
    1244,  1245,    -1,    -1,   357,    -1,    -1,    -1,    -1,    -1,
    1235,    -1,    -1,    -1,  1239,    -1,    -1,  1242,    -1,  1244,
    1245,    -1,    -1,    -1,    -1,    -1,    -1,  1235,    24,    25,
      -1,  1239,    28,  1277,  1242,    -1,  1244,  1245,  1282,    -1,
      -1,    25,   395,    -1,    -1,   567,    -1,    -1,   401,    -1,
      -1,    -1,  1277,  1297,   168,    -1,    -1,    -1,   580,   581,
     174,   414,    -1,    -1,    25,    -1,   180,    51,    -1,  1277,
     423,    42,    43,    44,    45,    46,    47,    -1,    49,  1323,
      -1,    -1,    -1,    -1,  1328,    -1,    -1,    71,    -1,    -1,
      51,    -1,    -1,    -1,    -1,    -1,    -1,   450,  1323,    -1,
      -1,    -1,    -1,  1328,    49,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    -1,   466,   228,  1323,    -1,    -1,    -1,    -1,
    1328,    -1,   236,   237,   238,    -1,    -1,   649,    -1,   243,
      -1,    -1,    -1,    -1,    -1,   249,    -1,    98,    -1,    -1,
     124,    -1,    -1,    -1,    -1,   106,   107,   108,   109,   110,
     111,    -1,    -1,    25,    -1,    -1,    -1,   141,    -1,    -1,
     144,   514,   146,   124,   148,   149,    -1,   151,   152,   153,
      -1,    -1,   168,   157,    -1,    -1,    -1,    -1,   174,    51,
     141,    -1,   166,   144,   180,   146,   170,   148,   149,    -1,
     151,   152,   153,   546,    -1,   548,    -1,    -1,    -1,    71,
      -1,   315,    -1,    -1,    -1,   166,    -1,    -1,    -1,   170,
      -1,   733,    -1,   566,   567,    -1,   738,    -1,    -1,    -1,
      -1,    -1,    -1,   745,    -1,   578,   579,   580,   581,    -1,
      -1,    -1,   228,    -1,    -1,    -1,   350,    -1,    -1,    -1,
     236,   237,   238,   357,    -1,    -1,    -1,   243,    -1,    -1,
     603,    -1,   124,   249,    -1,    41,    -1,   610,    -1,   612,
     613,    -1,   207,   208,    -1,    -1,    -1,   212,    -1,   141,
      -1,    41,   144,   626,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,    -1,    -1,    -1,   401,    -1,    -1,
     643,    -1,    -1,    -1,   166,    -1,   649,    -1,   170,   652,
     414,    -1,    -1,    -1,    -1,    -1,   828,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,   668,    -1,    -1,   104,   315,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    99,
      -1,    -1,   277,    -1,   104,   280,   106,   107,   108,   109,
     110,   111,   112,    -1,    -1,    -1,    -1,    -1,    71,    -1,
      -1,    -1,   466,    -1,   350,    -1,   709,    -1,    -1,    -1,
      -1,   357,   148,   149,    -1,   151,    -1,    -1,   721,    -1,
     723,    -1,    -1,    -1,    -1,    41,    -1,    -1,   148,   149,
     733,   151,    -1,    -1,    -1,   738,    -1,    -1,   174,   112,
      -1,    -1,   745,    -1,    -1,   748,   749,    -1,    -1,    -1,
     514,   124,    -1,    -1,   174,    -1,    -1,    -1,    -1,   931,
      -1,   933,    -1,   935,    -1,    -1,    -1,   939,   141,   941,
     773,   144,    -1,   146,    -1,   148,   149,   780,   151,   152,
     153,    -1,   546,    99,   548,    -1,    -1,    -1,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
     395,    -1,   566,   567,    -1,    -1,   401,    -1,    41,    -1,
      -1,   814,    -1,    -1,    71,    -1,   580,   581,    -1,   414,
     466,    -1,    -1,   826,    -1,   828,    -1,    -1,   423,    -1,
      -1,    -1,   148,   149,    -1,   151,   839,    -1,    -1,   603,
      -1,    -1,    -1,    -1,    -1,    -1,   610,    -1,   612,   613,
     853,    -1,    -1,    -1,    -1,   450,   859,   860,   174,    -1,
     863,    -1,   626,    -1,    -1,    -1,    99,    -1,   514,    -1,
      -1,   104,    -1,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,    -1,    -1,   141,   649,    -1,   144,   652,   146,
    1062,   148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,
     546,    -1,   548,    -1,   668,    -1,    -1,    -1,  1080,  1081,
    1082,    -1,    -1,    -1,  1086,   148,   149,    -1,   151,    -1,
     566,    42,    43,    -1,    -1,    -1,    -1,   930,   931,    -1,
     933,    -1,   935,    -1,    -1,    -1,   939,    -1,   941,    -1,
      61,   174,    -1,   946,    -1,   709,   949,   950,    69,    70,
      71,    -1,    -1,    -1,    -1,   958,    -1,   603,    79,    -1,
      -1,    -1,   965,    -1,    -1,    -1,   612,   613,    -1,   733,
      -1,    -1,   567,    -1,   738,    -1,   979,    -1,    -1,   982,
      -1,   745,    -1,   578,   579,   580,   581,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,   773,
      -1,    -1,    -1,    69,    70,   136,    -1,    -1,    -1,  1191,
      -1,    -1,   668,    79,    -1,    -1,    -1,   148,   149,    -1,
     151,   152,   153,    59,    60,    -1,    -1,    -1,    -1,    -1,
      -1,  1213,  1045,    -1,    -1,   166,    -1,    -1,   643,    -1,
     814,    -1,  1224,    -1,   649,    -1,    -1,  1229,    -1,  1062,
      -1,    -1,   826,   709,   828,  1237,    -1,    -1,    -1,   125,
     126,   127,   128,   129,    -1,    -1,    -1,  1080,  1081,  1082,
     136,    -1,    -1,  1086,  1087,    -1,   142,   143,  1091,   853,
      -1,    -1,    -1,    -1,    -1,   121,   860,    -1,    -1,   863,
     156,    -1,  1274,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,   169,  1286,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    73,    -1,    -1,    -1,   721,   773,   723,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   733,    -1,
      -1,    -1,    -1,   738,    -1,    -1,    -1,    -1,    -1,    -1,
     745,    -1,    -1,   748,   749,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,    -1,    -1,   931,   814,   933,
      -1,   935,    -1,    -1,    -1,   939,    -1,   941,    -1,    -1,
     826,    -1,    -1,    -1,    -1,   780,   950,    -1,  1191,    -1,
     141,    -1,    -1,   144,   958,   146,    -1,   148,   149,    63,
     151,   152,   153,    -1,    -1,    -1,  1209,    71,    -1,    73,
    1213,    -1,    -1,    -1,    -1,    -1,    -1,   863,    -1,   170,
      -1,  1224,    -1,    -1,   175,    -1,  1229,    -1,    -1,    -1,
      -1,    -1,    -1,   828,  1237,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   839,    -1,    -1,   111,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    -1,   859,    -1,    -1,    -1,    -1,    -1,
      -1,  1274,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,
     144,    -1,   146,  1286,   148,   149,    -1,   151,   152,   153,
      -1,    -1,    -1,    -1,    59,    60,    -1,    -1,  1062,    -1,
      -1,    -1,    -1,  1306,    -1,    -1,   170,    -1,  1311,    -1,
      -1,   175,    -1,    -1,    -1,    -1,  1080,  1081,  1082,    -1,
      -1,    -1,  1086,  1087,    -1,    -1,    -1,  1091,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   930,   931,    -1,   933,    -1,
     935,    -1,    -1,    -1,   939,    -1,   941,    -1,    -1,    -1,
      -1,   946,    -1,    -1,   949,    -1,   121,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     965,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   979,    -1,    -1,   982,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     5,     6,    -1,     8,     9,    10,  1191,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      -1,    -1,    26,    27,    -1,    -1,    -1,    -1,    -1,  1213,
    1045,    -1,    42,    43,    38,    -1,    -1,    -1,    -1,    -1,
    1224,    45,    -1,    47,    -1,  1229,    50,  1062,    52,    -1,
      -1,    -1,    -1,  1237,    -1,    -1,    -1,    -1,    -1,    69,
      70,    71,    -1,    -1,    -1,  1080,  1081,  1082,    -1,    79,
      -1,  1086,    76,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,    -1,    -1,
    1274,    -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,
      -1,    -1,  1286,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   125,   126,   127,   128,   129,
     177,    -1,  1306,    -1,    -1,    -1,   136,  1311,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,   111,   148,   149,
      -1,   151,   152,   153,    -1,   149,   156,    -1,    -1,   123,
      71,    -1,    73,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,
     144,    -1,   146,    -1,   148,   149,  1191,   151,   152,   153,
      -1,    -1,    -1,    -1,    -1,   189,    -1,    -1,   192,    -1,
     111,    -1,    -1,    -1,   198,   199,   170,    -1,  1213,    -1,
      -1,   175,   123,    -1,    -1,    -1,    -1,    -1,    -1,  1224,
      -1,    -1,    -1,    -1,  1229,    -1,    -1,    -1,    -1,    -1,
     141,    -1,  1237,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     244,    -1,    -1,    -1,   248,    -1,    -1,    -1,    -1,   170,
      -1,    -1,    -1,    -1,   175,    -1,    -1,    -1,    -1,  1274,
      -1,    -1,    -1,    -1,   268,    -1,    -1,    -1,    -1,    -1,
      -1,  1286,    -1,    -1,    -1,   279,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,    -1,    -1,   317,   318,   319,    -1,    -1,    -1,    -1,
      -1,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,    -1,    -1,    -1,    -1,    -1,   342,   343,
      -1,   345,   346,   347,    -1,    -1,    -1,    -1,   352,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   362,    -1,
     364,    -1,    -1,    -1,    -1,    -1,   370,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   380,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   410,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   477,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,   515,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,   529,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     554,    -1,    -1,   175,   176,    -1,   178,   179,    -1,    -1,
      -1,   565,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,   621,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,   633,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   647,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   655,    -1,    -1,   658,   141,   660,    -1,   144,   663,
     146,    -1,   148,   149,    -1,   151,   152,   153,   672,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,   170,    12,    -1,    -1,    -1,   175,
      -1,    -1,   696,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,   174,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,   757,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   769,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   787,   111,    -1,    -1,   114,   115,    -1,
      -1,   795,    -1,   797,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
     824,   148,   149,    -1,   151,   152,   153,    -1,   832,   156,
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,    -1,    -1,    -1,   175,   176,
      -1,   178,   179,    -1,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,   871,    -1,    -1,
      -1,   875,    -1,   877,    -1,    -1,   880,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   889,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   897,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,    -1,
      -1,    -1,   109,   110,   111,    -1,   113,   114,   115,   116,
     117,   118,    -1,   977,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
     167,    -1,   169,   170,    -1,   172,   173,   174,   175,   176,
      -1,   178,   179,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1054,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
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
     160,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,   169,
     170,    -1,   172,   173,   174,   175,   176,    11,   178,   179,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,   167,    -1,   169,   170,    -1,   172,
     173,    -1,   175,   176,    -1,   178,   179,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    25,    12,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
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
     156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,   175,
     176,    -1,   178,   179,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
     169,   170,    -1,   172,   173,   174,   175,   176,    -1,   178,
     179,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    83,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,    -1,   175,   176,    -1,   178,   179,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    89,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,    -1,
     175,   176,    -1,   178,   179,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,   169,   170,    -1,   172,   173,   174,   175,   176,    -1,
     178,   179,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    87,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   172,   173,    -1,   175,   176,    -1,   178,   179,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
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
     154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
     174,   175,   176,    -1,   178,   179,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,
      -1,    -1,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,   172,   173,   174,   175,   176,
      -1,   178,   179,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,
     170,    -1,   172,   173,    -1,   175,   176,    -1,   178,   179,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,
     173,   174,   175,   176,    -1,   178,   179,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,   174,   175,
     176,    -1,   178,   179,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
     169,   170,    -1,   172,   173,   174,   175,   176,    -1,   178,
     179,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,   174,   175,   176,    -1,   178,   179,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,   174,
     175,   176,    -1,   178,   179,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,   169,   170,    -1,   172,   173,    -1,   175,   176,    -1,
     178,   179,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,
     111,    -1,   113,   114,   115,    -1,    71,    -1,    73,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,   111,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   172,   173,    -1,   175,   176,    -1,   178,   179,     3,
       4,     5,     6,     7,    -1,    -1,   141,    -1,    12,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,    42,    43,
     175,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
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
      -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
      -1,   175,   176,    -1,   178,   179,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,
      -1,    -1,    -1,    -1,   111,    -1,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,   172,   173,    -1,   175,   176,
      -1,   178,   179,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
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
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,
     170,    -1,   172,   173,    -1,   175,   176,    -1,   178,   179,
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
     103,    -1,   105,    -1,    -1,    -1,    -1,    -1,   111,    -1,
     113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,
     173,    -1,   175,   176,    -1,   178,   179,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,    -1,   175,
     176,    -1,   178,   179,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
     169,   170,    -1,   172,    10,    11,   175,   176,    -1,   178,
     179,     3,     4,     5,     6,     7,    -1,    -1,    -1,    25,
      12,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
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
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,    -1,    -1,   175,   176,    -1,   178,   179,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,    -1,    -1,
     175,   176,    -1,   178,   179,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,   169,   170,    -1,     9,    10,    11,   175,   176,    -1,
     178,   179,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      25,    12,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,   172,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
     171,    -1,    -1,    -1,   175,   176,    -1,   178,   179,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,    -1,    -1,
      -1,   175,   176,    -1,   178,   179,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,    -1,    -1,    -1,   175,   176,
      -1,   178,   179,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,
     170,    -1,    -1,    -1,    -1,   175,   176,    -1,   178,   179,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,    -1,
      -1,    -1,   175,   176,    -1,   178,   179,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,    -1,    -1,    -1,   175,
     176,    -1,   178,   179,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
     169,   170,    -1,    -1,    -1,    -1,   175,   176,    -1,   178,
     179,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,
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
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
       9,    10,    11,   175,   176,    -1,   178,   179,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    25,    12,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,   172,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,     9,    10,    11,
     175,   176,    -1,   178,   179,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    25,    12,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
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
     172,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,   169,   170,    -1,     9,    10,    11,   175,   176,    -1,
     178,   179,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      25,    12,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
       3,     4,     5,     6,     7,   136,     9,    10,    11,    12,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,   172,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,    -1,    -1,    -1,   175,   176,    49,   178,   179,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,
      63,    64,    65,    66,    67,    68,    -1,    -1,    71,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,   142,
     143,    -1,   145,    -1,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,   159,     9,    10,    11,
      -1,    -1,    -1,    -1,   167,    -1,   169,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,   174,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,   174,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,   174,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   174,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,   174,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     174,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,   174,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,   174,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,   174,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   172,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   172,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   171,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     171,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
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
      43,    44,    45,    46,    47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   181,   182,     0,   183,     3,     4,     5,     6,     7,
      12,    41,    42,    43,    48,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    69,    70,    71,    72,    73,    75,    79,    80,    81,
      82,    84,    86,    88,    91,    95,    96,    97,    98,    99,
     100,   101,   103,   104,   105,   109,   110,   111,   113,   114,
     115,   116,   117,   118,   123,   124,   125,   126,   127,   128,
     129,   136,   141,   142,   143,   144,   145,   146,   148,   149,
     151,   152,   153,   154,   156,   160,   166,   167,   169,   170,
     172,   173,   175,   176,   178,   179,   184,   187,   190,   191,
     192,   193,   194,   195,   198,   209,   210,   213,   218,   224,
     280,   284,   285,   286,   287,   288,   295,   296,   297,   299,
     300,   303,   313,   314,   315,   320,   323,   341,   346,   348,
     349,   350,   351,   352,   353,   354,   355,   357,   370,   372,
     373,   111,   123,   141,   187,   209,   287,   348,   287,   170,
     287,   287,   287,   339,   340,   287,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   111,   170,   191,
     314,   315,   348,   348,    31,   287,   361,   362,   287,   111,
     170,   191,   314,   315,   316,   347,   353,   358,   359,   170,
     281,   317,   170,   281,   282,   287,   200,   281,   170,   170,
     170,   281,   172,   287,   187,   172,   287,    25,    51,   124,
     146,   166,   170,   187,   191,   192,   374,   384,   385,   172,
     287,   173,   287,   144,   188,   189,   190,    73,   175,   248,
     249,   117,   117,    73,   209,   250,   170,   170,   170,   170,
     187,   222,   375,   170,   170,    73,    78,   137,   138,   139,
     367,   368,   144,   173,   190,   190,    95,   287,   223,   375,
     146,   170,   375,   280,   287,   288,   348,   196,   173,    78,
     318,   367,    78,   367,   367,    26,   144,   162,   376,   170,
       8,   172,    31,   208,   146,   221,   375,   172,   172,   172,
       9,    10,    11,    25,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    49,   172,    61,    61,   173,
     140,   118,   154,   209,   224,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    59,    60,   121,
     343,   344,    61,   173,   345,   170,    61,   173,   175,   354,
     170,   208,    13,   287,    40,   187,   338,   170,   280,   348,
     140,   348,   122,   177,     8,   325,   280,   348,   376,   140,
     170,   319,   121,   343,   344,   345,   171,   287,    26,   198,
       8,   172,   198,   199,   282,   283,   287,   187,   236,   202,
     172,   172,   172,   385,   385,   162,   170,    98,   377,   385,
     376,    13,   187,   172,   196,   172,   190,     8,   172,    90,
     173,   348,     8,   172,    13,   208,     8,   172,   348,   371,
     371,   348,   171,   162,   216,   123,   348,   360,   361,    61,
     121,   137,   368,    72,   287,   348,    78,   137,   368,   190,
     186,   172,   173,   172,   219,   304,   306,    79,   291,   293,
      13,   171,   171,   171,   174,   197,   198,   210,   213,   218,
     287,   176,   178,   179,   187,   377,    31,   246,   247,   287,
     374,   170,   375,   214,   287,   287,   287,    26,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   287,   287,
     316,   287,   356,   356,   287,   363,   364,   187,   353,   354,
     222,   223,   208,   221,    31,   145,   284,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   287,   173,
     187,   353,   356,   287,   246,   356,   287,   360,   171,   170,
     337,     8,   325,   280,   171,   187,    31,   287,    31,   287,
     171,   171,   353,   246,   173,   187,   353,   171,   196,   240,
     287,    82,    26,   198,   234,   172,    90,    13,     8,   171,
      26,   173,   237,   385,    79,   381,   382,   383,   170,     8,
      42,    43,   124,   136,   146,   166,   191,   192,   194,   298,
     314,   320,   321,   322,   174,    90,   189,   187,   287,   249,
     321,   170,    73,     8,   171,   171,   171,   172,   192,   380,
     119,   227,   170,     8,   171,   171,    73,    74,   187,   369,
     187,    61,   174,   174,   183,   185,   120,   226,   161,    46,
     146,   161,   308,   122,     8,   325,   171,   385,   121,   343,
     344,   345,   174,     8,   163,   348,   171,     8,   326,    13,
     289,   211,   119,   225,   287,    26,   177,   177,   122,   174,
       8,   325,   376,   170,   217,   220,   375,   215,    63,   348,
     287,   376,   170,   177,   174,   171,   177,   174,   171,    42,
      43,    61,    69,    70,    79,   124,   136,   166,   187,   328,
     330,   333,   336,   187,   348,   348,   122,   343,   344,   345,
     171,   287,   241,    66,    67,   242,   281,   196,   283,    31,
     231,   348,   321,   187,    26,   198,   235,   172,   238,   172,
     238,     8,   163,   122,     8,   325,   171,   157,   377,   378,
     385,   321,   321,   170,    78,   140,   170,   140,   173,   102,
     205,   206,   187,   174,   290,    13,   348,   172,     8,    90,
     163,   228,   314,   173,   360,   123,   348,    13,   177,   287,
     174,   183,   228,   173,   307,    13,   287,    79,   172,   173,
     187,   353,   385,    31,   287,   321,   157,   244,   245,   341,
     342,   170,   314,   226,   287,   287,   287,   170,   246,   227,
     226,   212,   225,   316,   174,   170,   246,    13,    69,    70,
     187,   329,   329,   330,   331,   332,   170,    78,   137,   170,
       8,   325,   171,   337,    31,   287,   174,    66,    67,   243,
     281,   198,   172,    83,   172,   348,   122,   230,    13,   196,
     238,    92,    93,    94,   238,   174,   385,   385,   381,     8,
     171,   171,   321,   324,   327,   187,    79,   292,   294,   187,
     321,   365,   366,   170,   159,   244,   321,   380,   192,   384,
       8,   251,   171,   170,   284,   287,   177,   174,   251,   147,
     160,   173,   303,   310,   147,   173,   309,   122,   287,   376,
     170,   348,   171,     8,   326,   385,   386,   244,   173,   122,
     246,   171,   173,   173,   170,   226,   319,   170,   246,   171,
     122,   177,     8,   325,   331,   137,    79,   334,   335,   330,
     348,   281,    26,    68,   198,   172,   283,   231,   171,   321,
      89,    92,   172,   287,    26,   172,   239,   174,   163,   157,
      26,   122,   171,     8,   325,   122,     8,   325,   171,   122,
     174,     8,   325,   314,   173,   171,     8,   380,   314,    99,
     104,   106,   107,   108,   109,   110,   111,   112,   148,   149,
     151,   174,   252,   274,   275,   276,   277,   279,   341,   360,
     174,   174,    46,   287,   287,   287,   174,   170,   246,    26,
     379,   157,   342,    31,    73,   171,   251,   287,   171,   251,
     251,   244,   173,   246,   171,   330,   330,   171,   122,   171,
       8,   325,    26,   196,   172,   171,   203,   172,   172,   239,
     196,   385,   321,   321,   321,    79,   321,   321,    73,   196,
     379,   380,   374,   229,   314,   112,   124,   146,   152,   261,
     262,   263,   314,   150,   267,   268,   115,   170,   187,   269,
     270,   253,   209,   277,   385,     8,   172,   275,   276,   171,
     146,   305,   174,   174,   170,   246,   171,   385,   104,   301,
     386,    73,    13,   379,   174,   174,   174,   171,   251,   171,
     122,   330,    79,   196,   201,    26,   198,   233,   196,   171,
     122,   122,   122,   171,   174,   301,    13,     8,   172,   173,
     173,     8,   172,     3,     4,     5,     6,     7,     9,    10,
      11,    12,    49,    62,    63,    64,    65,    66,    67,    68,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   123,   124,   125,   126,   127,   128,   129,   141,   142,
     143,   145,   154,   156,   159,   167,   169,   187,   311,   312,
       8,   172,   146,   150,   187,   270,   271,   272,   172,    73,
     278,   208,   254,   374,   209,   246,   171,   170,   173,    31,
      73,    13,   321,   173,   379,   174,   330,   122,    26,   198,
     232,   196,   321,   321,   321,   173,   173,   321,   314,   257,
     264,   320,   262,    13,    26,    43,   265,   268,     8,    29,
     171,    25,    42,    45,    13,     8,   172,   375,   278,    13,
     208,   171,    31,    73,   302,   196,    73,    13,   321,   196,
     173,   330,   196,    87,   196,   196,   174,   187,   194,   258,
     259,   260,     8,   174,   321,   312,   312,    51,   266,   271,
     271,    25,    42,    45,   321,    73,   170,   172,   321,   375,
      73,     8,   326,   174,    13,   321,   174,   196,    85,   172,
     174,   174,   140,    90,   320,   153,    13,   255,   170,    31,
      73,   171,   321,   174,   172,   204,   187,   275,   276,   321,
     244,   256,    73,   102,   205,   207,   155,   187,   172,   171,
     244,   170,   229,   172,   379,   171,   314,   172,   172,   173,
     273,   379,    73,   196,   273,   171,   174,   173,   196,   174
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
#line 815 "hphp.y"
    { _p->initParseTree(); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 815 "hphp.y"
    { _p->popLabelInfo();
                                                  _p->finiParseTree();;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 822 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 855 "hphp.y"
    { ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 856 "hphp.y"
    { ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 859 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 860 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 861 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 863 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 875 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 878 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 885 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 892 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 900 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),  0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { _p->onBreak((yyval), NULL);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->onBreak((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { _p->onContinue((yyval), NULL);;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { _p->onContinue((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { (yyval).reset();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { finally_statement(_p);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (5)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { (yyval).reset();;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { (yyval).reset();;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(8) - (11)]),(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(9) - (12)]),(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1046 "hphp.y"
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
#line 1061 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1064 "hphp.y"
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
#line 1078 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1089 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1099 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,t_imp,
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1107 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1110 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,t_imp,
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1119 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1123 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1126 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1131 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1136 "hphp.y"
    { (yyval).reset();;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1140 "hphp.y"
    { (yyval).reset();;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1144 "hphp.y"
    { (yyval).reset();;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1149 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1154 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1159 "hphp.y"
    { (yyval).reset();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1163 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1169 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1174 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1179 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1184 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1197 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1205 "hphp.y"
    { (yyval).reset();;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval).reset();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1209 "hphp.y"
    { (yyval).reset();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1215 "hphp.y"
    { (yyval).reset();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1221 "hphp.y"
    { (yyval).reset();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1225 "hphp.y"
    { (yyval).reset();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1229 "hphp.y"
    { (yyval).reset();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset(); ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1238 "hphp.y"
    { (yyval).reset();;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,NULL,&(yyvsp[(1) - (3)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1245 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,NULL,&(yyvsp[(1) - (4)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1248 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,&(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1251 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,&(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1254 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,NULL,&(yyvsp[(3) - (5)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,NULL,&(yyvsp[(3) - (6)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1261 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,&(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1265 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,&(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1271 "hphp.y"
    { (yyval).reset();;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1274 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1283 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1284 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1296 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1302 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1303 "hphp.y"
    { (yyval).reset();;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1306 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1307 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1310 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1317 "hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1322 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1328 "hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1341 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1345 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1349 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1350 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1360 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1363 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1371 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1379 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1388 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1393 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1411 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1412 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1416 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1417 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1421 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1424 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1434 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1437 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1443 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1444 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1456 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1463 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1468 "hphp.y"
    { (yyval).reset();;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1469 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1472 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1473 "hphp.y"
    { (yyval).reset();;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1477 "hphp.y"
    { (yyval).reset();;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1482 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1489 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1490 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1497 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1498 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1517 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { (yyval).reset();;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),1);
                                         _p->popLabelInfo();;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { validate_shape_keyname((yyvsp[(3) - (5)]), _p);
                                        _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                        _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { validate_shape_keyname((yyvsp[(3) - (5)]), _p);
                                        _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                        _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1666 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1687 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1694 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1709 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1717 "hphp.y"
    { (yyval).reset();;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1724 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1731 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         (yyval).setText("");;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1741 "hphp.y"
    { _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { (yyval).reset();;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1759 "hphp.y"
    { (yyval).reset();;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1766 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1772 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1780 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1788 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1865 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { (yyval).reset();;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { (yyval).reset();;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { (yyval).reset();;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { (yyval).reset();;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval).reset();;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval).reset();;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval).reset();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval).reset();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval).reset();;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { validate_shape_keyname((yyvsp[(3) - (5)]), _p);
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                         _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval).reset();;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2069 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { user_attribute_check(_p);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval).reset();;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2091 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2096 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2107 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2121 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2123 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2135 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2142 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2153 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2156 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2162 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2182 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2187 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval).reset();;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval)++;;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2221 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2229 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval).reset();;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2278 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2287 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2301 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2344 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2351 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]);
                                         only_in_hh_syntax(_p); ;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2362 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (5)]).text()); ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2396 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2415 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2417 "hphp.y"
    {;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2421 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyval).setText("array"); ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2432 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2455 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2459 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2467 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10576 "new_hphp.tab.cpp"
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
#line 2471 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

