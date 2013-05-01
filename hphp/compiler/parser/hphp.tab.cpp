
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
#include "util/parser/test/parser.h"
#else
#include "compiler/parser/parser.h"
#endif
#include "util/util.h"
#include "util/logger.h"

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
  if (!_p->enableHipHopSyntax()) {
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

/* This is called from strict-mode productions (sm_*) to throw an
 * error if we're not in strict mode */
static void only_in_strict_mode(Parser *_p) {
  if (!_p->scanner().isStrictMode()) {
    HPHP_PARSER_ERROR("Syntax only allowed in strict mode", _p);
  }
}

static void only_in_hphp_syntax(Parser *_p) {
  if (!_p->enableHipHopSyntax()) {
    HPHP_PARSER_ERROR("Syntax only allowed with -v Eval.EnableHipHopSyntax=true", _p);
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
#line 763 "new_hphp.tab.cpp"

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
     T_STRICT_ERROR = 394,
     T_FINALLY = 395,
     T_XHP_TAG_LT = 396,
     T_XHP_TAG_GT = 397,
     T_TYPELIST_LT = 398,
     T_TYPELIST_GT = 399,
     T_UNRESOLVED_LT = 400,
     T_COLLECTION = 401,
     T_SHAPE = 402,
     T_TYPE = 403,
     T_UNRESOLVED_TYPE = 404
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
#line 967 "new_hphp.tab.cpp"

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
#define YYLAST   10373

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  179
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  206
/* YYNRULES -- Number of rules.  */
#define YYNRULES  708
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1324

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   404

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   177,     2,   174,    47,    31,   178,
     169,   170,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   171,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   176,    30,     2,   175,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   172,    29,   173,    50,     2,     2,     2,
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
     164,   165,   166,   167,   168
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    11,    13,    15,    17,
      19,    24,    28,    29,    36,    37,    43,    47,    50,    52,
      54,    56,    58,    60,    62,    66,    68,    70,    73,    77,
      82,    84,    88,    90,    93,    97,    99,   102,   105,   111,
     116,   119,   120,   122,   124,   126,   128,   132,   138,   147,
     148,   153,   154,   161,   162,   173,   174,   179,   182,   186,
     189,   193,   196,   200,   204,   208,   212,   216,   222,   224,
     226,   227,   237,   243,   258,   264,   268,   272,   275,   278,
     281,   284,   287,   297,   298,   299,   305,   307,   308,   310,
     311,   313,   314,   326,   327,   340,   341,   350,   351,   361,
     362,   370,   371,   380,   381,   388,   389,   397,   399,   401,
     403,   405,   407,   410,   413,   416,   417,   420,   421,   424,
     425,   427,   431,   433,   437,   440,   441,   443,   446,   448,
     453,   455,   460,   462,   467,   469,   474,   478,   484,   488,
     493,   498,   504,   510,   515,   516,   518,   520,   525,   526,
     532,   533,   536,   537,   541,   542,   546,   549,   551,   552,
     556,   561,   568,   574,   580,   587,   596,   604,   607,   608,
     610,   613,   617,   622,   626,   628,   630,   633,   638,   642,
     648,   650,   654,   657,   658,   659,   664,   665,   671,   674,
     675,   686,   687,   699,   703,   707,   711,   715,   721,   724,
     727,   728,   735,   741,   746,   750,   752,   754,   758,   763,
     765,   767,   769,   771,   776,   778,   782,   785,   786,   789,
     790,   792,   796,   798,   800,   802,   804,   808,   813,   818,
     823,   825,   827,   830,   833,   836,   840,   844,   846,   848,
     850,   852,   856,   858,   860,   862,   863,   865,   868,   870,
     872,   874,   876,   878,   880,   884,   890,   892,   896,   902,
     907,   911,   915,   919,   923,   925,   927,   928,   931,   935,
     942,   944,   946,   948,   955,   959,   964,   971,   974,   978,
     982,   986,   990,   994,   998,  1002,  1006,  1010,  1014,  1018,
    1021,  1024,  1027,  1030,  1034,  1038,  1042,  1046,  1050,  1054,
    1058,  1062,  1066,  1070,  1074,  1078,  1082,  1086,  1090,  1094,
    1097,  1100,  1103,  1106,  1110,  1114,  1118,  1122,  1126,  1130,
    1134,  1138,  1142,  1146,  1152,  1157,  1159,  1162,  1165,  1168,
    1171,  1174,  1177,  1180,  1183,  1186,  1188,  1190,  1192,  1196,
    1199,  1200,  1212,  1213,  1226,  1228,  1230,  1232,  1238,  1242,
    1248,  1252,  1255,  1256,  1259,  1260,  1265,  1270,  1274,  1279,
    1284,  1289,  1294,  1296,  1298,  1302,  1308,  1309,  1313,  1318,
    1320,  1323,  1328,  1331,  1338,  1339,  1341,  1346,  1347,  1350,
    1351,  1353,  1355,  1359,  1361,  1365,  1367,  1369,  1373,  1377,
    1379,  1381,  1383,  1385,  1387,  1389,  1391,  1393,  1395,  1397,
    1399,  1401,  1403,  1405,  1407,  1409,  1411,  1413,  1415,  1417,
    1419,  1421,  1423,  1425,  1427,  1429,  1431,  1433,  1435,  1437,
    1439,  1441,  1443,  1445,  1447,  1449,  1451,  1453,  1455,  1457,
    1459,  1461,  1463,  1465,  1467,  1469,  1471,  1473,  1475,  1477,
    1479,  1481,  1483,  1485,  1487,  1489,  1491,  1493,  1495,  1497,
    1499,  1501,  1503,  1505,  1507,  1509,  1511,  1513,  1515,  1517,
    1519,  1521,  1523,  1525,  1530,  1532,  1534,  1536,  1538,  1540,
    1542,  1544,  1546,  1549,  1551,  1552,  1553,  1555,  1557,  1561,
    1562,  1564,  1566,  1568,  1570,  1572,  1574,  1576,  1578,  1580,
    1582,  1584,  1588,  1591,  1593,  1595,  1598,  1601,  1606,  1611,
    1613,  1615,  1619,  1623,  1625,  1627,  1629,  1631,  1635,  1639,
    1643,  1646,  1647,  1649,  1650,  1652,  1653,  1659,  1663,  1667,
    1669,  1671,  1673,  1675,  1679,  1682,  1684,  1686,  1688,  1690,
    1692,  1695,  1698,  1703,  1707,  1712,  1715,  1716,  1722,  1726,
    1730,  1732,  1736,  1738,  1741,  1742,  1748,  1752,  1755,  1756,
    1760,  1761,  1766,  1769,  1770,  1774,  1778,  1780,  1781,  1783,
    1786,  1789,  1794,  1798,  1802,  1805,  1810,  1813,  1818,  1820,
    1822,  1824,  1826,  1828,  1831,  1836,  1840,  1845,  1849,  1851,
    1853,  1855,  1857,  1860,  1865,  1870,  1874,  1876,  1878,  1882,
    1890,  1897,  1906,  1916,  1925,  1936,  1944,  1951,  1953,  1956,
    1961,  1966,  1968,  1970,  1975,  1977,  1978,  1980,  1983,  1985,
    1987,  1990,  1995,  1999,  2003,  2004,  2006,  2009,  2014,  2018,
    2021,  2025,  2032,  2033,  2035,  2040,  2043,  2044,  2050,  2054,
    2058,  2060,  2067,  2072,  2077,  2080,  2083,  2084,  2090,  2094,
    2098,  2100,  2103,  2104,  2110,  2114,  2118,  2120,  2123,  2126,
    2128,  2131,  2133,  2138,  2142,  2146,  2153,  2157,  2159,  2161,
    2163,  2168,  2173,  2176,  2179,  2184,  2187,  2190,  2192,  2196,
    2200,  2206,  2208,  2211,  2213,  2218,  2222,  2223,  2225,  2229,
    2233,  2235,  2237,  2238,  2239,  2242,  2246,  2248,  2253,  2259,
    2263,  2267,  2271,  2275,  2277,  2280,  2281,  2286,  2289,  2292,
    2295,  2297,  2299,  2304,  2311,  2313,  2322,  2328,  2330
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     180,     0,    -1,   181,    -1,   181,   182,    -1,    -1,   196,
      -1,   208,    -1,   211,    -1,   216,    -1,   371,    -1,   116,
     169,   170,   171,    -1,   141,   188,   171,    -1,    -1,   141,
     188,   172,   183,   181,   173,    -1,    -1,   141,   172,   184,
     181,   173,    -1,   104,   186,   171,    -1,   193,   171,    -1,
      71,    -1,   148,    -1,   149,    -1,   151,    -1,   153,    -1,
     152,    -1,   186,     8,   187,    -1,   187,    -1,   188,    -1,
     144,   188,    -1,   188,    90,   185,    -1,   144,   188,    90,
     185,    -1,   185,    -1,   188,   144,   185,    -1,   188,    -1,
     144,   188,    -1,   141,   144,   188,    -1,   189,    -1,   189,
     374,    -1,   189,   374,    -1,   193,     8,   372,    13,   319,
      -1,    99,   372,    13,   319,    -1,   194,   195,    -1,    -1,
     196,    -1,   208,    -1,   211,    -1,   216,    -1,   172,   194,
     173,    -1,    65,   279,   196,   238,   240,    -1,    65,   279,
      26,   194,   239,   241,    68,   171,    -1,    -1,    82,   279,
     197,   232,    -1,    -1,    81,   198,   196,    82,   279,   171,
      -1,    -1,    84,   169,   281,   171,   281,   171,   281,   170,
     199,   230,    -1,    -1,    91,   279,   200,   235,    -1,    95,
     171,    -1,    95,   285,   171,    -1,    97,   171,    -1,    97,
     285,   171,    -1,   100,   171,    -1,   100,   285,   171,    -1,
     145,    95,   171,    -1,   105,   246,   171,    -1,   111,   248,
     171,    -1,    80,   280,   171,    -1,   113,   169,   369,   170,
     171,    -1,   171,    -1,    75,    -1,    -1,    86,   169,   285,
      90,   229,   228,   170,   201,   231,    -1,    88,   169,   234,
     170,   233,    -1,   101,   172,   194,   173,   102,   169,   312,
      73,   170,   172,   194,   173,   202,   205,    -1,   101,   172,
     194,   173,   203,    -1,   103,   285,   171,    -1,    96,   185,
     171,    -1,   285,   171,    -1,   282,   171,    -1,   283,   171,
      -1,   284,   171,    -1,   185,    26,    -1,   202,   102,   169,
     312,    73,   170,   172,   194,   173,    -1,    -1,    -1,   204,
     159,   172,   194,   173,    -1,   203,    -1,    -1,    31,    -1,
      -1,    98,    -1,    -1,   207,   206,   373,   209,   169,   242,
     170,   377,   172,   194,   173,    -1,    -1,   339,   207,   206,
     373,   210,   169,   242,   170,   377,   172,   194,   173,    -1,
      -1,   222,   219,   212,   223,   224,   172,   249,   173,    -1,
      -1,   339,   222,   219,   213,   223,   224,   172,   249,   173,
      -1,    -1,   118,   220,   214,   225,   172,   249,   173,    -1,
      -1,   339,   118,   220,   215,   225,   172,   249,   173,    -1,
      -1,   154,   221,   217,   172,   249,   173,    -1,    -1,   339,
     154,   221,   218,   172,   249,   173,    -1,   373,    -1,   146,
      -1,   373,    -1,   373,    -1,   117,    -1,   110,   117,    -1,
     109,   117,    -1,   119,   312,    -1,    -1,   120,   226,    -1,
      -1,   119,   226,    -1,    -1,   312,    -1,   226,     8,   312,
      -1,   312,    -1,   227,     8,   312,    -1,   122,   229,    -1,
      -1,   346,    -1,    31,   346,    -1,   196,    -1,    26,   194,
      85,   171,    -1,   196,    -1,    26,   194,    87,   171,    -1,
     196,    -1,    26,   194,    83,   171,    -1,   196,    -1,    26,
     194,    89,   171,    -1,   185,    13,   319,    -1,   234,     8,
     185,    13,   319,    -1,   172,   236,   173,    -1,   172,   171,
     236,   173,    -1,    26,   236,    92,   171,    -1,    26,   171,
     236,    92,   171,    -1,   236,    93,   285,   237,   194,    -1,
     236,    94,   237,   194,    -1,    -1,    26,    -1,   171,    -1,
     238,    66,   279,   196,    -1,    -1,   239,    66,   279,    26,
     194,    -1,    -1,    67,   196,    -1,    -1,    67,    26,   194,
      -1,    -1,   243,     8,   157,    -1,   243,   324,    -1,   157,
      -1,    -1,   340,   384,    73,    -1,   340,   384,    31,    73,
      -1,   340,   384,    31,    73,    13,   319,    -1,   340,   384,
      73,    13,   319,    -1,   243,     8,   340,   384,    73,    -1,
     243,     8,   340,   384,    31,    73,    -1,   243,     8,   340,
     384,    31,    73,    13,   319,    -1,   243,     8,   340,   384,
      73,    13,   319,    -1,   245,   324,    -1,    -1,   285,    -1,
      31,   346,    -1,   245,     8,   285,    -1,   245,     8,    31,
     346,    -1,   246,     8,   247,    -1,   247,    -1,    73,    -1,
     174,   346,    -1,   174,   172,   285,   173,    -1,   248,     8,
      73,    -1,   248,     8,    73,    13,   319,    -1,    73,    -1,
      73,    13,   319,    -1,   249,   250,    -1,    -1,    -1,   272,
     251,   276,   171,    -1,    -1,   274,   383,   252,   276,   171,
      -1,   277,   171,    -1,    -1,   273,   207,   206,   373,   169,
     253,   242,   170,   377,   271,    -1,    -1,   339,   273,   207,
     206,   373,   169,   254,   242,   170,   377,   271,    -1,   148,
     259,   171,    -1,   149,   265,   171,    -1,   151,   267,   171,
      -1,   104,   227,   171,    -1,   104,   227,   172,   255,   173,
      -1,   255,   256,    -1,   255,   257,    -1,    -1,   192,   140,
     185,   155,   227,   171,    -1,   258,    90,   273,   185,   171,
      -1,   258,    90,   274,   171,    -1,   192,   140,   185,    -1,
     185,    -1,   260,    -1,   259,     8,   260,    -1,   261,   309,
     263,   264,    -1,   146,    -1,   124,    -1,   312,    -1,   112,
      -1,   152,   172,   262,   173,    -1,   318,    -1,   262,     8,
     318,    -1,    13,   319,    -1,    -1,    51,   153,    -1,    -1,
     266,    -1,   265,     8,   266,    -1,   150,    -1,   268,    -1,
     185,    -1,   115,    -1,   169,   269,   170,    -1,   169,   269,
     170,    45,    -1,   169,   269,   170,    25,    -1,   169,   269,
     170,    42,    -1,   268,    -1,   270,    -1,   270,    45,    -1,
     270,    25,    -1,   270,    42,    -1,   269,     8,   269,    -1,
     269,    29,   269,    -1,   185,    -1,   146,    -1,   150,    -1,
     171,    -1,   172,   194,   173,    -1,   274,    -1,   112,    -1,
     274,    -1,    -1,   275,    -1,   274,   275,    -1,   106,    -1,
     107,    -1,   108,    -1,   111,    -1,   110,    -1,   109,    -1,
     276,     8,    73,    -1,   276,     8,    73,    13,   319,    -1,
      73,    -1,    73,    13,   319,    -1,   277,     8,   372,    13,
     319,    -1,    99,   372,    13,   319,    -1,    63,   314,   317,
      -1,   169,   278,   170,    -1,   169,   285,   170,    -1,   280,
       8,   285,    -1,   285,    -1,   280,    -1,    -1,   145,   285,
      -1,   346,    13,   282,    -1,   123,   169,   358,   170,    13,
     282,    -1,   286,    -1,   346,    -1,   278,    -1,   123,   169,
     358,   170,    13,   285,    -1,   346,    13,   285,    -1,   346,
      13,    31,   346,    -1,   346,    13,    31,    63,   314,   317,
      -1,    62,   285,    -1,   346,    24,   285,    -1,   346,    23,
     285,    -1,   346,    22,   285,    -1,   346,    21,   285,    -1,
     346,    20,   285,    -1,   346,    19,   285,    -1,   346,    18,
     285,    -1,   346,    17,   285,    -1,   346,    16,   285,    -1,
     346,    15,   285,    -1,   346,    14,   285,    -1,   346,    60,
      -1,    60,   346,    -1,   346,    59,    -1,    59,   346,    -1,
     285,    27,   285,    -1,   285,    28,   285,    -1,   285,     9,
     285,    -1,   285,    11,   285,    -1,   285,    10,   285,    -1,
     285,    29,   285,    -1,   285,    31,   285,    -1,   285,    30,
     285,    -1,   285,    44,   285,    -1,   285,    42,   285,    -1,
     285,    43,   285,    -1,   285,    45,   285,    -1,   285,    46,
     285,    -1,   285,    47,   285,    -1,   285,    41,   285,    -1,
     285,    40,   285,    -1,    42,   285,    -1,    43,   285,    -1,
      48,   285,    -1,    50,   285,    -1,   285,    33,   285,    -1,
     285,    32,   285,    -1,   285,    35,   285,    -1,   285,    34,
     285,    -1,   285,    36,   285,    -1,   285,    39,   285,    -1,
     285,    37,   285,    -1,   285,    38,   285,    -1,   285,    49,
     314,    -1,   169,   286,   170,    -1,   285,    25,   285,    26,
     285,    -1,   285,    25,    26,   285,    -1,   368,    -1,    58,
     285,    -1,    57,   285,    -1,    56,   285,    -1,    55,   285,
      -1,    54,   285,    -1,    53,   285,    -1,    52,   285,    -1,
      64,   315,    -1,    51,   285,    -1,   321,    -1,   294,    -1,
     293,    -1,   175,   316,   175,    -1,    12,   285,    -1,    -1,
     207,   206,   169,   287,   242,   170,   377,   299,   172,   194,
     173,    -1,    -1,   111,   207,   206,   169,   288,   242,   170,
     377,   299,   172,   194,   173,    -1,   301,    -1,   297,    -1,
     295,    -1,   289,     8,    79,   122,   285,    -1,    79,   122,
     285,    -1,   290,     8,    79,   122,   319,    -1,    79,   122,
     319,    -1,   289,   323,    -1,    -1,   290,   323,    -1,    -1,
     166,   169,   291,   170,    -1,   124,   169,   359,   170,    -1,
      61,   359,   176,    -1,   312,   172,   361,   173,    -1,   312,
     172,   363,   173,    -1,   297,    61,   354,   176,    -1,   298,
      61,   354,   176,    -1,   294,    -1,   370,    -1,   169,   286,
     170,    -1,   104,   169,   300,   324,   170,    -1,    -1,   300,
       8,    73,    -1,   300,     8,    31,    73,    -1,    73,    -1,
      31,    73,    -1,   160,   146,   302,   161,    -1,   304,    46,
      -1,   304,   161,   305,   160,    46,   303,    -1,    -1,   146,
      -1,   304,   306,    13,   307,    -1,    -1,   305,   308,    -1,
      -1,   146,    -1,   147,    -1,   172,   285,   173,    -1,   147,
      -1,   172,   285,   173,    -1,   301,    -1,   310,    -1,   309,
      26,   310,    -1,   309,    43,   310,    -1,   185,    -1,    64,
      -1,    98,    -1,    99,    -1,   100,    -1,   145,    -1,   101,
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
      -1,   106,    -1,   113,    -1,   123,    -1,   124,    -1,     9,
      -1,    11,    -1,    10,    -1,   125,    -1,   127,    -1,   126,
      -1,   128,    -1,   129,    -1,   143,    -1,   142,    -1,   154,
      -1,   156,    -1,   167,    -1,   191,   169,   244,   170,    -1,
     192,    -1,   146,    -1,   312,    -1,   111,    -1,   352,    -1,
     312,    -1,   111,    -1,   356,    -1,   169,   170,    -1,   279,
      -1,    -1,    -1,    78,    -1,   365,    -1,   169,   244,   170,
      -1,    -1,    69,    -1,    70,    -1,    79,    -1,   128,    -1,
     129,    -1,   143,    -1,   125,    -1,   156,    -1,   126,    -1,
     127,    -1,   142,    -1,   136,    78,   137,    -1,   136,   137,
      -1,   318,    -1,   190,    -1,    42,   319,    -1,    43,   319,
      -1,   124,   169,   322,   170,    -1,   166,   169,   292,   170,
      -1,   320,    -1,   296,    -1,   192,   140,   185,    -1,   146,
     140,   185,    -1,   190,    -1,    72,    -1,   370,    -1,   318,
      -1,   177,   365,   177,    -1,   178,   365,   178,    -1,   136,
     365,   137,    -1,   325,   323,    -1,    -1,     8,    -1,    -1,
       8,    -1,    -1,   325,     8,   319,   122,   319,    -1,   325,
       8,   319,    -1,   319,   122,   319,    -1,   319,    -1,    69,
      -1,    70,    -1,    79,    -1,   136,    78,   137,    -1,   136,
     137,    -1,    69,    -1,    70,    -1,   185,    -1,   326,    -1,
     185,    -1,    42,   327,    -1,    43,   327,    -1,   124,   169,
     329,   170,    -1,    61,   329,   176,    -1,   166,   169,   332,
     170,    -1,   330,   323,    -1,    -1,   330,     8,   328,   122,
     328,    -1,   330,     8,   328,    -1,   328,   122,   328,    -1,
     328,    -1,   331,     8,   328,    -1,   328,    -1,   333,   323,
      -1,    -1,   333,     8,    79,   122,   328,    -1,    79,   122,
     328,    -1,   331,   323,    -1,    -1,   169,   334,   170,    -1,
      -1,   336,     8,   185,   335,    -1,   185,   335,    -1,    -1,
     338,   336,   323,    -1,    41,   337,    40,    -1,   339,    -1,
      -1,   342,    -1,   121,   351,    -1,   121,   185,    -1,   121,
     172,   285,   173,    -1,    61,   354,   176,    -1,   172,   285,
     173,    -1,   347,   343,    -1,   169,   278,   170,   343,    -1,
     357,   343,    -1,   169,   278,   170,   343,    -1,   351,    -1,
     311,    -1,   349,    -1,   350,    -1,   344,    -1,   346,   341,
      -1,   169,   278,   170,   341,    -1,   313,   140,   351,    -1,
     348,   169,   244,   170,    -1,   169,   346,   170,    -1,   311,
      -1,   349,    -1,   350,    -1,   344,    -1,   346,   342,    -1,
     169,   278,   170,   342,    -1,   348,   169,   244,   170,    -1,
     169,   346,   170,    -1,   351,    -1,   344,    -1,   169,   346,
     170,    -1,   346,   121,   185,   374,   169,   244,   170,    -1,
     346,   121,   351,   169,   244,   170,    -1,   346,   121,   172,
     285,   173,   169,   244,   170,    -1,   169,   278,   170,   121,
     185,   374,   169,   244,   170,    -1,   169,   278,   170,   121,
     351,   169,   244,   170,    -1,   169,   278,   170,   121,   172,
     285,   173,   169,   244,   170,    -1,   313,   140,   185,   374,
     169,   244,   170,    -1,   313,   140,   351,   169,   244,   170,
      -1,   352,    -1,   355,   352,    -1,   352,    61,   354,   176,
      -1,   352,   172,   285,   173,    -1,   353,    -1,    73,    -1,
     174,   172,   285,   173,    -1,   285,    -1,    -1,   174,    -1,
     355,   174,    -1,   351,    -1,   345,    -1,   356,   341,    -1,
     169,   278,   170,   341,    -1,   313,   140,   351,    -1,   169,
     346,   170,    -1,    -1,   345,    -1,   356,   342,    -1,   169,
     278,   170,   342,    -1,   169,   346,   170,    -1,   358,     8,
      -1,   358,     8,   346,    -1,   358,     8,   123,   169,   358,
     170,    -1,    -1,   346,    -1,   123,   169,   358,   170,    -1,
     360,   323,    -1,    -1,   360,     8,   285,   122,   285,    -1,
     360,     8,   285,    -1,   285,   122,   285,    -1,   285,    -1,
     360,     8,   285,   122,    31,   346,    -1,   360,     8,    31,
     346,    -1,   285,   122,    31,   346,    -1,    31,   346,    -1,
     362,   323,    -1,    -1,   362,     8,   285,   122,   285,    -1,
     362,     8,   285,    -1,   285,   122,   285,    -1,   285,    -1,
     364,   323,    -1,    -1,   364,     8,   319,   122,   319,    -1,
     364,     8,   319,    -1,   319,   122,   319,    -1,   319,    -1,
     365,   366,    -1,   365,    78,    -1,   366,    -1,    78,   366,
      -1,    73,    -1,    73,    61,   367,   176,    -1,    73,   121,
     185,    -1,   138,   285,   173,    -1,   138,    72,    61,   285,
     176,   173,    -1,   139,   346,   173,    -1,   185,    -1,    74,
      -1,    73,    -1,   114,   169,   369,   170,    -1,   115,   169,
     346,   170,    -1,     7,   285,    -1,     6,   285,    -1,     5,
     169,   285,   170,    -1,     4,   285,    -1,     3,   285,    -1,
     346,    -1,   369,     8,   346,    -1,   313,   140,   185,    -1,
     167,   185,    13,   383,   171,    -1,   185,    -1,   383,   185,
      -1,   185,    -1,   185,   162,   378,   163,    -1,   162,   375,
     163,    -1,    -1,   383,    -1,   375,     8,   383,    -1,   375,
       8,   157,    -1,   375,    -1,   157,    -1,    -1,    -1,    26,
     383,    -1,   185,     8,   378,    -1,   185,    -1,   185,    90,
     382,   378,    -1,   185,    90,   185,     8,   378,    -1,   185,
      90,   185,    -1,   185,    90,   382,    -1,    79,   122,   383,
      -1,   380,     8,   379,    -1,   379,    -1,   380,   323,    -1,
      -1,   166,   169,   381,   170,    -1,    25,   383,    -1,    51,
     383,    -1,   185,   374,    -1,   124,    -1,   382,    -1,   124,
     162,   383,   163,    -1,   124,   162,   383,     8,   383,   163,
      -1,   146,    -1,   169,    98,   169,   376,   170,    26,   383,
     170,    -1,   169,   375,     8,   383,   170,    -1,   383,    -1,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   820,   820,   825,   827,   830,   832,   833,   834,   835,
     836,   837,   839,   839,   841,   841,   843,   844,   849,   850,
     851,   852,   853,   854,   858,   860,   863,   864,   865,   866,
     871,   872,   876,   877,   878,   883,   888,   894,   900,   903,
     908,   910,   913,   914,   915,   916,   919,   920,   924,   929,
     929,   933,   933,   938,   937,   941,   941,   944,   945,   946,
     947,   948,   949,   950,   951,   952,   953,   954,   955,   956,
     959,   957,   962,   964,   972,   975,   976,   980,   981,   982,
     983,   984,   991,   997,  1001,  1001,  1007,  1008,  1012,  1013,
    1017,  1022,  1021,  1031,  1030,  1043,  1042,  1061,  1059,  1078,
    1077,  1086,  1084,  1096,  1095,  1106,  1104,  1116,  1117,  1121,
    1124,  1127,  1128,  1129,  1132,  1134,  1137,  1138,  1141,  1142,
    1145,  1146,  1150,  1151,  1156,  1157,  1160,  1161,  1165,  1166,
    1170,  1171,  1175,  1176,  1180,  1181,  1186,  1187,  1192,  1193,
    1194,  1195,  1198,  1201,  1203,  1206,  1207,  1211,  1213,  1216,
    1219,  1222,  1223,  1226,  1227,  1231,  1233,  1235,  1236,  1240,
    1242,  1244,  1247,  1250,  1253,  1256,  1260,  1267,  1269,  1272,
    1273,  1274,  1276,  1281,  1282,  1285,  1286,  1287,  1291,  1292,
    1294,  1295,  1299,  1301,  1304,  1304,  1308,  1307,  1311,  1315,
    1313,  1326,  1323,  1334,  1336,  1338,  1340,  1342,  1346,  1347,
    1348,  1351,  1357,  1360,  1366,  1369,  1374,  1376,  1381,  1386,
    1390,  1391,  1397,  1398,  1403,  1404,  1409,  1410,  1414,  1415,
    1419,  1421,  1427,  1432,  1433,  1435,  1439,  1440,  1441,  1442,
    1446,  1447,  1448,  1449,  1450,  1451,  1453,  1458,  1461,  1462,
    1466,  1467,  1470,  1471,  1474,  1475,  1478,  1479,  1483,  1484,
    1485,  1486,  1487,  1488,  1491,  1493,  1495,  1496,  1499,  1501,
    1505,  1507,  1511,  1515,  1516,  1520,  1521,  1525,  1529,  1533,
    1538,  1539,  1540,  1543,  1545,  1546,  1547,  1550,  1551,  1552,
    1553,  1554,  1555,  1556,  1557,  1558,  1559,  1560,  1561,  1562,
    1563,  1564,  1565,  1566,  1567,  1568,  1569,  1570,  1571,  1572,
    1573,  1574,  1575,  1576,  1577,  1578,  1579,  1580,  1581,  1582,
    1583,  1584,  1585,  1586,  1587,  1588,  1589,  1590,  1591,  1593,
    1594,  1596,  1598,  1599,  1600,  1601,  1602,  1603,  1604,  1605,
    1606,  1607,  1608,  1609,  1610,  1611,  1612,  1613,  1614,  1615,
    1617,  1616,  1625,  1624,  1632,  1633,  1634,  1638,  1643,  1650,
    1655,  1662,  1664,  1668,  1670,  1674,  1679,  1680,  1684,  1691,
    1698,  1700,  1705,  1706,  1707,  1711,  1715,  1719,  1720,  1721,
    1722,  1726,  1732,  1737,  1746,  1747,  1750,  1753,  1756,  1757,
    1760,  1764,  1767,  1770,  1777,  1778,  1782,  1783,  1785,  1789,
    1790,  1791,  1792,  1793,  1794,  1795,  1796,  1797,  1798,  1799,
    1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,  1808,  1809,
    1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,  1818,  1819,
    1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,  1828,  1829,
    1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,  1838,  1839,
    1840,  1841,  1842,  1843,  1844,  1845,  1846,  1847,  1848,  1849,
    1850,  1851,  1852,  1853,  1854,  1855,  1856,  1857,  1858,  1859,
    1860,  1861,  1862,  1866,  1871,  1872,  1875,  1876,  1877,  1881,
    1882,  1883,  1887,  1888,  1889,  1893,  1894,  1895,  1898,  1900,
    1904,  1905,  1906,  1908,  1909,  1910,  1911,  1912,  1913,  1914,
    1915,  1916,  1919,  1924,  1925,  1926,  1927,  1928,  1930,  1933,
    1934,  1938,  1941,  1947,  1948,  1949,  1950,  1951,  1952,  1953,
    1958,  1960,  1964,  1965,  1968,  1969,  1973,  1976,  1978,  1980,
    1984,  1985,  1986,  1988,  1991,  1995,  1996,  1997,  2000,  2001,
    2002,  2003,  2004,  2006,  2007,  2013,  2015,  2018,  2021,  2023,
    2025,  2028,  2030,  2034,  2036,  2039,  2043,  2050,  2052,  2055,
    2056,  2061,  2064,  2068,  2068,  2073,  2076,  2077,  2081,  2082,
    2087,  2088,  2092,  2093,  2097,  2098,  2102,  2104,  2108,  2109,
    2110,  2111,  2112,  2113,  2114,  2115,  2118,  2120,  2124,  2125,
    2126,  2127,  2128,  2130,  2132,  2134,  2138,  2139,  2140,  2144,
    2147,  2150,  2153,  2156,  2159,  2165,  2169,  2176,  2177,  2182,
    2184,  2185,  2188,  2189,  2192,  2193,  2197,  2198,  2202,  2203,
    2204,  2205,  2206,  2209,  2212,  2213,  2214,  2216,  2218,  2222,
    2223,  2224,  2226,  2227,  2228,  2232,  2234,  2237,  2239,  2240,
    2241,  2242,  2245,  2247,  2248,  2252,  2254,  2257,  2259,  2260,
    2261,  2265,  2267,  2270,  2273,  2275,  2277,  2281,  2282,  2284,
    2285,  2291,  2292,  2294,  2296,  2298,  2300,  2303,  2304,  2305,
    2309,  2310,  2311,  2312,  2313,  2314,  2315,  2319,  2320,  2324,
    2333,  2338,  2339,  2345,  2346,  2354,  2357,  2361,  2364,  2369,
    2370,  2371,  2372,  2376,  2377,  2381,  2382,  2383,  2385,  2387,
    2388,  2392,  2398,  2400,  2404,  2407,  2410,  2419,  2422,  2425,
    2426,  2429,  2430,  2434,  2439,  2443,  2449,  2457,  2458
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
  "T_TRAIT", "T_INSTEADOF", "T_TRAIT_C", "T_VARARG", "T_STRICT_ERROR",
  "T_FINALLY", "T_XHP_TAG_LT", "T_XHP_TAG_GT", "T_TYPELIST_LT",
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "T_SHAPE", "T_TYPE",
  "T_UNRESOLVED_TYPE", "'('", "')'", "';'", "'{'", "'}'", "'$'", "'`'",
  "']'", "'\"'", "'\\''", "$accept", "start", "top_statement_list",
  "top_statement", "$@1", "$@2", "ident", "use_declarations",
  "use_declaration", "namespace_name", "namespace_string_base",
  "namespace_string", "namespace_string_typeargs",
  "class_namespace_string_typeargs", "constant_declaration",
  "inner_statement_list", "inner_statement", "statement", "$@3", "$@4",
  "$@5", "$@6", "$@7", "additional_catches", "finally", "$@8",
  "optional_finally", "is_reference", "function_loc",
  "function_declaration_statement", "$@9", "$@10",
  "class_declaration_statement", "$@11", "$@12", "$@13", "$@14",
  "trait_declaration_statement", "$@15", "$@16", "class_decl_name",
  "interface_decl_name", "trait_decl_name", "class_entry_type",
  "extends_from", "implements_list", "interface_extends_list",
  "interface_list", "trait_list", "foreach_optional_arg",
  "foreach_variable", "for_statement", "foreach_statement",
  "while_statement", "declare_statement", "declare_list",
  "switch_case_list", "case_list", "case_separator", "elseif_list",
  "new_elseif_list", "else_single", "new_else_single", "parameter_list",
  "non_empty_parameter_list", "function_call_parameter_list",
  "non_empty_fcall_parameter_list", "global_var_list", "global_var",
  "static_var_list", "class_statement_list", "class_statement", "$@17",
  "$@18", "$@19", "$@20", "trait_rules", "trait_precedence_rule",
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
  "expr_no_variable", "$@21", "$@22", "non_empty_shape_pair_list",
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
  "user_attribute_list", "$@23", "non_empty_user_attributes",
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
  "class_constant", "sm_typedef_statement", "sm_name_with_type",
  "sm_name_with_typevar", "sm_typeargs_opt", "sm_type_list",
  "sm_func_type_list", "sm_opt_return_type", "sm_typevar_list",
  "sm_shape_member_type", "sm_non_empty_shape_member_list",
  "sm_shape_member_list", "sm_shape_type", "sm_type", "sm_type_opt", 0
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
     396,   397,   398,   399,   400,   401,   402,   403,   404,    40,
      41,    59,   123,   125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   179,   180,   181,   181,   182,   182,   182,   182,   182,
     182,   182,   183,   182,   184,   182,   182,   182,   185,   185,
     185,   185,   185,   185,   186,   186,   187,   187,   187,   187,
     188,   188,   189,   189,   189,   190,   191,   192,   193,   193,
     194,   194,   195,   195,   195,   195,   196,   196,   196,   197,
     196,   198,   196,   199,   196,   200,   196,   196,   196,   196,
     196,   196,   196,   196,   196,   196,   196,   196,   196,   196,
     201,   196,   196,   196,   196,   196,   196,   196,   196,   196,
     196,   196,   202,   202,   204,   203,   205,   205,   206,   206,
     207,   209,   208,   210,   208,   212,   211,   213,   211,   214,
     211,   215,   211,   217,   216,   218,   216,   219,   219,   220,
     221,   222,   222,   222,   223,   223,   224,   224,   225,   225,
     226,   226,   227,   227,   228,   228,   229,   229,   230,   230,
     231,   231,   232,   232,   233,   233,   234,   234,   235,   235,
     235,   235,   236,   236,   236,   237,   237,   238,   238,   239,
     239,   240,   240,   241,   241,   242,   242,   242,   242,   243,
     243,   243,   243,   243,   243,   243,   243,   244,   244,   245,
     245,   245,   245,   246,   246,   247,   247,   247,   248,   248,
     248,   248,   249,   249,   251,   250,   252,   250,   250,   253,
     250,   254,   250,   250,   250,   250,   250,   250,   255,   255,
     255,   256,   257,   257,   258,   258,   259,   259,   260,   260,
     261,   261,   261,   261,   262,   262,   263,   263,   264,   264,
     265,   265,   266,   267,   267,   267,   268,   268,   268,   268,
     269,   269,   269,   269,   269,   269,   269,   270,   270,   270,
     271,   271,   272,   272,   273,   273,   274,   274,   275,   275,
     275,   275,   275,   275,   276,   276,   276,   276,   277,   277,
     278,   278,   279,   280,   280,   281,   281,   282,   283,   284,
     285,   285,   285,   286,   286,   286,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     287,   286,   288,   286,   286,   286,   286,   289,   289,   290,
     290,   291,   291,   292,   292,   293,   294,   294,   295,   296,
     297,   297,   298,   298,   298,   299,   299,   300,   300,   300,
     300,   301,   302,   302,   303,   303,   304,   304,   305,   305,
     306,   307,   307,   308,   308,   308,   309,   309,   309,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   310,   310,   310,   310,   310,   310,   310,
     310,   310,   310,   311,   312,   312,   313,   313,   313,   314,
     314,   314,   315,   315,   315,   316,   316,   316,   317,   317,
     318,   318,   318,   318,   318,   318,   318,   318,   318,   318,
     318,   318,   318,   319,   319,   319,   319,   319,   319,   319,
     319,   320,   320,   321,   321,   321,   321,   321,   321,   321,
     322,   322,   323,   323,   324,   324,   325,   325,   325,   325,
     326,   326,   326,   326,   326,   327,   327,   327,   328,   328,
     328,   328,   328,   328,   328,   329,   329,   330,   330,   330,
     330,   331,   331,   332,   332,   333,   333,   334,   334,   335,
     335,   336,   336,   338,   337,   339,   340,   340,   341,   341,
     342,   342,   343,   343,   344,   344,   345,   345,   346,   346,
     346,   346,   346,   346,   346,   346,   346,   346,   347,   347,
     347,   347,   347,   347,   347,   347,   348,   348,   348,   349,
     349,   349,   349,   349,   349,   350,   350,   351,   351,   352,
     352,   352,   353,   353,   354,   354,   355,   355,   356,   356,
     356,   356,   356,   356,   357,   357,   357,   357,   357,   358,
     358,   358,   358,   358,   358,   359,   359,   360,   360,   360,
     360,   360,   360,   360,   360,   361,   361,   362,   362,   362,
     362,   363,   363,   364,   364,   364,   364,   365,   365,   365,
     365,   366,   366,   366,   366,   366,   366,   367,   367,   367,
     368,   368,   368,   368,   368,   368,   368,   369,   369,   370,
     371,   372,   372,   373,   373,   374,   374,   375,   375,   376,
     376,   376,   376,   377,   377,   378,   378,   378,   378,   378,
     378,   379,   380,   380,   381,   381,   382,   383,   383,   383,
     383,   383,   383,   383,   383,   383,   383,   384,   384
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     1,     1,     1,
       4,     3,     0,     6,     0,     5,     3,     2,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     2,     3,     4,
       1,     3,     1,     2,     3,     1,     2,     2,     5,     4,
       2,     0,     1,     1,     1,     1,     3,     5,     8,     0,
       4,     0,     6,     0,    10,     0,     4,     2,     3,     2,
       3,     2,     3,     3,     3,     3,     3,     5,     1,     1,
       0,     9,     5,    14,     5,     3,     3,     2,     2,     2,
       2,     2,     9,     0,     0,     5,     1,     0,     1,     0,
       1,     0,    11,     0,    12,     0,     8,     0,     9,     0,
       7,     0,     8,     0,     6,     0,     7,     1,     1,     1,
       1,     1,     2,     2,     2,     0,     2,     0,     2,     0,
       1,     3,     1,     3,     2,     0,     1,     2,     1,     4,
       1,     4,     1,     4,     1,     4,     3,     5,     3,     4,
       4,     5,     5,     4,     0,     1,     1,     4,     0,     5,
       0,     2,     0,     3,     0,     3,     2,     1,     0,     3,
       4,     6,     5,     5,     6,     8,     7,     2,     0,     1,
       2,     3,     4,     3,     1,     1,     2,     4,     3,     5,
       1,     3,     2,     0,     0,     4,     0,     5,     2,     0,
      10,     0,    11,     3,     3,     3,     3,     5,     2,     2,
       0,     6,     5,     4,     3,     1,     1,     3,     4,     1,
       1,     1,     1,     4,     1,     3,     2,     0,     2,     0,
       1,     3,     1,     1,     1,     1,     3,     4,     4,     4,
       1,     1,     2,     2,     2,     3,     3,     1,     1,     1,
       1,     3,     1,     1,     1,     0,     1,     2,     1,     1,
       1,     1,     1,     1,     3,     5,     1,     3,     5,     4,
       3,     3,     3,     3,     1,     1,     0,     2,     3,     6,
       1,     1,     1,     6,     3,     4,     6,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     5,     4,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     3,     2,
       0,    11,     0,    12,     1,     1,     1,     5,     3,     5,
       3,     2,     0,     2,     0,     4,     4,     3,     4,     4,
       4,     4,     1,     1,     3,     5,     0,     3,     4,     1,
       2,     4,     2,     6,     0,     1,     4,     0,     2,     0,
       1,     1,     3,     1,     3,     1,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     0,     0,     1,     1,     3,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     2,     1,     1,     2,     2,     4,     4,     1,
       1,     3,     3,     1,     1,     1,     1,     3,     3,     3,
       2,     0,     1,     0,     1,     0,     5,     3,     3,     1,
       1,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       2,     2,     4,     3,     4,     2,     0,     5,     3,     3,
       1,     3,     1,     2,     0,     5,     3,     2,     0,     3,
       0,     4,     2,     0,     3,     3,     1,     0,     1,     2,
       2,     4,     3,     3,     2,     4,     2,     4,     1,     1,
       1,     1,     1,     2,     4,     3,     4,     3,     1,     1,
       1,     1,     2,     4,     4,     3,     1,     1,     3,     7,
       6,     8,     9,     8,    10,     7,     6,     1,     2,     4,
       4,     1,     1,     4,     1,     0,     1,     2,     1,     1,
       2,     4,     3,     3,     0,     1,     2,     4,     3,     2,
       3,     6,     0,     1,     4,     2,     0,     5,     3,     3,
       1,     6,     4,     4,     2,     2,     0,     5,     3,     3,
       1,     2,     0,     5,     3,     3,     1,     2,     2,     1,
       2,     1,     4,     3,     3,     6,     3,     1,     1,     1,
       4,     4,     2,     2,     4,     2,     2,     1,     3,     3,
       5,     1,     2,     1,     4,     3,     0,     1,     3,     3,
       1,     1,     0,     0,     2,     3,     1,     4,     5,     3,
       3,     3,     3,     1,     2,     0,     4,     2,     2,     2,
       1,     1,     4,     6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     2,     1,     0,     0,     0,     0,     0,     0,
     553,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   626,     0,   614,   474,     0,
     480,   481,    18,   504,   602,    69,   482,     0,    51,     0,
       0,     0,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,     0,     0,     0,     0,   467,     0,     0,     0,
       0,   111,     0,     0,     0,   486,   488,   489,   483,   484,
       0,     0,   490,   485,     0,     0,   465,    19,    20,    21,
      23,    22,     0,   487,     0,     0,     0,     0,    68,    41,
     606,   475,     0,     0,     3,    30,    32,    35,   503,     0,
     464,     0,     5,    89,     6,     7,     8,     0,   272,     0,
       0,     0,     0,   270,   337,   336,   346,   345,     0,   344,
     569,   466,     0,   506,   335,     0,   572,   271,     0,     0,
     570,   571,   568,   597,   601,     0,   325,   505,     9,   467,
       0,     0,    30,    89,   666,   271,   665,     0,   663,   662,
     339,     0,     0,   309,   310,   311,   312,   334,   332,   331,
     330,   329,   328,   327,   326,   467,     0,   676,   466,     0,
     292,   290,     0,   630,     0,   513,   277,   470,     0,   676,
     469,     0,   479,   609,   608,   471,     0,     0,   473,   333,
       0,     0,     0,   264,     0,    49,   266,     0,     0,    55,
      57,     0,     0,    59,     0,     0,     0,   700,   704,     0,
       0,   676,     0,   701,     0,    61,     0,    41,     0,     0,
       0,    25,    26,   175,     0,     0,   174,   113,   112,   180,
      89,     0,     0,     0,     0,     0,   673,    99,   109,   622,
     626,   651,     0,   492,     0,     0,     0,   649,     0,    14,
       0,    33,     0,   267,   103,   110,   377,   352,     0,   272,
       0,   270,   271,     0,     0,   476,     0,   477,     0,     0,
       0,    81,     0,     0,    37,   168,     0,    17,    88,     0,
     108,    95,   107,    78,    79,    80,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   614,    77,   605,   605,   636,     0,     0,     0,    89,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   291,   289,     0,   573,   558,   605,     0,
     564,   168,   605,     0,   607,   598,   622,     0,     0,     0,
     555,   550,   513,     0,     0,     0,     0,   634,     0,   357,
     512,   625,     0,     0,    37,     0,   168,   260,     0,   610,
     558,   566,   472,     0,    41,   148,     0,    66,     0,     0,
     265,     0,     0,     0,     0,     0,    58,    76,    60,   676,
     697,   698,     0,   695,     0,     0,   677,   699,     0,   672,
      62,     0,    75,    27,     0,    16,     0,     0,   176,     0,
      64,     0,     0,     0,    65,   667,     0,     0,     0,     0,
       0,   119,     0,   623,     0,     0,     0,     0,   491,   650,
     504,     0,     0,   648,   509,   647,    34,     4,    11,    12,
      63,     0,     0,     0,     0,   513,     0,     0,   261,   322,
     577,    46,    40,    42,    43,    44,    45,     0,   338,   507,
     508,    31,     0,     0,     0,   515,   169,     0,   340,    91,
     115,   295,   297,   296,     0,     0,   293,   294,   298,   300,
     299,   314,   313,   316,   315,   317,   319,   320,   318,   308,
     307,   302,   303,   301,   304,   305,   306,   321,   604,     0,
       0,   640,     0,   513,   669,   575,   597,   101,   105,     0,
      97,     0,     0,   268,   274,   288,   287,   286,   285,   284,
     283,   282,   281,   280,   279,   278,     0,   560,   559,     0,
       0,     0,     0,     0,     0,   664,   548,   552,   512,   554,
       0,     0,   676,     0,   629,     0,   628,     0,   613,   612,
       0,     0,   560,   559,   262,   150,   152,   263,     0,    41,
     132,    50,   266,     0,     0,     0,     0,   144,   144,    56,
       0,     0,   693,   513,     0,   682,     0,     0,     0,     0,
       0,   465,     0,    35,   494,   464,   500,     0,   493,    39,
     499,    84,     0,    24,    28,     0,   173,   181,   342,   178,
       0,     0,   660,   661,    10,   686,     0,     0,     0,   622,
     619,     0,   356,   659,   658,   657,     0,   653,     0,   654,
     656,     0,     4,   183,   371,   372,   380,   379,     0,     0,
     512,   351,   355,     0,     0,   574,   558,   565,   603,     0,
     675,   170,   463,   514,   167,     0,   557,     0,     0,   117,
     324,     0,   360,   361,     0,   358,   512,   635,     0,   168,
     119,     0,    93,   115,   614,   275,     0,     0,   168,   562,
     563,   576,   599,   600,     0,     0,     0,   536,   520,   521,
     522,     0,     0,     0,   529,   528,   542,   513,     0,   550,
     633,   632,     0,   611,   558,   567,   478,     0,   154,     0,
       0,    47,     0,     0,     0,     0,   125,   126,   136,     0,
      41,   134,    72,   144,     0,   144,     0,     0,   702,     0,
     512,   694,   696,   681,   680,     0,   678,   495,   496,   511,
       0,     0,   354,     0,   642,     0,    74,     0,    29,   177,
     557,     0,   668,    67,     0,     0,   674,   118,   120,   183,
       0,     0,   620,     0,   652,     0,    15,     0,   245,     0,
       0,   348,     0,   670,     0,   560,   559,   678,     0,   171,
      38,   157,     0,   515,   556,   708,   557,   114,     0,     0,
     323,   639,   638,   168,     0,     0,   183,     0,   117,   479,
     561,   168,     0,     0,   525,   526,   527,   530,   531,   540,
       0,   513,   536,     0,   524,   544,   512,   547,   549,   551,
       0,   627,   561,     0,     0,     0,     0,   151,    52,     0,
     266,   127,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   138,     0,   691,   692,     0,     0,   706,   519,     0,
     513,   502,     0,   513,     0,   501,   646,     0,   513,     0,
       0,     0,   179,   685,   689,   690,     0,   245,   624,   622,
     269,   273,     0,    13,     0,     0,   248,   249,   250,   253,
     252,   251,   243,     0,     0,     0,   104,   182,   184,     0,
     242,   246,     0,   245,   383,     0,     0,   385,   378,   381,
       0,   376,     0,     0,     0,   168,   172,   683,   557,   156,
     707,     0,     0,   116,   183,     0,     0,   596,   183,   245,
     557,     0,   276,   168,     0,   590,     0,   533,   512,   535,
       0,   523,     0,     0,   513,   541,   631,     0,    41,     0,
     147,   133,     0,   124,    70,   137,     0,     0,   140,     0,
     145,   146,    41,   139,   703,   679,     0,     0,   497,   512,
     510,     0,   512,   353,   498,     0,   359,   512,   641,     0,
      41,   683,     0,   687,   121,   100,     0,   655,     0,     0,
     122,   212,   210,   465,    23,     0,   206,     0,   211,   222,
       0,   220,   225,     0,   224,     0,   223,     0,    89,   247,
     186,     0,   188,     0,   244,   374,     0,     0,   347,   561,
     168,     0,     0,   366,   155,   708,     0,   159,   683,   245,
     637,   595,   245,   106,     0,   183,     0,   589,   539,   538,
     532,     0,   534,   512,   543,    41,   153,    48,    53,     0,
     135,   141,    41,   143,     0,   518,   517,   350,     0,   645,
     644,     0,     0,   366,   688,   621,     0,     0,   196,   200,
       0,     0,   193,   434,   433,   430,   432,   431,   450,   452,
     451,   422,   412,   428,   427,   390,   399,   400,   402,   401,
     421,   405,   403,   404,   406,   407,   408,   409,   410,   411,
     413,   414,   415,   416,   417,   418,   420,   419,   391,   392,
     393,   395,   396,   398,   436,   437,   446,   445,   444,   443,
     442,   441,   429,   447,   438,   439,   440,   423,   424,   425,
     426,   448,   449,   453,   455,   454,   456,   457,   435,   459,
     458,   394,   460,   461,   397,   462,   389,   217,   386,     0,
     194,   238,   239,   237,   230,     0,   231,   195,   256,     0,
       0,     0,     0,    89,   375,   373,   384,   382,   168,     0,
     593,   684,     0,     0,     0,   160,     0,     0,    96,   102,
     683,   245,   591,     0,   546,     0,   149,     0,    41,   130,
      71,   142,   705,     0,     0,     0,     0,    85,     0,   259,
     123,     0,     0,   214,   207,     0,     0,     0,   219,   221,
       0,     0,   226,   233,   234,   232,     0,     0,   185,     0,
       0,     0,     0,     0,   592,     0,    41,     0,   163,     0,
     162,    41,     0,    98,   537,     0,    41,   128,    54,     0,
     516,   349,   643,    41,    41,   197,    30,     0,   198,   199,
       0,     0,   213,   216,   387,   388,     0,   208,   235,   236,
     228,   229,   227,   257,   254,   189,   187,   258,     0,   594,
       0,   369,   515,     0,   164,     0,   161,     0,    41,   545,
       0,     0,     0,     0,     0,   245,   215,   218,     0,   557,
     191,   370,   514,     0,   341,     0,   166,    92,     0,     0,
     131,    83,   343,   204,     0,   244,   255,     0,   557,     0,
     367,   365,   165,    94,   129,    87,     0,     0,   203,   683,
       0,   368,     0,    86,    73,     0,   202,     0,   683,     0,
     201,   240,    41,   190,     0,     0,     0,   192,     0,   241,
       0,    41,     0,    82
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    94,   622,   437,   142,   220,   221,    96,
      97,    98,    99,   100,   101,   263,   452,   453,   379,   194,
    1167,   385,  1029,  1295,   736,   737,  1304,   279,   143,   454,
     647,   787,   455,   470,   663,   421,   660,   456,   441,   661,
     281,   237,   254,   107,   649,   779,   608,   747,   969,   823,
     706,  1218,  1170,   561,   712,   384,   569,   714,   942,   556,
     698,   701,   815,   772,   773,   464,   465,   225,   226,   231,
     758,   877,   987,  1141,  1269,  1288,  1181,  1228,  1229,  1230,
     975,   976,   977,  1182,  1188,  1237,   980,   981,   985,  1134,
    1135,  1136,  1313,   878,   879,   880,   881,  1139,   882,   108,
     188,   380,   381,   109,   110,   111,   112,   113,   646,   740,
     445,   843,   446,   844,   114,   115,   116,   586,   117,   118,
    1153,  1252,   119,   442,  1145,   443,   759,   628,   891,   888,
    1127,  1128,   120,   121,   122,   182,   189,   266,   367,   123,
     589,   590,   124,   839,   361,   644,   840,   685,   797,   799,
     800,   801,   687,   923,   924,   688,   537,   352,   151,   152,
     125,   775,   336,   337,   637,   126,   183,   145,   128,   129,
     130,   131,   132,   133,   134,   499,   135,   185,   186,   424,
     174,   175,   502,   503,   847,   848,   246,   247,   616,   136,
     416,   137,   138,   212,   238,   274,   395,   725,  1003,   606,
     572,   573,   574,   213,   214,   901
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -888
static const yytype_int16 yypact[] =
{
    -888,   109,  3371,  -888,  8919,  8919,   -66,  8919,  8919,  8919,
    -888,  8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,
    8919,  8919,  8919,  9520,  9520,  6689,  8919,  9663,   -51,   148,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,  8919,  -888,   148,
     166,   186,   218,   148,  6847,   947,  7005,  -888,   643,  7163,
     -59,  8919,   846,   108,    19,    25,    43,   234,   242,   250,
     259,  -888,   947,   269,   272,  -888,  -888,  -888,  -888,  -888,
     366,   633,  -888,  -888,   947,  7321,  -888,  -888,  -888,  -888,
    -888,  -888,   947,  -888,    28,   279,   947,  8919,  -888,  -888,
     192,   222,   229,   229,  -888,   332,   228,   165,  -888,   282,
    -888,    39,  -888,   345,  -888,  -888,  -888,   901,  -888,   299,
     301,   315,  9570,  -888,  -888,   398,  -888,   407,   428,  -888,
      84,   327,   372,  -888,  -888,  1050,   -12,  2333,    86,   337,
     116,   118,   347,   -10,  -888,   129,  -888,   463,  -888,   433,
     370,   402,  -888,   345, 10261,  2546, 10261,  8919, 10261, 10261,
   10177,   508,   947,  -888,  -888,   501,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  1568,   390,  -888,   418,
     439,   439,  9520,  9921,   386,   559,  -888,   430,  1568,   390,
     431,   435,   399,   122,  -888,   459,    86,  7479,  -888,  -888,
    8919,  2529,    40, 10261,  6531,  -888,  8919,  8919,   947,  -888,
    -888,  9611,   412,  -888,  9672,   643,   643,   427,  -888,   429,
     531,    32,   587,  -888,   947,  -888,  9715,  -888,  9756,   947,
      42,  -888,    10,  -888,  2770,    44,  -888,  -888,  -888,   588,
     345,    45,  9520,  9520,  9520,   434,   437,  -888,  -888,  9125,
    6689,    27,   276,  -888,  9077,  9520,   449,  -888,   947,  -888,
     -33,   228,   432, 10261,  -888,  -888,  -888,   526,   594,   442,
   10261,   447,   460,  3529,  8919,   311,   443,   258,   311,   278,
     231,  -888,   947,   643,   440,  7655,   643,  -888,  -888,   339,
    -888,  -888,  -888,  -888,  -888,  -888,  8919,  8919,  8919,  7813,
    8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,
    8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,
    8919,  9663,  -888,  8919,  8919,  8919,   797,   947,   947,   345,
     901,  2709,  8919,  8919,  8919,  8919,  8919,  8919,  8919,  8919,
    8919,  8919,  8919,  -888,  -888,   349,  -888,   124,  8919,  8919,
    -888,  7655,  8919,  8919,   192,   126,  9125,   445,  7971,  9817,
    -888,   452,   614,  1568,   453,   -50,   797,   439,  8129,  -888,
    8287,  -888,   455,   -27,  -888,   189,  7655,  -888,   572,  -888,
     140,  -888,  -888,  9860,  -888,  -888,  8919,  -888,   551,  5899,
     626,   464, 10154,   623,    53,    69,  -888,  -888,  -888,   390,
    -888,  -888,   643,   560,   471,   634,  -888,  -888,  9868,  -888,
    -888,  3687,  -888,    14,   846,  -888,   947,  8919,   439,   108,
    -888,  9868,   472,   571,  -888,   439,    61,    62,   201,   476,
     947,   530,   484,   439,    74,   492,  1063,   947,  -888,  -888,
     593,  1374,    -7,  -888,  -888,  -888,   228,  -888,  -888,  -888,
    -888,   491,   503,    21,   545,   661,   500,   643,    70,   611,
      88,  -888,  -888,  -888,  -888,  -888,  -888,  1764,  -888,  -888,
    -888,  -888,    64,  9520,   511,   677, 10261,   676,  -888,  -888,
     573, 10301,  3202, 10177,  8919, 10220,  2119,  6523,  3362,  3519,
    3676,  3834,  3834,  3834,  3834,  2384,  2384,  2384,  2384,   498,
     498,   296,   296,   296,   501,   501,   501,  -888, 10261,   515,
     517, 10017,   522,   694,     1,   534,   126,  -888,  -888,   947,
    -888,  7525,  8919,  -888, 10177, 10177, 10177, 10177, 10177, 10177,
   10177, 10177, 10177, 10177, 10177, 10177,  8919,     1,   536,   532,
    2056,   537,   540,  2097,    77,  -888,   617,  -888,   947,  -888,
     442,    70,   390,  9520, 10261,  9520, 10058,   203,   156,  -888,
     542,  8919,  -888,  -888,  -888,  5741,   253, 10261,   148,  -888,
    -888,  -888,  8919,  1751,  9868,   947,  6057,   539,   548,  -888,
      73,   600,  -888,   720,   562,   864,   643,  9868,  9868,   561,
       0,   596,   579,   149,  -888,   609,  -888,   568,  -888,  -888,
    -888,   648,   947,  -888,  -888,  2863,  -888,  -888,  -888,   741,
    9520,   601,  -888,  -888,  -888,    75,   612,  1309,   604,  9125,
    9138,   746,  -888,  -888,  -888,  -888,   602,  -888,  8919,  -888,
    -888,  3037,  -888,  -888,  -888,  -888,  -888,  -888,   766,  8919,
     701,  -888,  -888,   622,   785,  -888,   207,  -888,  -888,   643,
    -888,   439,  -888,  8445,  -888,  9868,   110,   619,  1309,   678,
   10324,  8919,  -888,  -888,  8919,  -888,  8919,  -888,   628,  7655,
     530,   629,  -888,   573,  9663,   439,  2938,   637,  7655,  -888,
    -888,   223,  -888,  -888,   789,   834,   834,   617,  -888,  -888,
    -888,   638,    24,   639,  -888,  -888,  -888,   795,   641,   452,
     439,   439,  8603,  -888,   230,  -888,  -888,  2983,   281,   148,
    6531,  -888,   642,  3845,   647,  9520,   698,   439,  -888,   808,
    -888,  -888,  -888,  -888,   288,  -888,    13,   643,  -888,   643,
     560,  -888,  -888,  -888,   818,   654,   657,  -888,  -888,  9868,
     692,   947,   751,   947,  9868,   663,  -888,   675,  -888,  -888,
     110,  9868,   439,  -888,   947,   586,  -888,   827,  -888,  -888,
      79,   667,   439,  8761,  -888,  1080,  -888,  3213,  1387,   280,
     -28, 10261,   722,  -888,  8919,     1,   679,  -888,  9520, 10261,
    -888,  -888,   680,   838,  -888,   643,   110,  -888,  1309,   682,
   10324, 10261, 10113,  7655,   681,   687,  -888,   683,   678,   399,
     691,  7655,   693,  8919,  -888,  -888,  -888,  -888,  -888,   725,
     685,   856,   617,   729,  -888,   788,   617,  -888,  -888,  -888,
    9520, 10261,  -888,   148,   843,   803,  6531,  -888,  -888,   706,
    8919,   439,  1751,   709,  9868,  4003,   340,   716,  8919,    66,
     232,  -888,   727,  -888,  -888,  1400,   862,  -888,   769,   724,
     884,  -888,   777,   893,   737,  -888,   791,   738,   910,  1309,
     747,   754,  -888,  -888,   913,   947,  1309,  1855,  -888,  9125,
    -888, 10177,   755,  -888,   643,  1309,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  1909,   779,   444,  -888,  -888,  -888,   433,
     774,  -888,    47,   732,  -888,    22,  8919,  -888,  -888,  -888,
    8919,  -888,  8919,  9404,   761,  7655,   439,   905,   115,  -888,
    -888,    67,   771,   827,  -888,  8919,   772,  -888,  -888,  2132,
     110,   767,  -888,  7655,   781,  -888,   617,  -888,   617,  -888,
     782,  -888,   825,   784,   953,  -888,   439,   929,  -888,   792,
    -888,  -888,   794,  -888,  -888,  -888,   799,   806,  -888,  9527,
    -888,  -888,  -888,  -888,  -888,  -888,   643,  9868,  -888,  9868,
    -888,  9868,   883,  -888,  -888,  9868,  -888,  9868,  -888,   892,
    -888,   905,   947,  -888,  -888,  -888,    82,  -888,   961,    34,
    -888,  -888,  -888,    48,   812,    51,  -888,  9253,  -888,  -888,
      54,  -888,  -888,  1005,  -888,   807,  -888,   907,   345,  -888,
    -888,   643,  -888,   433,   732,   847,  9445,  9486, 10261,   823,
    7655,   832,   643,   902,  -888,   643,   932,   994,   905,  2153,
   10261,  -888,  2232,  -888,   841,  -888,   844,  -888,  -888,   898,
    -888,   617,  -888,   943,  -888,  -888,  5741,  -888,  -888,  6215,
    -888,  -888,  -888,  5741,   855,  -888,   904,  -888,   912,  -888,
     914,   859,  4161,   902,  -888,  -888,  9868,  1309,  -888,  -888,
    2926,  1909,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,   297,  -888,   779,
    -888,  -888,  -888,  -888,  -888,    50,   401,  -888,  1024,    55,
     947,   907,  1025,   345,  -888,  -888,  -888,  -888,  7655,   871,
    -888,  -888,   873,   872,    98,  1033,  9868,   879,  -888,  -888,
     905,  2265,  -888,   617,  -888,   933,  5741,  6373,  -888,  -888,
    -888,  5741,  -888,  9868,  9868,  9868,   887,  -888,   888,  -888,
    -888,   312,    36,  -888,  -888,  9868,  9253,  9253,  1013,  -888,
    1005,  1005,   469,  -888,  -888,  -888,  9868,   992,  -888,   897,
      57,  9868,   947,   899,  -888,   102,  -888,   998,  1054,  9868,
    -888,  -888,   906,  -888,  -888,   617,  -888,  -888,  -888,  4319,
    -888,  -888,  -888,  -888,  -888,  -888,   983,   937,  -888,  -888,
     989,  2926,  -888,  -888,  -888,  -888,   935,  -888,  1053,  -888,
    -888,  -888,  -888,  -888,  1079,  -888,  -888,  -888,   924,  -888,
    1021,  -888,  1093,  4477,  1089,  9868,  -888,  4635,  -888,  -888,
    4793,   957,  4951,  5109,   947,   732,  -888,  -888,  9868,   110,
    -888,  -888,   103,   934,  -888,  9868,  -888,  -888,  5267,   960,
    -888,  -888,  -888,   948,   947,    17,  -888,   963,   110,  1062,
    -888,  -888,  -888,  -888,  -888,   -13,  1309,   968,  -888,   905,
     970,  -888,   972,  -888,  -888,    58,  -888,   286,   905,  1309,
    -888,  -888,  -888,  -888,   286,  1070,  5425,  -888,   974,  -888,
     977,  -888,  5583,  -888
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -888,  -888,  -398,  -888,  -888,  -888,    -2,  -888,   748,     2,
     734,   659,  -888,   497,  -888,  -215,  -888,    -1,  -888,  -888,
    -888,  -888,  -888,  -888,  -133,  -888,  -888,  -139,    30,     3,
    -888,  -888,     5,  -888,  -888,  -888,  -888,     7,  -888,  -888,
     850,   854,   845,  1051,   512,   392,   518,   403,  -114,  -888,
     361,  -888,  -888,  -888,  -888,  -888,  -888,  -460,   247,  -888,
    -888,  -888,  -888,  -712,  -888,  -329,  -888,  -888,   778,  -888,
    -709,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,   139,  -888,  -888,  -888,  -888,  -888,    63,  -888,   316,
    -662,  -888,  -121,  -888,  -864,  -857,  -862,    65,  -888,   -56,
     -23,  1157,  -528,  -306,  -888,  -888,  2171,  1108,  -888,  -888,
    -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
     155,  -888,   446,  -888,  -888,  -888,  -888,  -888,  -888,  -888,
    -888,  -632,  -888,  1123,  1413,  -290,  -888,  -888,   410,  -299,
     796,  -888,  -888,  -888,  -330,  -759,  -888,  -888,   527,  -525,
     400,  -888,  -888,  -888,  -888,  -888,   519,  -888,  -888,  -888,
    -623,   320,  -149,  -147,  -111,  -888,  -888,     6,  -888,  -888,
    -888,  -888,    -3,  -102,  -888,   195,  -888,  -888,  -888,  -333,
     969,  -888,  -888,  -888,  -888,  -888,   486,   816,  -888,  -888,
     986,  -888,  -888,  -273,   -72,  -154,  -246,  -888,  -887,  -703,
     490,  -888,  -888,   475,  -109,   216
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -677
static const yytype_int16 yytable[] =
{
      95,   102,   401,   467,   347,   104,   191,   105,   127,   106,
     255,   686,   531,   534,   899,   513,   195,   340,   989,   993,
     199,   497,   539,   774,   184,   364,   994,   462,   851,   170,
     171,   259,   103,   345,   704,   282,   369,   550,   370,   621,
     857,   853,  1047,   202,  1231,  -671,   211,   276,   376,  -581,
     404,   342,   409,   413,   222,   991,  -209,   397,  1190,  1051,
     236,   565,  1129,  1197,   902,  1197,  1047,   625,   995,   600,
     600,   335,   639,   250,  1043,   371,   251,   909,   730,  1191,
     236,   717,   610,   744,   258,   610,   230,   610,   426,  1302,
     610,   412,   940,   262,   335,   567,   390,   391,  1006,   588,
     406,   396,   803,   147,   592,   236,   828,   829,   716,     3,
     354,   272,   588,   217,   335,   631,   229,   774,   187,   889,
     450,  1157,   362,   866,   867,   868,   869,   870,   871,  1207,
    -468,   338,   989,  1250,  1289,   883,   227,   243,   438,   439,
    1007,    47,   228,   548,   890,  -578,   -84,   338,   427,  -585,
     351,    10,   963,   774,   272,   319,    10,  -587,   272,   555,
    -581,   804,   343,   273,   396,   745,   620,   626,   256,   230,
    -676,  1208,   355,   657,   256,  1251,  1290,  -579,   357,  -580,
     509,   223,   627,  -615,   363,  -582,   831,   342,  1298,    95,
     375,   634,    95,   378,   273,  1009,   383,   127,  1014,  1012,
     127,  -616,    34,   389,   389,  1048,  1049,   469,   389,  1232,
     277,   377,   399,   405,   506,   410,   414,  -618,   992,  -209,
    1192,   403,  1052,   566,   757,  1130,  1198,   640,  1246,  1310,
     408,   601,   602,   506,   883,   397,   718,   941,   415,   415,
     418,   568,   339,   721,   611,   423,   255,   674,   282,   858,
     436,   432,  1045,   826,   506,   830,  -578,  -588,   339,  1044,
    -585,    95,    34,   506,   338,   588,   506,   771,  -583,   127,
     461,   389,  1004,  1212,   211,   774,   750,   236,   588,   588,
    -158,   925,   224,   570,  -584,  -514,   883,   774,  -579,  -676,
    -580,  -617,   932,   103,  -615,   241,  -582,   540,   343,   635,
     265,   636,   241,   344,   241,  -676,  1161,   268,   184,   433,
    1185,   273,  -616,   505,   504,   236,   236,   190,   236,   699,
     700,  -676,   335,  1186,   368,   828,   829,   273,  -618,   724,
     784,   241,   528,   527,  -676,   196,   433,  -676,   633,   792,
    1187,   308,   309,   310,   703,   311,   588,   813,   814,   241,
     658,   241,   423,   505,   542,   197,   433,   807,   271,   355,
     244,   245,   549,    90,   264,   553,   552,   244,   245,   244,
     245,   603,   272,   667,   789,   339,   278,    95,   560,  -583,
     827,   828,   829,    32,   241,   127,   883,   198,   658,   883,
     389,  1018,   635,  1019,   636,  -584,   244,   245,   693,    95,
     694,  1284,  -617,   232,   594,   943,   222,   127,  1285,   460,
      32,   233,  1307,   428,   244,   245,   244,   245,   605,   234,
      32,  1314,    34,   989,   615,   617,  1193,   884,   235,   364,
     588,   103,   937,   828,   829,   588,   695,   662,   239,   241,
     885,   240,   588,  1194,   242,   389,  1195,   860,   257,   244,
     245,   275,   886,   141,   906,   459,    74,  1311,  1312,  -362,
      77,    78,   914,    79,    80,    81,   396,   726,   313,   641,
     283,   919,   284,   348,   322,   323,   324,   325,   326,   327,
     328,   329,   330,   331,   332,  1225,   285,    77,    78,   314,
      79,    80,    81,  1273,  1240,   825,  1164,    77,    78,   315,
      79,    80,    81,   243,   244,   245,   341,   236,   468,   500,
     950,  1241,   316,   953,  1242,    32,  -586,   665,   958,   333,
     334,   526,   241,    90,  -363,   588,   966,   433,  1238,  1239,
     767,    47,   506,   529,   684,   702,   689,   532,   883,   346,
     305,   306,   307,   308,   309,   310,   248,   311,   350,   690,
     311,   691,   273,    95,  1234,  1235,   205,  1287,   356,   982,
     335,   127,   359,   709,    95,   711,  1001,   360,   366,   707,
    -467,  -466,   127,   389,   389,   365,  1300,   267,   269,   270,
     368,   335,   206,   387,  1016,   103,   434,   244,   245,   392,
     738,   968,    77,    78,  1024,    79,    80,    81,   393,   420,
     398,   411,    32,   440,   419,   444,   742,   447,   832,   -36,
     833,   894,   448,   983,   468,   423,   752,   449,   458,    95,
     102,   536,   538,   541,   104,   547,   105,   127,   106,   394,
     450,   766,   765,   558,   376,   562,   564,   389,  1214,   571,
     575,   598,   576,    32,   599,    34,   774,   604,   588,   607,
     588,   103,   588,   609,   618,   207,   588,    32,   588,   675,
     676,   184,   612,   623,   624,   774,   900,   629,   205,   630,
     632,  1149,  -364,   796,   796,   684,   816,   208,   677,    77,
      78,   642,    79,    80,    81,   643,   678,   679,    32,   645,
    1259,   652,   648,   653,   206,   655,   680,   209,    95,   817,
     210,    95,   656,   659,    32,   668,   127,   671,   669,   127,
     713,   821,   696,  1026,    32,   389,   672,   389,  1142,   715,
      77,    78,   719,    79,    80,    81,   767,  1033,   720,   841,
     729,   845,   722,   103,    77,    78,   731,    79,    80,    81,
     734,   681,   605,   854,   551,  1042,    90,   588,   732,   733,
     735,  1183,   209,   682,   741,    95,   102,   167,   167,   753,
     104,   179,   105,   127,   106,    77,    78,   207,    79,    80,
      81,   990,   743,   389,   896,   746,   749,   248,   754,   760,
     762,    77,    78,   683,    79,    80,    81,   103,   776,   208,
     927,    77,    78,   763,    79,    80,    81,   783,   778,   205,
     684,   786,   793,   806,   684,   249,   791,   802,   805,   209,
    1166,   808,   210,   818,    95,   930,   926,  1171,   820,  1203,
     822,   824,   127,    95,   836,   206,   835,   837,   707,   428,
     842,   127,   849,   389,   850,   856,   859,  1034,   866,   867,
     868,   869,   870,   871,   892,    32,   898,   916,   895,  1140,
     897,   907,   910,   605,   904,   103,    32,   588,    34,   908,
     913,   917,   211,   915,   918,   423,   921,   922,    32,   928,
      34,   929,  -244,   984,   588,   588,   588,   931,   389,   934,
     866,   867,   868,   869,   870,   871,   588,   938,   946,   205,
     944,   947,   949,  1151,   948,   585,   900,   588,   207,   951,
     167,   952,   588,   794,   795,    32,   167,   954,   585,   988,
     588,   956,   167,   955,   684,   206,   684,    32,   957,   960,
     208,   962,    77,    78,   961,    79,    80,    81,   967,   979,
    1000,  1002,  1266,    77,    78,    32,    79,    80,    81,  1015,
     209,  1008,  1011,   210,   389,    77,    78,  1021,    79,    80,
      81,  1017,  1020,  1219,  1022,  1025,   588,   764,   167,    90,
     605,  1023,  1038,  1027,  1028,  1041,   167,   167,   167,   588,
    1030,    90,    32,   167,  1046,  1126,   588,  1031,  1137,   167,
    1138,  1133,    77,    78,  1050,    79,    80,    81,   207,   211,
     219,  1253,  1148,  1144,    77,    78,  1257,    79,    80,    81,
     389,  1260,  1150,   389,  1202,  1155,  1152,  1156,  1262,  1263,
     208,  1160,    77,    78,  1162,    79,    80,    81,    32,   684,
    1163,   723,  1165,  1143,    95,  1172,  1173,    95,  1169,  1176,
     209,    95,   127,   210,  1174,   127,  1175,  1196,  1201,   127,
      95,  1204,  1205,  1278,  1206,   179,  1209,   280,   127,    77,
      78,  1211,    79,    80,    81,  1215,   103,   584,   429,  1223,
    1224,   585,   435,   103,  1236,  1244,  1245,  1255,  1199,  1249,
     584,  1254,   103,  -205,   585,   585,    32,  1264,  1258,  1265,
     167,   429,  1191,   435,   429,   435,   435,   167,  1267,   286,
     287,   288,  1268,  1270,  1271,    77,    78,  1316,    79,    80,
      81,  1272,  1275,  1296,  1291,   289,  1322,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,  1280,   311,
    1248,  1294,   583,  1299,    32,  1301,   613,   614,   236,  1306,
    1308,  1309,   585,  1318,  1320,   583,   168,   168,    47,  1321,
     180,  1131,   593,    77,    78,  1132,    79,    80,    81,    54,
      55,   684,  1303,   508,    95,    95,  1217,    61,   317,    95,
     510,   507,   127,   127,   983,   788,   320,   127,   785,  1226,
     911,   903,  1305,   933,  1126,  1126,  1032,   596,  1133,  1133,
    1184,   986,  1189,  1317,   192,   261,   103,   167,  1178,   912,
     236,   103,   920,   798,   318,   887,  1200,   597,   809,   425,
     834,    77,    78,   684,    79,    80,    81,    95,  1005,   417,
     855,  1154,     0,   584,     0,   127,   585,     0,     0,     0,
       0,   585,     0,     0,     0,     0,   584,   584,   585,     0,
       0,     0,     0,     0,     0,   167,     0,     0,     0,   103,
       0,    95,     0,     0,     0,    95,   862,     0,    95,   127,
      95,    95,  1283,   127,     0,     0,   127,     0,   127,   127,
       0,     0,     0,     0,     0,     0,    95,   167,     0,   167,
       0,     0,  1297,   103,   127,     0,     0,   103,     0,   168,
     103,     0,   103,   103,     0,   168,     0,   167,   583,     0,
       0,   168,     0,     0,   584,     0,     0,     0,   103,     0,
       0,   583,   583,     0,    95,     0,     0,     0,     0,     0,
      95,   585,   127,     0,     0,     0,     0,     0,   127,     0,
       0,     0,     0,     0,   167,     0,     0,     0,     0,     0,
       0,   179,     0,   167,   167,     0,   103,   168,     0,     0,
       0,     0,   103,     0,     0,   168,   168,   168,     0,     0,
     708,     0,   168,     0,     0,     0,     0,     0,   168,     0,
       0,     0,     0,   727,   728,     0,     0,     0,     0,   583,
      32,     0,   179,   286,   287,   288,     0,     0,   584,     0,
       0,     0,     0,   584,     0,     0,     0,     0,   179,   289,
     584,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,     0,   311,     0,   205,     0,     0,    10,     0,
       0,     0,     0,     0,   180,     0,   169,   169,     0,   167,
     181,   770,     0,     0,   585,     0,   585,     0,   585,     0,
     141,   206,   585,    74,   585,    76,     0,    77,    78,     0,
      79,    80,    81,   583,     0,     0,     0,     0,   583,   168,
       0,    32,     0,     0,     0,   583,   168,     0,     0,     0,
       0,     0,     0,   584,     0,     0,   864,     0,     0,     0,
       0,   865,     0,   866,   867,   868,   869,   870,   871,   872,
       0,     0,   167,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   179,     0,     0,     0,     0,     0,     0,     0,
       0,   587,     0,     0,   207,   838,     0,     0,     0,     0,
     846,     0,     0,     0,   587,   873,   874,   852,   875,     0,
       0,     0,     0,   585,   167,     0,   208,   619,    77,    78,
       0,    79,    80,    81,     0,     0,   167,   945,   583,     0,
     876,     0,     0,     0,     0,     0,   209,     0,     0,   210,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   169,
       0,     0,     0,   179,     0,   169,   168,     0,     0,     0,
     179,   169,     0,   167,     0,     0,     0,     0,     0,   179,
       0,     0,     0,     0,     0,     0,   584,   179,   584,     0,
     584,     0,     0,     0,   584,     0,   584,     0,     0,     0,
     935,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    27,     0,     0,   168,     0,     0,   169,     0,    32,
       0,    34,     0,     0,     0,   169,   169,   169,     0,     0,
       0,     0,   169,   585,     0,     0,     0,     0,   169,     0,
       0,     0,     0,     0,     0,     0,   168,     0,   168,     0,
     585,   585,   585,     0,     0,     0,     0,     0,  1227,   165,
       0,   583,   585,   583,     0,   583,   168,   587,     0,   583,
       0,   583,     0,   585,     0,     0,     0,     0,   585,     0,
     587,   587,     0,     0,     0,   584,   585,     0,     0,   141,
       0,     0,    74,     0,    76,     0,    77,    78,     0,    79,
      80,    81,     0,   168,   181,     0,     0,     0,     0,     0,
     748,     0,   168,   168,     0,     0,     0,   353,     0,     0,
       0,     0,    90,  1035,     0,  1036,     0,  1037,     0,     0,
       0,  1039,   585,  1040,     0,     0,     0,     0,     0,   169,
       0,     0,     0,     0,     0,   585,   169,     0,   587,     0,
       0,   777,   585,   286,   287,   288,     0,     0,     0,     0,
     583,   179,   705,     0,     0,   179,     0,   180,     0,   289,
       0,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,     0,   311,     0,   584,     0,     0,     0,     0,
       0,     0,    32,     0,    34,     0,     0,     0,   168,     0,
       0,     0,   584,   584,   584,     0,     0,     0,     0,     0,
       0,     0,  1179,     0,   584,     0,     0,     0,     0,     0,
       0,     0,   587,     0,     0,   584,     0,   587,     0,     0,
     584,     0,   165,     0,   587,     0,     0,     0,   584,     0,
       0,     0,     0,     0,     0,     0,   169,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     583,   168,   141,     0,     0,    74,    10,    76,     0,    77,
      78,   748,    79,    80,    81,     0,     0,   583,   583,   583,
       0,     0,     0,     0,   584,   179,     0,     0,     0,   583,
     166,     0,     0,     0,   169,    90,     0,   584,     0,     0,
     583,     0,     0,   168,   584,   583,     0,   638,     0,     0,
       0,     0,     0,   583,     0,   168,     0,   587,     0,     0,
       0,     0,  1210,     0,   864,     0,   169,     0,   169,   865,
       0,   866,   867,   868,   869,   870,   871,   872,     0,  1220,
    1221,  1222,   959,     0,     0,     0,   169,     0,     0,   964,
      32,  1233,   168,     0,     0,     0,     0,     0,   970,   583,
       0,     0,  1243,     0,     0,     0,   978,  1247,     0,     0,
       0,     0,   583,   873,   874,  1256,   875,     0,     0,   583,
       0,     0,     0,   169,     0,     0,     0,     0,     0,     0,
       0,   971,   169,   169,     0,     0,     0,     0,   965,     0,
     179,     0,     0,   972,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   179,     0,     0,     0,     0,     0,     0,
     141,  1276,     0,    74,     0,   973,     0,    77,    78,     0,
      79,   974,    81,     0,  1286,   286,   287,   288,     0,     0,
     587,  1292,   587,     0,   587,     0,     0,   181,   587,     0,
     587,   289,     0,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,     0,   311,   286,   287,   288,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   169,     0,
       0,     0,   289,     0,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,     0,   311,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,     0,   311,   587,
    1180,     0,     0,    10,   978,   144,   146,     0,   148,   149,
     150,   169,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,    10,     0,   173,   176,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   193,     0,
       0,     0,     0,     0,     0,   201,     0,   204,     0,     0,
     216,     0,   218,   169,     0,     0,     0,     0,     0,   670,
       0,   864,     0,     0,     0,   169,   865,     0,   866,   867,
     868,   869,   870,   871,   872,     0,   253,     0,     0,     0,
       0,     0,   864,     0,     0,     0,     0,   865,   260,   866,
     867,   868,   869,   870,   871,   872,     0,     0,     0,     0,
     673,     0,   169,    10,     0,     0,     0,     0,     0,   587,
     873,   874,     0,   875,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   587,   587,   587,     0,
       0,   873,   874,     0,   875,  1013,    10,     0,   587,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   349,   587,
       0,     0,     0,     0,   587,     0,  1158,     0,     0,     0,
       0,   864,   587,     0,     0,     0,   865,     0,   866,   867,
     868,   869,   870,   871,   872,     0,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   373,     0,
       0,   373,     0,     0,   864,     0,     0,   193,   382,   865,
       0,   866,   867,   868,   869,   870,   871,   872,   587,     0,
     873,   874,     0,   875,     0,     0,     0,     0,     0,     0,
       0,   587,   333,   334,     0,     0,     0,     0,   587,     0,
       0,     0,     0,     0,     0,  1159,     0,     0,     0,     0,
       0,   173,     0,   873,   874,   431,   875,     0,     0,   970,
    -677,  -677,  -677,  -677,   303,   304,   305,   306,   307,   308,
     309,   310,  1315,   311,     0,   457,     0,     0,  1213,     0,
       0,     0,     0,     0,     0,     0,   466,     0,     0,     0,
       0,     0,     0,     0,   335,     0,     0,   471,   472,   473,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,     0,     0,   498,   498,   501,     0,     0,     0,
       0,     0,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,     0,     0,     0,     0,     0,   498,
     530,     0,   466,   498,   533,     0,     0,     0,     0,   514,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   544,
       0,   546,     4,     5,     6,     7,     8,   466,     0,     0,
       0,     9,     0,     0,     0,     0,     0,   557,     0,     0,
       0,     0,     0,     0,     0,   374,     0,     0,     0,   348,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,    11,    12,     0,     0,     0,     0,    13,   595,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,    33,    34,     0,    35,   333,   334,     0,    36,    37,
      38,    39,     0,    40,     0,    41,     0,    42,     0,     0,
      43,     0,     0,     0,    44,    45,    46,    47,     0,    49,
      50,     0,    51,     0,    53,     0,     0,     0,     0,     0,
      56,     0,    57,    58,    59,   650,     0,     0,     0,     0,
       0,     0,    63,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,   335,     0,     0,
     141,    72,    73,    74,    75,    76,     0,    77,    78,     0,
      79,    80,    81,   253,     0,    83,     0,     0,     0,    84,
       0,     0,     0,     0,     0,    85,     0,   666,    87,     0,
      88,    89,     0,    90,    91,     0,    92,    93,     0,     0,
       0,     0,     4,     5,     6,     7,     8,     0,     0,     0,
       0,     9,   697,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   193,     0,     0,     0,     0,     0,     0,
     511,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,     0,     0,     0,    30,    31,
      32,    33,    34,     0,     0,     0,     0,     0,    36,   755,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     761,     0,     0,     0,     0,     0,     0,    47,     0,     0,
       0,     0,     0,     0,   769,     0,     0,     0,     0,     0,
     139,     0,   780,    58,    59,   781,     0,   782,     0,     0,
     466,     0,   140,    64,    65,    66,    67,    68,    69,   466,
       0,    32,     0,    34,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,   512,    76,     0,    77,    78,     0,
      79,    80,    81,   811,     0,    83,     0,     0,     0,    84,
       0,     0,   286,   287,   288,    85,     0,     0,    87,     0,
       0,   165,     0,    90,    91,     0,    92,    93,   289,     0,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   141,   311,     0,    74,     0,    76,     0,    77,    78,
       0,    79,    80,    81,   861,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   893,     0,     0,     0,   166,
       0,     0,   407,     0,    90,     0,     0,   286,   287,   288,
       0,     0,     0,     0,   466,     0,     0,     0,     0,     0,
       0,     0,   466,   289,   861,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,     0,   311,     0,     0,
       0,   193,   286,   287,   288,    30,    31,     0,     0,   939,
       0,     0,     0,     0,     0,    36,     0,     0,   289,     0,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,     0,   311,     0,     0,     0,   739,     0,     0,     0,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
       0,    65,    66,    67,    68,    69,     0,   996,     0,     0,
       0,   997,   580,   998,     0,     0,   466,     0,    72,    73,
       0,     0,     0,     0,     0,     0,  1010,     0,    10,    11,
      12,     0,    83,     0,   466,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,   790,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,    60,    61,    62,   812,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,   466,     0,    70,     0,     0,     0,     0,    71,    72,
      73,    74,    75,    76,     0,    77,    78,     0,    79,    80,
      81,    82,     0,    83,     0,     0,     0,    84,     0,     0,
       0,     0,     0,    85,    86,     0,    87,     0,    88,    89,
     756,    90,    91,   288,    92,    93,     4,     5,     6,     7,
       8,     0,     0,     0,     0,     9,     0,   289,     0,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
       0,   311,     0,     0,    10,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,    33,    34,     0,    35,     0,
       0,     0,    36,    37,    38,    39,     0,    40,     0,    41,
       0,    42,     0,     0,    43,     0,     0,     0,    44,    45,
      46,    47,    48,    49,    50,     0,    51,    52,    53,   466,
       0,     0,    54,    55,    56,     0,    57,    58,    59,    60,
      61,    62,     0,     0,     0,     0,    63,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,    71,    72,    73,    74,    75,    76,
       0,    77,    78,     0,    79,    80,    81,    82,     0,    83,
       0,     0,     0,    84,     4,     5,     6,     7,     8,    85,
      86,     0,    87,     9,    88,    89,   863,    90,    91,     0,
      92,    93,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
       0,   311,    10,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,    33,    34,     0,    35,     0,     0,     0,
      36,    37,    38,    39,     0,    40,     0,    41,     0,    42,
       0,     0,    43,     0,     0,     0,    44,    45,    46,    47,
      48,    49,    50,     0,    51,    52,    53,     0,     0,     0,
      54,    55,    56,     0,    57,    58,    59,    60,    61,    62,
       0,     0,     0,     0,    63,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,    71,    72,    73,    74,    75,    76,     0,    77,
      78,     0,    79,    80,    81,    82,     0,    83,     0,     0,
       0,    84,     4,     5,     6,     7,     8,    85,    86,     0,
      87,     9,    88,    89,     0,    90,    91,     0,    92,    93,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,     0,   311,     0,
      10,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,    33,    34,     0,    35,     0,     0,     0,    36,    37,
      38,    39,     0,    40,     0,    41,     0,    42,     0,     0,
      43,     0,     0,     0,    44,    45,    46,    47,     0,    49,
      50,     0,    51,     0,    53,     0,     0,     0,    54,    55,
      56,     0,    57,    58,    59,     0,    61,    62,     0,     0,
       0,     0,    63,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,    75,    76,     0,    77,    78,     0,
      79,    80,    81,    82,     0,    83,     0,     0,     0,    84,
       4,     5,     6,     7,     8,    85,     0,     0,    87,     9,
      88,    89,   451,    90,    91,     0,    92,    93,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,     0,   311,     0,     0,    10,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   141,    72,
      73,    74,    75,    76,     0,    77,    78,     0,    79,    80,
      81,    82,     0,    83,     0,     0,     0,    84,     4,     5,
       6,     7,     8,    85,     0,     0,    87,     9,    88,    89,
     591,    90,    91,     0,    92,    93,  -677,  -677,  -677,  -677,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,     0,   311,     0,     0,    10,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,    33,    34,     0,
      35,     0,     0,     0,    36,    37,    38,    39,   819,    40,
       0,    41,     0,    42,     0,     0,    43,     0,     0,     0,
      44,    45,    46,    47,     0,    49,    50,     0,    51,     0,
      53,     0,     0,     0,    54,    55,    56,     0,    57,    58,
      59,     0,    61,    62,     0,     0,     0,     0,    63,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,    70,     0,     0,     0,     0,   141,    72,    73,    74,
      75,    76,     0,    77,    78,     0,    79,    80,    81,    82,
       0,    83,     0,     0,     0,    84,     4,     5,     6,     7,
       8,    85,     0,     0,    87,     9,    88,    89,     0,    90,
      91,     0,    92,    93,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    10,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,    33,    34,     0,    35,     0,
       0,     0,    36,    37,    38,    39,     0,    40,     0,    41,
       0,    42,   936,     0,    43,     0,     0,     0,    44,    45,
      46,    47,     0,    49,    50,     0,    51,     0,    53,     0,
       0,     0,    54,    55,    56,     0,    57,    58,    59,     0,
      61,    62,     0,     0,     0,     0,    63,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   141,    72,    73,    74,    75,    76,
       0,    77,    78,     0,    79,    80,    81,    82,     0,    83,
       0,     0,     0,    84,     4,     5,     6,     7,     8,    85,
       0,     0,    87,     9,    88,    89,     0,    90,    91,     0,
      92,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    10,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,    33,    34,     0,    35,     0,     0,     0,
      36,    37,    38,    39,     0,    40,     0,    41,     0,    42,
       0,     0,    43,     0,     0,     0,    44,    45,    46,    47,
       0,    49,    50,     0,    51,     0,    53,     0,     0,     0,
      54,    55,    56,     0,    57,    58,    59,     0,    61,    62,
       0,     0,     0,     0,    63,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,   141,    72,    73,    74,    75,    76,     0,    77,
      78,     0,    79,    80,    81,    82,     0,    83,     0,     0,
       0,    84,     4,     5,     6,     7,     8,    85,     0,     0,
      87,     9,    88,    89,  1177,    90,    91,     0,    92,    93,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      10,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,    33,    34,     0,    35,     0,     0,     0,    36,    37,
      38,    39,     0,    40,     0,    41,  1261,    42,     0,     0,
      43,     0,     0,     0,    44,    45,    46,    47,     0,    49,
      50,     0,    51,     0,    53,     0,     0,     0,    54,    55,
      56,     0,    57,    58,    59,     0,    61,    62,     0,     0,
       0,     0,    63,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,    75,    76,     0,    77,    78,     0,
      79,    80,    81,    82,     0,    83,     0,     0,     0,    84,
       4,     5,     6,     7,     8,    85,     0,     0,    87,     9,
      88,    89,     0,    90,    91,     0,    92,    93,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   141,    72,
      73,    74,    75,    76,     0,    77,    78,     0,    79,    80,
      81,    82,     0,    83,     0,     0,     0,    84,     4,     5,
       6,     7,     8,    85,     0,     0,    87,     9,    88,    89,
    1274,    90,    91,     0,    92,    93,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    10,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,    33,    34,     0,
      35,     0,     0,     0,    36,    37,    38,    39,     0,    40,
       0,    41,     0,    42,     0,     0,    43,     0,     0,     0,
      44,    45,    46,    47,     0,    49,    50,     0,    51,     0,
      53,     0,     0,     0,    54,    55,    56,     0,    57,    58,
      59,     0,    61,    62,     0,     0,     0,     0,    63,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,    70,     0,     0,     0,     0,   141,    72,    73,    74,
      75,    76,     0,    77,    78,     0,    79,    80,    81,    82,
       0,    83,     0,     0,     0,    84,     4,     5,     6,     7,
       8,    85,     0,     0,    87,     9,    88,    89,  1277,    90,
      91,     0,    92,    93,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    10,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,    33,    34,     0,    35,     0,
       0,     0,    36,    37,    38,    39,     0,    40,  1279,    41,
       0,    42,     0,     0,    43,     0,     0,     0,    44,    45,
      46,    47,     0,    49,    50,     0,    51,     0,    53,     0,
       0,     0,    54,    55,    56,     0,    57,    58,    59,     0,
      61,    62,     0,     0,     0,     0,    63,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   141,    72,    73,    74,    75,    76,
       0,    77,    78,     0,    79,    80,    81,    82,     0,    83,
       0,     0,     0,    84,     4,     5,     6,     7,     8,    85,
       0,     0,    87,     9,    88,    89,     0,    90,    91,     0,
      92,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    10,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,    33,    34,     0,    35,     0,     0,     0,
      36,    37,    38,    39,     0,    40,     0,    41,     0,    42,
       0,     0,    43,     0,     0,     0,    44,    45,    46,    47,
       0,    49,    50,     0,    51,     0,    53,     0,     0,     0,
      54,    55,    56,     0,    57,    58,    59,     0,    61,    62,
       0,     0,     0,     0,    63,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,   141,    72,    73,    74,    75,    76,     0,    77,
      78,     0,    79,    80,    81,    82,     0,    83,     0,     0,
       0,    84,     4,     5,     6,     7,     8,    85,     0,     0,
      87,     9,    88,    89,  1281,    90,    91,     0,    92,    93,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      10,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,    33,    34,     0,    35,     0,     0,     0,    36,    37,
      38,    39,     0,    40,     0,    41,     0,    42,     0,     0,
      43,     0,     0,     0,    44,    45,    46,    47,     0,    49,
      50,     0,    51,     0,    53,     0,     0,     0,    54,    55,
      56,     0,    57,    58,    59,     0,    61,    62,     0,     0,
       0,     0,    63,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,    75,    76,     0,    77,    78,     0,
      79,    80,    81,    82,     0,    83,     0,     0,     0,    84,
       4,     5,     6,     7,     8,    85,     0,     0,    87,     9,
      88,    89,  1282,    90,    91,     0,    92,    93,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   141,    72,
      73,    74,    75,    76,     0,    77,    78,     0,    79,    80,
      81,    82,     0,    83,     0,     0,     0,    84,     4,     5,
       6,     7,     8,    85,     0,     0,    87,     9,    88,    89,
    1293,    90,    91,     0,    92,    93,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    10,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,    33,    34,     0,
      35,     0,     0,     0,    36,    37,    38,    39,     0,    40,
       0,    41,     0,    42,     0,     0,    43,     0,     0,     0,
      44,    45,    46,    47,     0,    49,    50,     0,    51,     0,
      53,     0,     0,     0,    54,    55,    56,     0,    57,    58,
      59,     0,    61,    62,     0,     0,     0,     0,    63,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,    70,     0,     0,     0,     0,   141,    72,    73,    74,
      75,    76,     0,    77,    78,     0,    79,    80,    81,    82,
       0,    83,     0,     0,     0,    84,     4,     5,     6,     7,
       8,    85,     0,     0,    87,     9,    88,    89,  1319,    90,
      91,     0,    92,    93,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    10,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,    33,    34,     0,    35,     0,
       0,     0,    36,    37,    38,    39,     0,    40,     0,    41,
       0,    42,     0,     0,    43,     0,     0,     0,    44,    45,
      46,    47,     0,    49,    50,     0,    51,     0,    53,     0,
       0,     0,    54,    55,    56,     0,    57,    58,    59,     0,
      61,    62,     0,     0,     0,     0,    63,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   141,    72,    73,    74,    75,    76,
       0,    77,    78,     0,    79,    80,    81,    82,     0,    83,
       0,     0,     0,    84,     4,     5,     6,     7,     8,    85,
       0,     0,    87,     9,    88,    89,  1323,    90,    91,     0,
      92,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    10,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,    33,    34,     0,    35,     0,     0,     0,
      36,    37,    38,    39,     0,    40,     0,    41,     0,    42,
       0,     0,    43,     0,     0,     0,    44,    45,    46,    47,
       0,    49,    50,     0,    51,     0,    53,     0,     0,     0,
      54,    55,    56,     0,    57,    58,    59,     0,    61,    62,
       0,     0,     0,     0,    63,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,   141,    72,    73,    74,    75,    76,     0,    77,
      78,     0,    79,    80,    81,    82,     0,    83,     0,     0,
       0,    84,     4,     5,     6,     7,     8,    85,     0,     0,
      87,     9,    88,    89,     0,    90,    91,     0,    92,    93,
       0,     0,     0,     0,     0,   559,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,    30,    31,
      32,    33,    34,     0,    35,     0,     0,     0,    36,    37,
      38,    39,     0,    40,     0,    41,     0,    42,     0,     0,
      43,     0,     0,     0,    44,    45,    46,    47,     0,    49,
      50,     0,    51,     0,    53,     0,     0,     0,     0,     0,
      56,     0,    57,    58,    59,     0,     0,     0,     0,     0,
       0,     0,    63,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,    75,    76,     0,    77,    78,     0,
      79,    80,    81,     0,     0,    83,     0,     0,     0,    84,
       4,     5,     6,     7,     8,    85,     0,     0,    87,     9,
      88,    89,     0,    90,    91,     0,    92,    93,     0,     0,
       0,     0,     0,   710,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,     0,     0,    56,     0,
      57,    58,    59,     0,     0,     0,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   141,    72,
      73,    74,    75,    76,     0,    77,    78,     0,    79,    80,
      81,     0,     0,    83,     0,     0,     0,    84,     4,     5,
       6,     7,     8,    85,     0,     0,    87,     9,    88,    89,
       0,    90,    91,     0,    92,    93,     0,     0,     0,     0,
       0,  1168,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,     0,     0,     0,    30,    31,    32,    33,    34,     0,
      35,     0,     0,     0,    36,    37,    38,    39,     0,    40,
       0,    41,     0,    42,     0,     0,    43,     0,     0,     0,
      44,    45,    46,    47,     0,    49,    50,     0,    51,     0,
      53,     0,     0,     0,     0,     0,    56,     0,    57,    58,
      59,     0,     0,     0,     0,     0,     0,     0,    63,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,    70,     0,     0,     0,     0,   141,    72,    73,    74,
      75,    76,     0,    77,    78,     0,    79,    80,    81,     0,
       0,    83,     0,     0,     0,    84,     4,     5,     6,     7,
       8,    85,     0,     0,    87,     9,    88,    89,     0,    90,
      91,     0,    92,    93,     0,     0,     0,     0,     0,  1216,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,     0,
       0,     0,    30,    31,    32,    33,    34,     0,    35,     0,
       0,     0,    36,    37,    38,    39,     0,    40,     0,    41,
       0,    42,     0,     0,    43,     0,     0,     0,    44,    45,
      46,    47,     0,    49,    50,     0,    51,     0,    53,     0,
       0,     0,     0,     0,    56,     0,    57,    58,    59,     0,
       0,     0,     0,     0,     0,     0,    63,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   141,    72,    73,    74,    75,    76,
       0,    77,    78,     0,    79,    80,    81,     0,     0,    83,
       0,     0,     0,    84,     4,     5,     6,     7,     8,    85,
       0,     0,    87,     9,    88,    89,     0,    90,    91,     0,
      92,    93,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,     0,   311,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,     0,     0,     0,
      30,    31,    32,    33,    34,     0,    35,     0,     0,     0,
      36,    37,    38,    39,     0,    40,     0,    41,     0,    42,
       0,     0,    43,     0,     0,     0,    44,    45,    46,    47,
       0,    49,    50,     0,    51,     0,    53,     0,     0,     0,
       0,     0,    56,     0,    57,    58,    59,     0,     0,     0,
       0,     0,     0,     0,    63,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,   141,    72,    73,    74,    75,    76,     0,    77,
      78,     0,    79,    80,    81,     0,     0,    83,     0,     0,
       0,    84,     4,     5,     6,     7,     8,    85,     0,     0,
      87,     9,    88,    89,     0,    90,    91,     0,    92,    93,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     172,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,     0,     0,     0,    30,    31,
      32,    33,    34,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     139,     0,     0,    58,    59,     0,     0,     0,     0,     0,
       0,     0,   140,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,     0,    76,     0,    77,    78,     0,
      79,    80,    81,     0,     0,    83,     0,     0,     0,    84,
       4,     5,     6,     7,     8,    85,     0,     0,    87,     9,
       0,     0,     0,    90,    91,     0,    92,    93,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,     0,     0,     0,    30,    31,    32,    33,
      34,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   139,     0,
       0,    58,    59,     0,     0,     0,     0,     0,     0,     0,
     140,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   141,    72,
      73,    74,     0,    76,     0,    77,    78,     0,    79,    80,
      81,     0,     0,    83,     0,     0,     0,    84,     4,     5,
       6,     7,     8,    85,     0,     0,    87,     9,   200,     0,
       0,    90,    91,     0,    92,    93,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    30,    31,    32,    33,    34,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   139,     0,     0,    58,
      59,     0,     0,     0,     0,     0,     0,     0,   140,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,    70,     0,     0,     0,     0,   141,    72,    73,    74,
       0,    76,     0,    77,    78,     0,    79,    80,    81,     0,
       0,    83,     0,     0,     0,    84,     4,     5,     6,     7,
       8,    85,     0,     0,    87,     9,   203,     0,     0,    90,
      91,     0,    92,    93,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,     0,
       0,     0,    30,    31,    32,    33,    34,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   139,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,   140,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   141,    72,    73,    74,     0,    76,
       0,    77,    78,     0,    79,    80,    81,     0,     0,    83,
       0,     0,     0,    84,     4,     5,     6,     7,     8,    85,
       0,     0,    87,     9,   215,     0,     0,    90,    91,     0,
      92,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,     0,     0,     0,
      30,    31,    32,    33,    34,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   252,     0,     0,    47,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   139,     0,     0,    58,    59,     0,     0,     0,
       0,     0,     0,     0,   140,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,   141,    72,    73,    74,     0,    76,     0,    77,
      78,     0,    79,    80,    81,     0,     0,    83,     0,     0,
       0,    84,     4,     5,     6,     7,     8,    85,     0,     0,
      87,     9,     0,     0,     0,    90,    91,     0,    92,    93,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,     0,     0,     0,    30,    31,
      32,    33,    34,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   664,     0,
     139,     0,     0,    58,    59,     0,    32,     0,    34,     0,
       0,     0,   140,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,     0,    76,     0,    77,    78,     0,
      79,    80,    81,     0,     0,    83,   165,     0,     0,    84,
       0,     0,     0,     0,     0,    85,     0,     0,    87,   372,
       0,     0,     0,    90,    91,     0,    92,    93,     4,     5,
       6,     7,     8,     0,     0,     0,   141,     9,     0,    74,
       0,    76,     0,    77,    78,     0,    79,    80,    81,     0,
       0,     0,     0,     0,     0,     0,   463,     0,     0,     0,
       0,     0,     0,     0,   166,     0,     0,    11,    12,    90,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    30,    31,    32,    33,    34,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   139,     0,     0,    58,
      59,     0,     0,     0,     0,     0,     0,     0,   140,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,    70,     0,     0,     0,     0,   141,    72,    73,    74,
       0,    76,     0,    77,    78,     0,    79,    80,    81,     0,
       0,    83,     0,     0,     0,    84,     4,     5,     6,     7,
       8,    85,     0,     0,    87,     9,     0,     0,     0,    90,
      91,     0,    92,    93,     0,     0,     0,     0,     0,   474,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,     0,
       0,     0,    30,    31,    32,    33,    34,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   139,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,   140,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   141,    72,    73,    74,     0,    76,
       0,    77,    78,     0,    79,    80,    81,     0,     0,    83,
       0,     0,     0,    84,     4,     5,     6,     7,     8,    85,
       0,     0,    87,     9,     0,     0,     0,    90,    91,     0,
      92,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   511,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,     0,     0,     0,
      30,    31,    32,    33,    34,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    47,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   139,     0,     0,    58,    59,     0,     0,     0,
       0,     0,     0,     0,   140,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,   141,    72,    73,    74,     0,    76,     0,    77,
      78,     0,    79,    80,    81,     0,     0,    83,     0,     0,
       0,    84,     4,     5,     6,     7,     8,    85,     0,     0,
      87,     9,     0,     0,     0,    90,    91,     0,    92,    93,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     543,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,     0,     0,     0,    30,    31,
      32,    33,    34,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     139,     0,     0,    58,    59,     0,     0,     0,     0,     0,
       0,     0,   140,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,     0,    76,     0,    77,    78,     0,
      79,    80,    81,     0,     0,    83,     0,     0,     0,    84,
       4,     5,     6,     7,     8,    85,     0,     0,    87,     9,
       0,     0,     0,    90,    91,     0,    92,    93,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   545,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,     0,     0,     0,    30,    31,    32,    33,
      34,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   139,     0,
       0,    58,    59,     0,     0,     0,     0,     0,     0,     0,
     140,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   141,    72,
      73,    74,     0,    76,     0,    77,    78,     0,    79,    80,
      81,     0,     0,    83,     0,     0,     0,    84,     4,     5,
       6,     7,     8,    85,     0,     0,    87,     9,     0,     0,
       0,    90,    91,     0,    92,    93,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   768,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       0,     0,     0,     0,    30,    31,    32,    33,    34,     0,
       0,     0,     0,     0,    36,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    47,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   139,     0,     0,    58,
      59,     0,     0,     0,     0,     0,     0,     0,   140,    64,
      65,    66,    67,    68,    69,     0,     0,     0,     0,     0,
       0,    70,     0,     0,     0,     0,   141,    72,    73,    74,
       0,    76,     0,    77,    78,     0,    79,    80,    81,     0,
       0,    83,     0,     0,     0,    84,     4,     5,     6,     7,
       8,    85,     0,     0,    87,     9,     0,     0,     0,    90,
      91,     0,    92,    93,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   810,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,     0,     0,
       0,     0,    30,    31,    32,    33,    34,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   139,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,   140,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   141,    72,    73,    74,     0,    76,
       0,    77,    78,     0,    79,    80,    81,     0,     0,    83,
       0,     0,     0,    84,     4,     5,     6,     7,     8,    85,
       0,     0,    87,     9,     0,     0,     0,    90,    91,     0,
      92,    93,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,     0,     0,     0,     0,
      30,    31,    32,    33,    34,     0,     0,     0,     0,     0,
      36,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    47,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   139,     0,     0,    58,    59,     0,     0,     0,
       0,     0,     0,     0,   140,    64,    65,    66,    67,    68,
      69,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,   141,    72,    73,    74,   512,    76,     0,    77,
      78,     0,    79,    80,    81,     0,     0,    83,     0,     0,
       0,    84,     4,     5,     6,     7,     8,    85,     0,     0,
      87,     9,     0,     0,     0,    90,    91,     0,    92,    93,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,     0,     0,     0,     0,    30,    31,
      32,    33,    34,     0,     0,     0,     0,     0,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    47,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     139,     0,     0,    58,    59,     0,     0,     0,     0,     0,
       0,     0,   140,    64,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
     141,    72,    73,    74,     0,    76,     0,    77,    78,     0,
      79,    80,    81,     0,     0,    83,     0,     0,     0,    84,
       4,     5,     6,     7,     8,    85,     0,     0,    87,     9,
       0,     0,     0,    90,    91,     0,    92,    93,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,     0,     0,     0,    30,    31,    32,   430,
      34,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   139,     0,
       0,    58,    59,     0,     0,     0,    32,     0,    34,     0,
     140,    64,    65,    66,    67,    68,    69,     0,     0,    32,
       0,    34,     0,    70,     0,     0,     0,     0,   141,    72,
      73,    74,     0,    76,     0,    77,    78,     0,    79,    80,
      81,     0,     0,    83,     0,     0,   165,    84,     0,     0,
       0,     0,     0,    85,     0,     0,    87,     0,   422,   165,
       0,    90,    91,     0,    92,    93,  1053,  1054,  1055,  1056,
    1057,   751,  1058,  1059,  1060,  1061,   141,     0,     0,    74,
       0,    76,     0,    77,    78,     0,    79,    80,    81,   141,
       0,     0,    74,     0,    76,     0,    77,    78,     0,    79,
      80,    81,     0,     0,   166,     0,     0,     0,     0,    90,
       0,     0,  1062,     0,     0,     0,     0,   166,     0,     0,
       0,     0,    90,     0,     0,  1063,  1064,  1065,  1066,  1067,
    1068,  1069,     0,     0,    32,     0,     0,     0,     0,     0,
       0,     0,     0,  1070,  1071,  1072,  1073,  1074,  1075,  1076,
    1077,  1078,  1079,  1080,  1081,  1082,  1083,  1084,  1085,  1086,
    1087,  1088,  1089,  1090,  1091,  1092,  1093,  1094,  1095,  1096,
    1097,  1098,  1099,  1100,  1101,  1102,  1103,  1104,  1105,  1106,
    1107,  1108,  1109,  1110,     0,     0,  1111,  1112,  1113,  1114,
    1115,  1116,  1117,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1118,  1119,  1120,     0,  1121,     0,
       0,    77,    78,     0,    79,    80,    81,  1122,     0,  1123,
       0,     0,  1124,   286,   287,   288,     0,     0,     0,     0,
    1125,     0,     0,     0,     0,     0,     0,     0,     0,   289,
       0,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,     0,   311,   286,   287,   288,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     289,     0,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,     0,   311,   286,   287,   288,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   289,     0,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,     0,   311,   286,   287,   288,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   289,   940,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,     0,   311,   999,     0,   286,
     287,   288,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    32,     0,    34,     0,   289,     0,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,  1146,   311,
     286,   287,   288,     0,     0,     0,     0,     0,     0,     0,
       0,   165,     0,     0,     0,     0,   289,     0,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,  1147,
     311,   141,     0,     0,    74,     0,    76,     0,    77,    78,
       0,    79,    80,    81,     0,     0,     0,     0,     0,     0,
       0,   286,   287,   288,     0,     0,     0,     0,     0,   166,
       0,     0,     0,     0,    90,     0,     0,   289,   941,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
       0,   311,     0,     0,   286,   287,   288,     0,     0,     0,
       0,     0,     0,     0,    32,     0,    34,     0,     0,     0,
     289,   312,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,     0,   311,   286,   287,   288,     0,     0,
       0,     0,     0,     0,   177,     0,     0,     0,     0,     0,
       0,   289,   386,   290,   291,   292,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   141,   311,     0,    74,     0,    76,
       0,    77,    78,     0,    79,    80,    81,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   286,   287,   288,     0,
       0,     0,   178,     0,     0,     0,     0,    90,     0,     0,
       0,     0,   289,   388,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,     0,   311,     0,     0,   286,
     287,   288,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   289,   400,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,     0,   311,
     577,   578,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   402,     0,     0,
     286,   287,   288,     0,     0,     0,     0,    30,    31,    32,
       0,     0,     0,     0,     0,     0,   289,    36,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,     0,
     311,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   535,     0,     0,
       0,     0,   579,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,     0,   580,     0,     0,     0,     0,   141,
      72,    73,    74,     0,   581,     0,    77,    78,     0,    79,
      80,    81,     0,     0,    83,     0,   286,   287,   288,     0,
     554,     0,     0,     0,   582,     0,     0,     0,     0,     0,
       0,     0,   289,   358,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,     0,   311,   286,   287,   288,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   289,     0,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,     0,   311,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   286,   287,   288,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   289,   654,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,     0,   311,   286,   287,   288,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   289,
     692,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   289,   311,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,     0,   311,     0,     0,   286,
     287,   288,     0,     0,     0,   905,     0,     0,     0,     0,
       0,     0,     0,     0,   563,   289,   651,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,     0,   311,
     286,   287,   288,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   289,     0,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,     0,
     311,   287,   288,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   289,     0,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,     0,
     311,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,     0,   311
};

static const yytype_int16 yycheck[] =
{
       2,     2,   217,   276,   143,     2,    29,     2,     2,     2,
      82,   536,   341,   346,   773,   321,    39,   128,   880,   883,
      43,   311,   352,   646,    27,   179,   883,   273,   740,    23,
      24,    87,     2,   135,   562,   107,   185,   366,   185,   437,
     749,   744,     8,    45,     8,    13,    48,     8,     8,    61,
       8,    61,     8,     8,    52,     8,     8,   211,     8,     8,
      62,     8,     8,     8,   776,     8,     8,    46,    46,     8,
       8,   121,     8,    71,   961,   186,    74,   786,    78,    29,
      82,     8,     8,     8,    86,     8,    56,     8,    61,   102,
       8,   230,    26,    87,   121,    26,   205,   206,    31,   398,
      90,   210,    78,   169,    90,   107,    93,    94,   568,     0,
     166,   144,   411,   172,   121,   445,    73,   740,   169,   147,
     170,  1008,   178,   106,   107,   108,   109,   110,   111,    31,
     140,    61,   994,    31,    31,   758,   117,   137,   171,   172,
      73,    98,   117,   170,   172,    61,   159,    61,   121,    61,
     152,    41,   855,   776,   144,   125,    41,   169,   144,   374,
     172,   137,   172,   162,   273,    90,   173,   146,   146,   139,
     169,    73,   166,   503,   146,    73,    73,    61,   172,    61,
     319,    73,   161,    61,   178,    61,   173,    61,   171,   191,
     191,   121,   194,   194,   162,   904,   198,   191,   910,   908,
     194,    61,    73,   205,   206,   171,   172,   279,   210,   173,
     171,   171,   214,   171,   316,   171,   171,    61,   171,   171,
     170,   219,   171,   170,   622,   171,   171,   163,   171,   171,
     224,   170,   170,   335,   857,   389,   163,   171,   232,   233,
     234,   172,   172,   573,   170,   239,   318,   170,   320,   170,
     248,   245,   170,   713,   356,   715,   172,   169,   172,   962,
     172,   263,    73,   365,    61,   564,   368,   157,    61,   263,
     272,   273,   157,  1160,   276,   898,   609,   279,   577,   578,
     170,   806,   174,   392,    61,   170,   909,   910,   172,   140,
     172,    61,   820,   263,   172,    73,   172,   353,   172,   448,
      78,   448,    73,   174,    73,   140,  1015,    78,   311,    78,
      13,   162,   172,   316,   316,   317,   318,   169,   320,    66,
      67,   172,   121,    26,   121,    93,    94,   162,   172,   575,
     659,    73,   335,   335,   169,   169,    78,   172,   447,   668,
      43,    45,    46,    47,   559,    49,   645,    66,    67,    73,
     504,    73,   346,   356,   356,   169,    78,   687,    26,   353,
     138,   139,   365,   174,   172,   368,   368,   138,   139,   138,
     139,   170,   144,   527,   664,   172,    31,   379,   379,   172,
      92,    93,    94,    71,    73,   379,  1009,   169,   542,  1012,
     392,   916,   541,   918,   541,   172,   138,   139,   547,   401,
     547,  1265,   172,   169,   406,   173,   404,   401,  1265,   178,
      71,   169,  1299,   137,   138,   139,   138,   139,   420,   169,
      71,  1308,    73,  1285,   426,   427,    25,   147,   169,   583,
     729,   401,    92,    93,    94,   734,   547,   509,   169,    73,
     160,   169,   741,    42,    78,   447,    45,   753,   169,   138,
     139,   169,   172,   141,   783,   177,   144,   171,   172,    61,
     148,   149,   791,   151,   152,   153,   575,   576,    61,   463,
     171,   801,   171,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   173,   171,   148,   149,    61,
     151,   152,   153,  1252,    25,   710,  1021,   148,   149,   172,
     151,   152,   153,   137,   138,   139,   169,   509,   169,   314,
     840,    42,   140,   843,    45,    71,   169,   511,   848,    59,
      60,   172,    73,   174,    61,   824,   859,    78,  1190,  1191,
     639,    98,   634,   338,   536,   558,   538,   342,  1161,   169,
      42,    43,    44,    45,    46,    47,   144,    49,    40,   543,
      49,   545,   162,   555,  1186,  1187,    25,  1269,   140,   115,
     121,   555,   176,   565,   566,   566,   895,     8,   169,   563,
     140,   140,   566,   575,   576,   140,  1288,    91,    92,    93,
     121,   121,    51,   171,   913,   555,   137,   138,   139,   162,
     592,   864,   148,   149,   924,   151,   152,   153,   169,   162,
      13,    13,    71,   171,   170,    79,   600,    13,   717,   169,
     719,   765,   170,   169,   169,   609,   610,   170,   175,   621,
     621,   169,     8,   170,   621,   170,   621,   621,   621,    98,
     170,   634,   634,    82,     8,   171,    13,   639,  1163,    79,
     169,   169,     8,    71,    73,    73,  1269,   171,   947,   119,
     949,   621,   951,   169,    61,   124,   955,    71,   957,    42,
      43,   664,   170,   172,   161,  1288,   775,   122,    25,     8,
     170,  1000,    61,   675,   676,   677,   699,   146,    61,   148,
     149,   170,   151,   152,   153,     8,    69,    70,    71,    13,
    1215,   176,   119,   176,    51,   173,    79,   166,   700,   700,
     169,   703,     8,   169,    71,   169,   700,   170,   176,   703,
     171,   705,   170,   928,    71,   717,   176,   719,   991,   171,
     148,   149,   122,   151,   152,   153,   835,   942,     8,   731,
     169,   733,   170,   703,   148,   149,   140,   151,   152,   153,
     172,   124,   744,   745,   172,   960,   174,  1046,   169,   140,
     102,  1050,   166,   136,    13,   757,   757,    23,    24,    13,
     757,    27,   757,   757,   757,   148,   149,   124,   151,   152,
     153,   880,   171,   775,   768,   163,   172,   144,   176,    13,
      79,   148,   149,   166,   151,   152,   153,   757,   169,   146,
     813,   148,   149,   171,   151,   152,   153,   169,   120,    25,
     802,   172,    13,     8,   806,   172,   169,   169,   169,   166,
    1025,   170,   169,   171,   816,   816,   810,  1032,   171,  1148,
     122,    13,   816,   825,   170,    51,     8,   170,   822,   137,
      79,   825,   169,   835,   159,     8,   169,   946,   106,   107,
     108,   109,   110,   111,   122,    71,     8,   122,   169,   988,
     170,   170,   169,   855,   172,   825,    71,  1156,    73,   172,
     169,   176,   864,   170,     8,   859,   137,    79,    71,    26,
      73,    68,    98,   875,  1173,  1174,  1175,   171,   880,   170,
     106,   107,   108,   109,   110,   111,  1185,   171,    26,    25,
     163,   122,     8,  1002,   170,   398,  1005,  1196,   124,   122,
     166,     8,  1201,    69,    70,    71,   172,   170,   411,   879,
    1209,   173,   178,   122,   916,    51,   918,    71,     8,   172,
     146,     8,   148,   149,   170,   151,   152,   153,   173,   150,
     169,    26,  1231,   148,   149,    71,   151,   152,   153,   172,
     166,   170,   170,   169,   946,   148,   149,   122,   151,   152,
     153,   170,   170,  1168,   170,    26,  1255,   172,   224,   174,
     962,     8,    79,   171,   170,    73,   232,   233,   234,  1268,
     171,   174,    71,   239,    13,   977,  1275,   171,   171,   245,
      73,   983,   148,   149,   172,   151,   152,   153,   124,   991,
     144,  1206,   169,   146,   148,   149,  1211,   151,   152,   153,
    1002,  1216,   170,  1005,  1143,    73,   104,    13,  1223,  1224,
     146,   170,   148,   149,   170,   151,   152,   153,    71,  1021,
     122,   157,    79,   993,  1026,   170,   122,  1029,  1029,   170,
     166,  1033,  1026,   169,   122,  1029,   122,    13,    13,  1033,
    1042,   170,   169,  1258,   172,   311,    13,   146,  1042,   148,
     149,   172,   151,   152,   153,   122,  1026,   398,   242,   172,
     172,   564,   246,  1033,    51,    73,   169,    13,  1140,   170,
     411,    73,  1042,    90,   577,   578,    71,   140,   172,    90,
     346,   265,    29,   267,   268,   269,   270,   353,   153,     9,
      10,    11,    13,   169,    73,   148,   149,  1312,   151,   152,
     153,     8,    13,   155,   170,    25,  1321,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   171,    49,
    1202,   171,   398,   170,    71,    73,    73,    74,  1140,   171,
     170,   169,   645,    73,   170,   411,    23,    24,    98,   172,
      27,   146,   404,   148,   149,   150,   151,   152,   153,   109,
     110,  1163,  1295,   318,  1166,  1167,  1167,   117,   118,  1171,
     320,   317,  1166,  1167,   169,   663,   125,  1171,   660,  1181,
     788,   778,  1296,   822,  1186,  1187,   939,   409,  1190,  1191,
    1051,   875,  1129,  1314,    37,    87,  1166,   463,  1043,   789,
    1202,  1171,   802,   676,   154,   759,  1141,   411,   689,   240,
     720,   148,   149,  1215,   151,   152,   153,  1219,   898,   233,
     745,  1005,    -1,   564,    -1,  1219,   729,    -1,    -1,    -1,
      -1,   734,    -1,    -1,    -1,    -1,   577,   578,   741,    -1,
      -1,    -1,    -1,    -1,    -1,   511,    -1,    -1,    -1,  1219,
      -1,  1253,    -1,    -1,    -1,  1257,   176,    -1,  1260,  1253,
    1262,  1263,  1264,  1257,    -1,    -1,  1260,    -1,  1262,  1263,
      -1,    -1,    -1,    -1,    -1,    -1,  1278,   543,    -1,   545,
      -1,    -1,  1284,  1253,  1278,    -1,    -1,  1257,    -1,   166,
    1260,    -1,  1262,  1263,    -1,   172,    -1,   563,   564,    -1,
      -1,   178,    -1,    -1,   645,    -1,    -1,    -1,  1278,    -1,
      -1,   577,   578,    -1,  1316,    -1,    -1,    -1,    -1,    -1,
    1322,   824,  1316,    -1,    -1,    -1,    -1,    -1,  1322,    -1,
      -1,    -1,    -1,    -1,   600,    -1,    -1,    -1,    -1,    -1,
      -1,   607,    -1,   609,   610,    -1,  1316,   224,    -1,    -1,
      -1,    -1,  1322,    -1,    -1,   232,   233,   234,    -1,    -1,
     564,    -1,   239,    -1,    -1,    -1,    -1,    -1,   245,    -1,
      -1,    -1,    -1,   577,   578,    -1,    -1,    -1,    -1,   645,
      71,    -1,   648,     9,    10,    11,    -1,    -1,   729,    -1,
      -1,    -1,    -1,   734,    -1,    -1,    -1,    -1,   664,    25,
     741,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    25,    -1,    -1,    41,    -1,
      -1,    -1,    -1,    -1,   311,    -1,    23,    24,    -1,   705,
      27,   645,    -1,    -1,   947,    -1,   949,    -1,   951,    -1,
     141,    51,   955,   144,   957,   146,    -1,   148,   149,    -1,
     151,   152,   153,   729,    -1,    -1,    -1,    -1,   734,   346,
      -1,    71,    -1,    -1,    -1,   741,   353,    -1,    -1,    -1,
      -1,    -1,    -1,   824,    -1,    -1,    99,    -1,    -1,    -1,
      -1,   104,    -1,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,   768,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   778,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   398,    -1,    -1,   124,   729,    -1,    -1,    -1,    -1,
     734,    -1,    -1,    -1,   411,   148,   149,   741,   151,    -1,
      -1,    -1,    -1,  1046,   810,    -1,   146,   173,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   822,   157,   824,    -1,
     173,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,    -1,   849,    -1,   172,   463,    -1,    -1,    -1,
     856,   178,    -1,   859,    -1,    -1,    -1,    -1,    -1,   865,
      -1,    -1,    -1,    -1,    -1,    -1,   947,   873,   949,    -1,
     951,    -1,    -1,    -1,   955,    -1,   957,    -1,    -1,    -1,
     824,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    63,    -1,    -1,   511,    -1,    -1,   224,    -1,    71,
      -1,    73,    -1,    -1,    -1,   232,   233,   234,    -1,    -1,
      -1,    -1,   239,  1156,    -1,    -1,    -1,    -1,   245,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   543,    -1,   545,    -1,
    1173,  1174,  1175,    -1,    -1,    -1,    -1,    -1,  1181,   111,
      -1,   947,  1185,   949,    -1,   951,   563,   564,    -1,   955,
      -1,   957,    -1,  1196,    -1,    -1,    -1,    -1,  1201,    -1,
     577,   578,    -1,    -1,    -1,  1046,  1209,    -1,    -1,   141,
      -1,    -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,   600,   311,    -1,    -1,    -1,    -1,    -1,
     607,    -1,   609,   610,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,   947,    -1,   949,    -1,   951,    -1,    -1,
      -1,   955,  1255,   957,    -1,    -1,    -1,    -1,    -1,   346,
      -1,    -1,    -1,    -1,    -1,  1268,   353,    -1,   645,    -1,
      -1,   648,  1275,     9,    10,    11,    -1,    -1,    -1,    -1,
    1046,  1047,    31,    -1,    -1,  1051,    -1,   664,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,  1156,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    73,    -1,    -1,    -1,   705,    -1,
      -1,    -1,  1173,  1174,  1175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1046,    -1,  1185,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   729,    -1,    -1,  1196,    -1,   734,    -1,    -1,
    1201,    -1,   111,    -1,   741,    -1,    -1,    -1,  1209,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   463,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1156,   768,   141,    -1,    -1,   144,    41,   146,    -1,   148,
     149,   778,   151,   152,   153,    -1,    -1,  1173,  1174,  1175,
      -1,    -1,    -1,    -1,  1255,  1181,    -1,    -1,    -1,  1185,
     169,    -1,    -1,    -1,   511,   174,    -1,  1268,    -1,    -1,
    1196,    -1,    -1,   810,  1275,  1201,    -1,   173,    -1,    -1,
      -1,    -1,    -1,  1209,    -1,   822,    -1,   824,    -1,    -1,
      -1,    -1,  1156,    -1,    99,    -1,   543,    -1,   545,   104,
      -1,   106,   107,   108,   109,   110,   111,   112,    -1,  1173,
    1174,  1175,   849,    -1,    -1,    -1,   563,    -1,    -1,   856,
      71,  1185,   859,    -1,    -1,    -1,    -1,    -1,   865,  1255,
      -1,    -1,  1196,    -1,    -1,    -1,   873,  1201,    -1,    -1,
      -1,    -1,  1268,   148,   149,  1209,   151,    -1,    -1,  1275,
      -1,    -1,    -1,   600,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   112,   609,   610,    -1,    -1,    -1,    -1,   173,    -1,
    1296,    -1,    -1,   124,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1309,    -1,    -1,    -1,    -1,    -1,    -1,
     141,  1255,    -1,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,  1268,     9,    10,    11,    -1,    -1,
     947,  1275,   949,    -1,   951,    -1,    -1,   664,   955,    -1,
     957,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   705,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,  1046,
    1047,    -1,    -1,    41,  1051,     4,     5,    -1,     7,     8,
       9,   768,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    41,    -1,    25,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    44,    -1,    46,    -1,    -1,
      49,    -1,    51,   810,    -1,    -1,    -1,    -1,    -1,   173,
      -1,    99,    -1,    -1,    -1,   822,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,    -1,    75,    -1,    -1,    -1,
      -1,    -1,    99,    -1,    -1,    -1,    -1,   104,    87,   106,
     107,   108,   109,   110,   111,   112,    -1,    -1,    -1,    -1,
     173,    -1,   859,    41,    -1,    -1,    -1,    -1,    -1,  1156,
     148,   149,    -1,   151,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1173,  1174,  1175,    -1,
      -1,   148,   149,    -1,   151,   173,    41,    -1,  1185,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   147,  1196,
      -1,    -1,    -1,    -1,  1201,    -1,   173,    -1,    -1,    -1,
      -1,    99,  1209,    -1,    -1,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,    -1,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,   187,    -1,
      -1,   190,    -1,    -1,    99,    -1,    -1,   196,   197,   104,
      -1,   106,   107,   108,   109,   110,   111,   112,  1255,    -1,
     148,   149,    -1,   151,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1268,    59,    60,    -1,    -1,    -1,    -1,  1275,    -1,
      -1,    -1,    -1,    -1,    -1,   173,    -1,    -1,    -1,    -1,
      -1,   240,    -1,   148,   149,   244,   151,    -1,    -1,  1296,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,  1309,    49,    -1,   264,    -1,    -1,   173,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   275,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   121,    -1,    -1,   286,   287,   288,
     289,   290,   291,   292,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,    -1,    -1,   313,   314,   315,    -1,    -1,    -1,
      -1,    -1,   321,   322,   323,   324,   325,   326,   327,   328,
     329,   330,   331,   332,    -1,    -1,    -1,    -1,    -1,   338,
     339,    -1,   341,   342,   343,    -1,    -1,    -1,    -1,   348,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   358,
      -1,   360,     3,     4,     5,     6,     7,   366,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,   376,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    42,    43,    -1,    -1,    -1,    -1,    48,   407,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    59,    60,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,
     111,    -1,   113,   114,   115,   474,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,   121,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   512,    -1,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,   526,   169,    -1,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,   551,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   562,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,   618,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     629,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,   643,    -1,    -1,    -1,    -1,    -1,
     111,    -1,   651,   114,   115,   654,    -1,   656,    -1,    -1,
     659,    -1,   123,   124,   125,   126,   127,   128,   129,   668,
      -1,    71,    -1,    73,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   692,    -1,   156,    -1,    -1,    -1,   160,
      -1,    -1,     9,    10,    11,   166,    -1,    -1,   169,    -1,
      -1,   111,    -1,   174,   175,    -1,   177,   178,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,   141,    49,    -1,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,   753,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   764,    -1,    -1,    -1,   169,
      -1,    -1,   172,    -1,   174,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,   783,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   791,    25,   793,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,   820,     9,    10,    11,    69,    70,    -1,    -1,   828,
      -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,   173,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,   125,   126,   127,   128,   129,    -1,   886,    -1,    -1,
      -1,   890,   136,   892,    -1,    -1,   895,    -1,   142,   143,
      -1,    -1,    -1,    -1,    -1,    -1,   905,    -1,    41,    42,
      43,    -1,   156,    -1,   913,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,   173,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,   116,   117,   118,   173,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,  1000,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,   167,    -1,   169,    -1,   171,   172,
     173,   174,   175,    11,   177,   178,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,  1148,
      -1,    -1,   109,   110,   111,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
     167,    -1,   169,    12,   171,   172,   173,   174,   175,    -1,
     177,   178,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,   167,    -1,
     169,    12,   171,   172,    -1,   174,   175,    -1,   177,   178,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,   169,    12,
     171,   172,   173,   174,   175,    -1,   177,   178,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    41,    42,
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
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,   169,    12,   171,   172,
     173,   174,   175,    -1,   177,   178,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    83,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,   169,    12,   171,   172,    -1,   174,
     175,    -1,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    89,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,
      -1,    -1,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,   169,    12,   171,   172,    -1,   174,   175,    -1,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
     169,    12,   171,   172,   173,   174,   175,    -1,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,   169,    12,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,    -1,
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
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,   169,    12,   171,   172,
     173,   174,   175,    -1,   177,   178,    -1,    -1,    -1,    -1,
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
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,   169,    12,   171,   172,   173,   174,
     175,    -1,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    85,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,
      -1,    -1,   109,   110,   111,    -1,   113,   114,   115,    -1,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,   169,    12,   171,   172,    -1,   174,   175,    -1,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
     169,    12,   171,   172,   173,   174,   175,    -1,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,   169,    12,
     171,   172,   173,   174,   175,    -1,   177,   178,    -1,    -1,
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
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,   169,    12,   171,   172,
     173,   174,   175,    -1,   177,   178,    -1,    -1,    -1,    -1,
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
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,   169,    12,   171,   172,   173,   174,
     175,    -1,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,   169,    12,   171,   172,   173,   174,   175,    -1,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
     169,    12,   171,   172,    -1,   174,   175,    -1,   177,   178,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,
     111,    -1,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,   169,    12,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,    -1,
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
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,   169,    12,   171,   172,
      -1,   174,   175,    -1,   177,   178,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,    -1,    -1,    -1,    -1,    -1,   111,    -1,   113,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,   169,    12,   171,   172,    -1,   174,
     175,    -1,   177,   178,    -1,    -1,    -1,    -1,    -1,    26,
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
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,   169,    12,   171,   172,    -1,   174,   175,    -1,
     177,   178,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,
      -1,    -1,   111,    -1,   113,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
     169,    12,   171,   172,    -1,   174,   175,    -1,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,   169,    12,
      -1,    -1,    -1,   174,   175,    -1,   177,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,   169,    12,   171,    -1,
      -1,   174,   175,    -1,   177,   178,    -1,    -1,    -1,    -1,
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
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,   169,    12,   171,    -1,    -1,   174,
     175,    -1,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,   169,    12,   171,    -1,    -1,   174,   175,    -1,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    95,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
     169,    12,    -1,    -1,    -1,   174,   175,    -1,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,
     111,    -1,    -1,   114,   115,    -1,    71,    -1,    73,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,   111,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,    -1,    -1,   174,   175,    -1,   177,   178,     3,     4,
       5,     6,     7,    -1,    -1,    -1,   141,    12,    -1,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   169,    -1,    -1,    42,    43,   174,
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
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,   169,    12,    -1,    -1,    -1,   174,
     175,    -1,   177,   178,    -1,    -1,    -1,    -1,    -1,    26,
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
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,   169,    12,    -1,    -1,    -1,   174,   175,    -1,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
     169,    12,    -1,    -1,    -1,   174,   175,    -1,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,   169,    12,
      -1,    -1,    -1,   174,   175,    -1,   177,   178,    -1,    -1,
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
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,   169,    12,    -1,    -1,
      -1,   174,   175,    -1,   177,   178,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
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
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,   169,    12,    -1,    -1,    -1,   174,
     175,    -1,   177,   178,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,   169,    12,    -1,    -1,    -1,   174,   175,    -1,
     177,   178,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
     169,    12,    -1,    -1,    -1,   174,   175,    -1,   177,   178,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,   169,    12,
      -1,    -1,    -1,   174,   175,    -1,   177,   178,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    71,    -1,    73,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    71,
      -1,    73,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,   111,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,   169,    -1,   123,   111,
      -1,   174,   175,    -1,   177,   178,     3,     4,     5,     6,
       7,   123,     9,    10,    11,    12,   141,    -1,    -1,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,   141,
      -1,    -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   169,    -1,    -1,    -1,    -1,   174,
      -1,    -1,    49,    -1,    -1,    -1,    -1,   169,    -1,    -1,
      -1,    -1,   174,    -1,    -1,    62,    63,    64,    65,    66,
      67,    68,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   141,   142,   143,    -1,   145,    -1,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,   159,     9,    10,    11,    -1,    -1,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
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
      -1,    -1,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,   173,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    71,    -1,    73,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   173,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,   173,
      49,   141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,   169,
      -1,    -1,    -1,    -1,   174,    -1,    -1,    25,   171,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71,    -1,    73,    -1,    -1,    -1,
      25,   171,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,    -1,
      -1,    25,   171,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,   141,    49,    -1,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,   169,    -1,    -1,    -1,    -1,   174,    -1,    -1,
      -1,    -1,    25,   171,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   171,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    69,    70,    71,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    79,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,
      -1,    -1,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   156,    -1,     9,    10,    11,    -1,
     170,    -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   122,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   122,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     122,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    25,    49,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,     9,
      10,    11,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    90,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   180,   181,     0,     3,     4,     5,     6,     7,    12,
      41,    42,    43,    48,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      69,    70,    71,    72,    73,    75,    79,    80,    81,    82,
      84,    86,    88,    91,    95,    96,    97,    98,    99,   100,
     101,   103,   104,   105,   109,   110,   111,   113,   114,   115,
     116,   117,   118,   123,   124,   125,   126,   127,   128,   129,
     136,   141,   142,   143,   144,   145,   146,   148,   149,   151,
     152,   153,   154,   156,   160,   166,   167,   169,   171,   172,
     174,   175,   177,   178,   182,   185,   188,   189,   190,   191,
     192,   193,   196,   207,   208,   211,   216,   222,   278,   282,
     283,   284,   285,   286,   293,   294,   295,   297,   298,   301,
     311,   312,   313,   318,   321,   339,   344,   346,   347,   348,
     349,   350,   351,   352,   353,   355,   368,   370,   371,   111,
     123,   141,   185,   207,   285,   346,   285,   169,   285,   285,
     285,   337,   338,   285,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   111,   169,   189,   312,   313,
     346,   346,    31,   285,   359,   360,   285,   111,   169,   189,
     312,   313,   314,   345,   351,   356,   357,   169,   279,   315,
     169,   279,   280,   285,   198,   279,   169,   169,   169,   279,
     171,   285,   185,   171,   285,    25,    51,   124,   146,   166,
     169,   185,   372,   382,   383,   171,   285,   172,   285,   144,
     186,   187,   188,    73,   174,   246,   247,   117,   117,    73,
     207,   248,   169,   169,   169,   169,   185,   220,   373,   169,
     169,    73,    78,   137,   138,   139,   365,   366,   144,   172,
     188,   188,    95,   285,   221,   373,   146,   169,   185,   278,
     285,   286,   346,   194,   172,    78,   316,   365,    78,   365,
     365,    26,   144,   162,   374,   169,     8,   171,    31,   206,
     146,   219,   373,   171,   171,   171,     9,    10,    11,    25,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    49,   171,    61,    61,   172,   140,   118,   154,   207,
     222,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    59,    60,   121,   341,   342,    61,   172,
     343,   169,    61,   172,   174,   352,   169,   206,    13,   285,
      40,   185,   336,   169,   278,   346,   140,   346,   122,   176,
       8,   323,   278,   346,   374,   140,   169,   317,   121,   341,
     342,   343,   170,   285,    26,   196,     8,   171,   196,   197,
     280,   281,   285,   185,   234,   200,   171,   171,   171,   185,
     383,   383,   162,   169,    98,   375,   383,   374,    13,   185,
     171,   194,   171,   188,     8,   171,    90,   172,   346,     8,
     171,    13,   206,     8,   171,   346,   369,   369,   346,   170,
     162,   214,   123,   346,   358,   359,    61,   121,   137,   366,
      72,   285,   346,    78,   137,   366,   188,   184,   171,   172,
     171,   217,   302,   304,    79,   289,   291,    13,   170,   170,
     170,   173,   195,   196,   208,   211,   216,   285,   175,   177,
     178,   185,   375,    31,   244,   245,   285,   372,   169,   373,
     212,   285,   285,   285,    26,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   285,   285,   314,   285,   354,
     354,   285,   361,   362,   185,   351,   352,   220,   221,   206,
     219,    31,   145,   282,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   285,   172,   185,   351,   354,
     285,   244,   354,   285,   358,   170,   169,   335,     8,   323,
     278,   170,   185,    31,   285,    31,   285,   170,   170,   351,
     244,   172,   185,   351,   170,   194,   238,   285,    82,    26,
     196,   232,   171,    90,    13,     8,   170,    26,   172,   235,
     383,    79,   379,   380,   381,   169,     8,    42,    43,   124,
     136,   146,   166,   189,   190,   192,   296,   312,   318,   319,
     320,   173,    90,   187,   185,   285,   247,   319,   169,    73,
       8,   170,   170,   170,   171,   185,   378,   119,   225,   169,
       8,   170,   170,    73,    74,   185,   367,   185,    61,   173,
     173,   181,   183,   172,   161,    46,   146,   161,   306,   122,
       8,   323,   170,   383,   121,   341,   342,   343,   173,     8,
     163,   346,   170,     8,   324,    13,   287,   209,   119,   223,
     285,    26,   176,   176,   122,   173,     8,   323,   374,   169,
     215,   218,   373,   213,    63,   346,   285,   374,   169,   176,
     173,   170,   176,   173,   170,    42,    43,    61,    69,    70,
      79,   124,   136,   166,   185,   326,   328,   331,   334,   185,
     346,   346,   122,   341,   342,   343,   170,   285,   239,    66,
      67,   240,   279,   194,   281,    31,   229,   346,   319,   185,
      26,   196,   233,   171,   236,   171,   236,     8,   163,   122,
       8,   323,   170,   157,   375,   376,   383,   319,   319,   169,
      78,   140,   169,   140,   172,   102,   203,   204,   185,   173,
     288,    13,   346,   171,     8,    90,   163,   226,   312,   172,
     358,   123,   346,    13,   176,   285,   173,   181,   249,   305,
      13,   285,    79,   171,   172,   185,   351,   383,    31,   285,
     319,   157,   242,   243,   339,   340,   169,   312,   120,   224,
     285,   285,   285,   169,   244,   225,   172,   210,   223,   314,
     173,   169,   244,    13,    69,    70,   185,   327,   327,   328,
     329,   330,   169,    78,   137,   169,     8,   323,   170,   335,
      31,   285,   173,    66,    67,   241,   279,   196,   171,    83,
     171,   346,   122,   228,    13,   194,   236,    92,    93,    94,
     236,   173,   383,   383,   379,     8,   170,   170,   319,   322,
     325,   185,    79,   290,   292,   185,   319,   363,   364,   169,
     159,   242,   319,   378,   185,   382,     8,   249,   170,   169,
     282,   285,   176,   173,    99,   104,   106,   107,   108,   109,
     110,   111,   112,   148,   149,   151,   173,   250,   272,   273,
     274,   275,   277,   339,   147,   160,   172,   301,   308,   147,
     172,   307,   122,   285,   374,   169,   346,   170,     8,   324,
     383,   384,   242,   226,   172,   122,   244,   170,   172,   249,
     169,   224,   317,   169,   244,   170,   122,   176,     8,   323,
     329,   137,    79,   332,   333,   328,   346,   279,    26,    68,
     196,   171,   281,   229,   170,   319,    89,    92,   171,   285,
      26,   171,   237,   173,   163,   157,    26,   122,   170,     8,
     323,   122,     8,   323,   170,   122,   173,     8,   323,   312,
     172,   170,     8,   378,   312,   173,   358,   173,   372,   227,
     312,   112,   124,   146,   152,   259,   260,   261,   312,   150,
     265,   266,   115,   169,   185,   267,   268,   251,   207,   275,
     383,     8,   171,   273,   274,    46,   285,   285,   285,   173,
     169,   244,    26,   377,   157,   340,    31,    73,   170,   249,
     285,   170,   249,   173,   242,   172,   244,   170,   328,   328,
     170,   122,   170,     8,   323,    26,   194,   171,   170,   201,
     171,   171,   237,   194,   383,   319,   319,   319,    79,   319,
     319,    73,   194,   377,   378,   170,    13,     8,   171,   172,
     172,     8,   171,     3,     4,     5,     6,     7,     9,    10,
      11,    12,    49,    62,    63,    64,    65,    66,    67,    68,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   123,   124,   125,   126,   127,   128,   129,   141,   142,
     143,   145,   154,   156,   159,   167,   185,   309,   310,     8,
     171,   146,   150,   185,   268,   269,   270,   171,    73,   276,
     206,   252,   372,   207,   146,   303,   173,   173,   169,   244,
     170,   383,   104,   299,   384,    73,    13,   377,   173,   173,
     170,   249,   170,   122,   328,    79,   194,   199,    26,   196,
     231,   194,   170,   122,   122,   122,   170,   173,   299,   319,
     312,   255,   262,   318,   260,    13,    26,    43,   263,   266,
       8,    29,   170,    25,    42,    45,    13,     8,   171,   373,
     276,    13,   206,   244,   170,   169,   172,    31,    73,    13,
     319,   172,   377,   173,   328,   122,    26,   196,   230,   194,
     319,   319,   319,   172,   172,   173,   185,   192,   256,   257,
     258,     8,   173,   319,   310,   310,    51,   264,   269,   269,
      25,    42,    45,   319,    73,   169,   171,   319,   373,   170,
      31,    73,   300,   194,    73,    13,   319,   194,   172,   328,
     194,    87,   194,   194,   140,    90,   318,   153,    13,   253,
     169,    73,     8,   324,   173,    13,   319,   173,   194,    85,
     171,   173,   173,   185,   273,   274,   319,   242,   254,    31,
      73,   170,   319,   173,   171,   202,   155,   185,   171,   170,
     242,    73,   102,   203,   205,   227,   171,   377,   170,   169,
     171,   171,   172,   271,   377,   312,   194,   271,    73,   173,
     170,   172,   194,   173
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
	  (Current).line0   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).char0 = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).line1    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).char1  = YYRHSLOC (Rhs, N).last_column;	\
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
#line 820 "hphp.y"
    { _p->popLabelInfo();
                                         _p->saveParseTree((yyval));;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 830 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num() == T_DECLARE);
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 833 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { (yyval).reset();;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());
                                         (yyval).reset();;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 840 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 841 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 842 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 843 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 859 "hphp.y"
    { ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 860 "hphp.y"
    { ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 863 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 864 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 865 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 867 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 0;;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 879 "hphp.y"
    { (yyval).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         (yyval) = 0;;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 883 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num())
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 889 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num())
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 895 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num())
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 901 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                          on_constant(_p,(yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 903 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                          on_constant(_p,(yyval),  0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 910 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 919 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 923 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 930 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 933 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 935 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 939 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 942 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { _p->onBreak((yyval), NULL);;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->onBreak((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { _p->onContinue((yyval), NULL);;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { _p->onContinue((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { (yyval).reset();;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 956 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 960 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 963 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 971 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 976 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 984 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 997 "hphp.y"
    { (yyval).reset();;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 1001 "hphp.y"
    { finally_statement(_p);;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 1003 "hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (5)]));;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 1008 "hphp.y"
    { (yyval).reset();;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 1013 "hphp.y"
    { (yyval).reset();;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 1017 "hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1022 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1027 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(8) - (11)]),(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1031 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1036 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(9) - (12)]),(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1043 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 96:

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

  case 97:

/* Line 1455 of yacc.c  */
#line 1061 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 98:

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

  case 99:

/* Line 1455 of yacc.c  */
#line 1078 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1081 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1086 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1089 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1096 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (6)]),t_ext,t_imp,
                                                     (yyvsp[(5) - (6)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1108 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (7)]),t_ext,t_imp,
                                                     (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1116 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1121 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1124 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1127 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1133 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { (yyval).reset();;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1137 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { (yyval).reset();;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1141 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1142 "hphp.y"
    { (yyval).reset();;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1145 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1147 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1150 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1152 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1156 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { (yyval).reset();;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1160 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1165 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1167 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1170 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1172 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1175 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1177 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1180 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1182 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1192 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1200 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1202 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { (yyval).reset();;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1206 "hphp.y"
    { (yyval).reset();;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval).reset();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1212 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { (yyval).reset();;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1218 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { (yyval).reset();;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1222 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval).reset();;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1226 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { (yyval).reset();;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1232 "hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1234 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { (yyval).reset();;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1241 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,NULL,&(yyvsp[(1) - (3)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,NULL,&(yyvsp[(1) - (4)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1246 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,&(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1249 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,&(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,NULL,&(yyvsp[(3) - (5)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1255 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,NULL,&(yyvsp[(3) - (6)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1259 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,&(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1263 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,&(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1268 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1269 "hphp.y"
    { (yyval).reset();;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1272 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1275 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1277 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1281 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1282 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1286 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1291 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1293 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1295 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1300 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { (yyval).reset();;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1304 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1308 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1311 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1315 "hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1320 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1326 "hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1331 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1335 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1337 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1339 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1340 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1358 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1368 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1374 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1377 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1384 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1386 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1390 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1391 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1397 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1399 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1403 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1405 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1409 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1410 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1414 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1427 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1432 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1433 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1439 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1440 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1441 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1442 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1446 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1448 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1449 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1450 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1454 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1461 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1462 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { (yyval).reset();;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1467 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { (yyval).reset();;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1474 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { (yyval).reset();;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1478 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1484 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1485 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1486 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1487 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1492 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1495 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1506 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1516 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1520 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1521 "hphp.y"
    { (yyval).reset();;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1525 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1529 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1534 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1538 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1540 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1550 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1551 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1594 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1598 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),1);
                                         _p->popLabelInfo();;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { validate_shape_keyname((yyvsp[(3) - (5)]), _p);
                                        _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                        _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { validate_shape_keyname((yyvsp[(3) - (5)]), _p);
                                        _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                        _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1663 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { only_in_strict_mode(_p);
                                        _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1699 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1701 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1705 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1714 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { (yyval).reset();;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1719 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1729 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1732 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         (yyval).setText("");;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1746 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1752 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { (yyval).reset();;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1756 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { (yyval).reset();;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1760 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1764 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1767 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1770 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1778 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1782 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1786 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval).reset();;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { (yyval).reset();;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { (yyval).reset();;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { (yyval).reset();;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { only_in_strict_mode(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1948 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1950 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1960 "hphp.y"
    { (yyval).reset();;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval).reset();;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval).reset();;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { only_in_hphp_syntax(_p); (yyval).reset();;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval).reset();;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { only_in_strict_mode(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2014 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { (yyval).reset();;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2022 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { validate_shape_keyname((yyvsp[(3) - (5)]), _p);
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                         _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2051 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
    { (yyval).reset();;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2055 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { user_attribute_check(_p);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2077 "hphp.y"
    { (yyval).reset();;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2110 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2111 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2126 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2127 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2140 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2146 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2149 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2168 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2172 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2178 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2185 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval).reset();;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval)++;;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2215 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { (yyval).reset();;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2240 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2254 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2261 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2274 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2284 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2299 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2303 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { only_in_strict_mode(_p);
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)])); ;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2338 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]);
                                         only_in_strict_mode(_p); ;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2357 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2369 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (5)]).text()); ;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2407 "hphp.y"
    {;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2411 "hphp.y"
    { only_in_strict_mode(_p);
                                         (yyval).setText("array"); ;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2419 "hphp.y"
    { only_in_strict_mode(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { only_in_strict_mode(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2425 "hphp.y"
    { _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2429 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2431 "hphp.y"
    { only_in_strict_mode(_p);
                                         (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2435 "hphp.y"
    { only_in_strict_mode(_p);
                                         _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2445 "hphp.y"
    { only_in_strict_mode(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2449 "hphp.y"
    { only_in_strict_mode(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10411 "new_hphp.tab.cpp"
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
#line 2461 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

