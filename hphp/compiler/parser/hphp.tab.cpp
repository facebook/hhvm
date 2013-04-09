
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
#define YYLAST   11169

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  180
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  208
/* YYNRULES -- Number of rules.  */
#define YYNRULES  713
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1333

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
    1521,  1523,  1525,  1527,  1529,  1531,  1533,  1538,  1540,  1542,
    1544,  1546,  1548,  1550,  1552,  1554,  1557,  1559,  1560,  1561,
    1563,  1565,  1569,  1570,  1572,  1574,  1576,  1578,  1580,  1582,
    1584,  1586,  1588,  1590,  1592,  1594,  1598,  1601,  1603,  1605,
    1608,  1611,  1616,  1620,  1625,  1627,  1629,  1633,  1637,  1639,
    1641,  1643,  1645,  1649,  1653,  1657,  1660,  1661,  1663,  1664,
    1666,  1667,  1673,  1677,  1681,  1683,  1685,  1687,  1689,  1693,
    1696,  1698,  1700,  1702,  1704,  1706,  1709,  1712,  1717,  1721,
    1726,  1729,  1730,  1736,  1740,  1744,  1746,  1750,  1752,  1755,
    1756,  1762,  1766,  1769,  1770,  1774,  1775,  1780,  1783,  1784,
    1788,  1792,  1794,  1795,  1797,  1800,  1803,  1808,  1812,  1816,
    1819,  1824,  1827,  1832,  1834,  1836,  1838,  1840,  1842,  1845,
    1850,  1854,  1859,  1863,  1865,  1867,  1869,  1871,  1874,  1879,
    1884,  1888,  1890,  1892,  1896,  1904,  1911,  1920,  1930,  1939,
    1950,  1958,  1965,  1967,  1970,  1975,  1980,  1982,  1984,  1989,
    1991,  1992,  1994,  1997,  1999,  2001,  2004,  2009,  2013,  2017,
    2018,  2020,  2023,  2028,  2032,  2035,  2039,  2046,  2047,  2049,
    2054,  2057,  2058,  2064,  2068,  2072,  2074,  2081,  2086,  2091,
    2094,  2097,  2098,  2104,  2108,  2112,  2114,  2117,  2118,  2124,
    2128,  2132,  2134,  2137,  2140,  2142,  2145,  2147,  2152,  2156,
    2160,  2167,  2171,  2173,  2175,  2177,  2182,  2187,  2190,  2193,
    2198,  2201,  2204,  2206,  2210,  2214,  2220,  2222,  2225,  2227,
    2232,  2236,  2237,  2239,  2243,  2247,  2249,  2251,  2252,  2253,
    2256,  2260,  2262,  2267,  2273,  2277,  2281,  2285,  2289,  2291,
    2294,  2295,  2300,  2303,  2306,  2308,  2310,  2312,  2317,  2324,
    2326,  2335,  2341,  2343
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     181,     0,    -1,    -1,   182,   183,    -1,   183,   184,    -1,
      -1,   198,    -1,   210,    -1,   213,    -1,   218,    -1,   374,
      -1,   116,   170,   171,   172,    -1,   141,   190,   172,    -1,
      -1,   141,   190,   173,   185,   183,   174,    -1,    -1,   141,
     173,   186,   183,   174,    -1,   104,   188,   172,    -1,   195,
     172,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   188,     8,   189,    -1,   189,    -1,
     190,    -1,   144,   190,    -1,   190,    90,   187,    -1,   144,
     190,    90,   187,    -1,   187,    -1,   190,   144,   187,    -1,
     190,    -1,   141,   144,   190,    -1,   144,   190,    -1,   191,
      -1,   191,   377,    -1,   191,   377,    -1,   195,     8,   375,
      13,   322,    -1,    99,   375,    13,   322,    -1,   196,   197,
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
     372,   171,   172,    -1,   172,    -1,    75,    -1,    -1,    86,
     170,   287,    90,   231,   230,   171,   203,   233,    -1,    88,
     170,   236,   171,   235,    -1,   101,   173,   196,   174,   102,
     170,   315,    73,   171,   173,   196,   174,   204,   207,    -1,
     101,   173,   196,   174,   205,    -1,   103,   287,   172,    -1,
      96,   187,   172,    -1,   287,   172,    -1,   284,   172,    -1,
     285,   172,    -1,   286,   172,    -1,   187,    26,    -1,   204,
     102,   170,   315,    73,   171,   173,   196,   174,    -1,    -1,
      -1,   206,   159,   173,   196,   174,    -1,   205,    -1,    -1,
      31,    -1,    -1,    98,    -1,    -1,   209,   208,   376,   211,
     170,   244,   171,   380,   173,   196,   174,    -1,    -1,   342,
     209,   208,   376,   212,   170,   244,   171,   380,   173,   196,
     174,    -1,    -1,   224,   221,   214,   225,   226,   173,   251,
     174,    -1,    -1,   342,   224,   221,   215,   225,   226,   173,
     251,   174,    -1,    -1,   118,   222,   216,   227,   173,   251,
     174,    -1,    -1,   342,   118,   222,   217,   227,   173,   251,
     174,    -1,    -1,   154,   223,   219,   226,   173,   251,   174,
      -1,    -1,   342,   154,   223,   220,   226,   173,   251,   174,
      -1,   376,    -1,   146,    -1,   376,    -1,   376,    -1,   117,
      -1,   110,   117,    -1,   109,   117,    -1,   119,   315,    -1,
      -1,   120,   228,    -1,    -1,   119,   228,    -1,    -1,   315,
      -1,   228,     8,   315,    -1,   315,    -1,   229,     8,   315,
      -1,   122,   231,    -1,    -1,   349,    -1,    31,   349,    -1,
     198,    -1,    26,   196,    85,   172,    -1,   198,    -1,    26,
     196,    87,   172,    -1,   198,    -1,    26,   196,    83,   172,
      -1,   198,    -1,    26,   196,    89,   172,    -1,   187,    13,
     322,    -1,   236,     8,   187,    13,   322,    -1,   173,   238,
     174,    -1,   173,   172,   238,   174,    -1,    26,   238,    92,
     172,    -1,    26,   172,   238,    92,   172,    -1,   238,    93,
     287,   239,   196,    -1,   238,    94,   239,   196,    -1,    -1,
      26,    -1,   172,    -1,   240,    66,   281,   198,    -1,    -1,
     241,    66,   281,    26,   196,    -1,    -1,    67,   198,    -1,
      -1,    67,    26,   196,    -1,    -1,   245,     8,   157,    -1,
     245,   327,    -1,   157,    -1,    -1,   343,   387,    73,    -1,
     343,   387,    31,    73,    -1,   343,   387,    31,    73,    13,
     322,    -1,   343,   387,    73,    13,   322,    -1,   245,     8,
     343,   387,    73,    -1,   245,     8,   343,   387,    31,    73,
      -1,   245,     8,   343,   387,    31,    73,    13,   322,    -1,
     245,     8,   343,   387,    73,    13,   322,    -1,   247,   327,
      -1,    -1,   287,    -1,    31,   349,    -1,   247,     8,   287,
      -1,   247,     8,    31,   349,    -1,   248,     8,   249,    -1,
     249,    -1,    73,    -1,   175,   349,    -1,   175,   173,   287,
     174,    -1,   250,     8,    73,    -1,   250,     8,    73,    13,
     322,    -1,    73,    -1,    73,    13,   322,    -1,   251,   252,
      -1,    -1,    -1,   274,   253,   278,   172,    -1,    -1,   276,
     386,   254,   278,   172,    -1,   279,   172,    -1,    -1,   275,
     209,   208,   376,   170,   255,   244,   171,   380,   273,    -1,
      -1,   342,   275,   209,   208,   376,   170,   256,   244,   171,
     380,   273,    -1,   148,   261,   172,    -1,   149,   267,   172,
      -1,   151,   269,   172,    -1,   104,   229,   172,    -1,   104,
     229,   173,   257,   174,    -1,   257,   258,    -1,   257,   259,
      -1,    -1,   194,   140,   187,   155,   229,   172,    -1,   260,
      90,   275,   187,   172,    -1,   260,    90,   276,   172,    -1,
     194,   140,   187,    -1,   187,    -1,   262,    -1,   261,     8,
     262,    -1,   263,   312,   265,   266,    -1,   146,    -1,   124,
      -1,   315,    -1,   112,    -1,   152,   173,   264,   174,    -1,
     321,    -1,   264,     8,   321,    -1,    13,   322,    -1,    -1,
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
      73,    13,   322,    -1,    73,    -1,    73,    13,   322,    -1,
     279,     8,   375,    13,   322,    -1,    99,   375,    13,   322,
      -1,   170,   280,   171,    -1,    63,   317,   320,    -1,    62,
     287,    -1,   304,    -1,   170,   287,   171,    -1,   282,     8,
     287,    -1,   287,    -1,   282,    -1,    -1,   145,   287,    -1,
     349,    13,   284,    -1,   123,   170,   361,   171,    13,   284,
      -1,   288,    -1,   349,    -1,   280,    -1,   123,   170,   361,
     171,    13,   287,    -1,   349,    13,   287,    -1,   349,    13,
      31,   349,    -1,   349,    13,    31,    63,   317,   320,    -1,
     349,    24,   287,    -1,   349,    23,   287,    -1,   349,    22,
     287,    -1,   349,    21,   287,    -1,   349,    20,   287,    -1,
     349,    19,   287,    -1,   349,    18,   287,    -1,   349,    17,
     287,    -1,   349,    16,   287,    -1,   349,    15,   287,    -1,
     349,    14,   287,    -1,   349,    60,    -1,    60,   349,    -1,
     349,    59,    -1,    59,   349,    -1,   287,    27,   287,    -1,
     287,    28,   287,    -1,   287,     9,   287,    -1,   287,    11,
     287,    -1,   287,    10,   287,    -1,   287,    29,   287,    -1,
     287,    31,   287,    -1,   287,    30,   287,    -1,   287,    44,
     287,    -1,   287,    42,   287,    -1,   287,    43,   287,    -1,
     287,    45,   287,    -1,   287,    46,   287,    -1,   287,    47,
     287,    -1,   287,    41,   287,    -1,   287,    40,   287,    -1,
      42,   287,    -1,    43,   287,    -1,    48,   287,    -1,    50,
     287,    -1,   287,    33,   287,    -1,   287,    32,   287,    -1,
     287,    35,   287,    -1,   287,    34,   287,    -1,   287,    36,
     287,    -1,   287,    39,   287,    -1,   287,    37,   287,    -1,
     287,    38,   287,    -1,   287,    49,   317,    -1,   170,   288,
     171,    -1,   287,    25,   287,    26,   287,    -1,   287,    25,
      26,   287,    -1,   371,    -1,    58,   287,    -1,    57,   287,
      -1,    56,   287,    -1,    55,   287,    -1,    54,   287,    -1,
      53,   287,    -1,    52,   287,    -1,    64,   318,    -1,    51,
     287,    -1,   324,    -1,   297,    -1,   296,    -1,   176,   319,
     176,    -1,    12,   287,    -1,    -1,   209,   208,   170,   289,
     244,   171,   380,   302,   173,   196,   174,    -1,    -1,   111,
     209,   208,   170,   290,   244,   171,   380,   302,   173,   196,
     174,    -1,   300,    -1,   298,    -1,    79,    -1,   292,     8,
     291,   122,   287,    -1,   291,   122,   287,    -1,   293,     8,
     291,   122,   322,    -1,   291,   122,   322,    -1,   292,   326,
      -1,    -1,   293,   326,    -1,    -1,   166,   170,   294,   171,
      -1,   124,   170,   362,   171,    -1,    61,   362,   177,    -1,
     315,   173,   364,   174,    -1,   315,   173,   366,   174,    -1,
     300,    61,   357,   177,    -1,   301,    61,   357,   177,    -1,
     297,    -1,   373,    -1,   170,   288,   171,    -1,   104,   170,
     303,   327,   171,    -1,    -1,   303,     8,    73,    -1,   303,
       8,    31,    73,    -1,    73,    -1,    31,    73,    -1,   160,
     146,   305,   161,    -1,   307,    46,    -1,   307,   161,   308,
     160,    46,   306,    -1,    -1,   146,    -1,   307,   309,    13,
     310,    -1,    -1,   308,   311,    -1,    -1,   146,    -1,   147,
      -1,   173,   287,   174,    -1,   147,    -1,   173,   287,   174,
      -1,   304,    -1,   313,    -1,   312,    26,   313,    -1,   312,
      43,   313,    -1,   187,    -1,    64,    -1,    98,    -1,    99,
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
      -1,   143,    -1,   142,    -1,   169,    -1,   154,    -1,   156,
      -1,   167,    -1,   193,   170,   246,   171,    -1,   194,    -1,
     146,    -1,   315,    -1,   111,    -1,   355,    -1,   315,    -1,
     111,    -1,   359,    -1,   170,   171,    -1,   281,    -1,    -1,
      -1,    78,    -1,   368,    -1,   170,   246,   171,    -1,    -1,
      69,    -1,    70,    -1,    79,    -1,   128,    -1,   129,    -1,
     143,    -1,   125,    -1,   156,    -1,   126,    -1,   127,    -1,
     142,    -1,   169,    -1,   136,    78,   137,    -1,   136,   137,
      -1,   321,    -1,   192,    -1,    42,   322,    -1,    43,   322,
      -1,   124,   170,   325,   171,    -1,    61,   325,   177,    -1,
     166,   170,   295,   171,    -1,   323,    -1,   299,    -1,   194,
     140,   187,    -1,   146,   140,   187,    -1,   192,    -1,    72,
      -1,   373,    -1,   321,    -1,   178,   368,   178,    -1,   179,
     368,   179,    -1,   136,   368,   137,    -1,   328,   326,    -1,
      -1,     8,    -1,    -1,     8,    -1,    -1,   328,     8,   322,
     122,   322,    -1,   328,     8,   322,    -1,   322,   122,   322,
      -1,   322,    -1,    69,    -1,    70,    -1,    79,    -1,   136,
      78,   137,    -1,   136,   137,    -1,    69,    -1,    70,    -1,
     187,    -1,   329,    -1,   187,    -1,    42,   330,    -1,    43,
     330,    -1,   124,   170,   332,   171,    -1,    61,   332,   177,
      -1,   166,   170,   335,   171,    -1,   333,   326,    -1,    -1,
     333,     8,   331,   122,   331,    -1,   333,     8,   331,    -1,
     331,   122,   331,    -1,   331,    -1,   334,     8,   331,    -1,
     331,    -1,   336,   326,    -1,    -1,   336,     8,   291,   122,
     331,    -1,   291,   122,   331,    -1,   334,   326,    -1,    -1,
     170,   337,   171,    -1,    -1,   339,     8,   187,   338,    -1,
     187,   338,    -1,    -1,   341,   339,   326,    -1,    41,   340,
      40,    -1,   342,    -1,    -1,   345,    -1,   121,   354,    -1,
     121,   187,    -1,   121,   173,   287,   174,    -1,    61,   357,
     177,    -1,   173,   287,   174,    -1,   350,   346,    -1,   170,
     280,   171,   346,    -1,   360,   346,    -1,   170,   280,   171,
     346,    -1,   354,    -1,   314,    -1,   352,    -1,   353,    -1,
     347,    -1,   349,   344,    -1,   170,   280,   171,   344,    -1,
     316,   140,   354,    -1,   351,   170,   246,   171,    -1,   170,
     349,   171,    -1,   314,    -1,   352,    -1,   353,    -1,   347,
      -1,   349,   345,    -1,   170,   280,   171,   345,    -1,   351,
     170,   246,   171,    -1,   170,   349,   171,    -1,   354,    -1,
     347,    -1,   170,   349,   171,    -1,   349,   121,   187,   377,
     170,   246,   171,    -1,   349,   121,   354,   170,   246,   171,
      -1,   349,   121,   173,   287,   174,   170,   246,   171,    -1,
     170,   280,   171,   121,   187,   377,   170,   246,   171,    -1,
     170,   280,   171,   121,   354,   170,   246,   171,    -1,   170,
     280,   171,   121,   173,   287,   174,   170,   246,   171,    -1,
     316,   140,   187,   377,   170,   246,   171,    -1,   316,   140,
     354,   170,   246,   171,    -1,   355,    -1,   358,   355,    -1,
     355,    61,   357,   177,    -1,   355,   173,   287,   174,    -1,
     356,    -1,    73,    -1,   175,   173,   287,   174,    -1,   287,
      -1,    -1,   175,    -1,   358,   175,    -1,   354,    -1,   348,
      -1,   359,   344,    -1,   170,   280,   171,   344,    -1,   316,
     140,   354,    -1,   170,   349,   171,    -1,    -1,   348,    -1,
     359,   345,    -1,   170,   280,   171,   345,    -1,   170,   349,
     171,    -1,   361,     8,    -1,   361,     8,   349,    -1,   361,
       8,   123,   170,   361,   171,    -1,    -1,   349,    -1,   123,
     170,   361,   171,    -1,   363,   326,    -1,    -1,   363,     8,
     287,   122,   287,    -1,   363,     8,   287,    -1,   287,   122,
     287,    -1,   287,    -1,   363,     8,   287,   122,    31,   349,
      -1,   363,     8,    31,   349,    -1,   287,   122,    31,   349,
      -1,    31,   349,    -1,   365,   326,    -1,    -1,   365,     8,
     287,   122,   287,    -1,   365,     8,   287,    -1,   287,   122,
     287,    -1,   287,    -1,   367,   326,    -1,    -1,   367,     8,
     322,   122,   322,    -1,   367,     8,   322,    -1,   322,   122,
     322,    -1,   322,    -1,   368,   369,    -1,   368,    78,    -1,
     369,    -1,    78,   369,    -1,    73,    -1,    73,    61,   370,
     177,    -1,    73,   121,   187,    -1,   138,   287,   174,    -1,
     138,    72,    61,   287,   177,   174,    -1,   139,   349,   174,
      -1,   187,    -1,    74,    -1,    73,    -1,   114,   170,   372,
     171,    -1,   115,   170,   349,   171,    -1,     7,   287,    -1,
       6,   287,    -1,     5,   170,   287,   171,    -1,     4,   287,
      -1,     3,   287,    -1,   349,    -1,   372,     8,   349,    -1,
     316,   140,   187,    -1,   167,   376,    13,   386,   172,    -1,
     187,    -1,   386,   187,    -1,   187,    -1,   187,   162,   381,
     163,    -1,   162,   378,   163,    -1,    -1,   386,    -1,   378,
       8,   386,    -1,   378,     8,   157,    -1,   378,    -1,   157,
      -1,    -1,    -1,    26,   386,    -1,   192,     8,   381,    -1,
     192,    -1,   192,    90,   385,   381,    -1,   192,    90,   192,
       8,   381,    -1,   192,    90,   192,    -1,   192,    90,   385,
      -1,    79,   122,   386,    -1,   383,     8,   382,    -1,   382,
      -1,   383,   326,    -1,    -1,   166,   170,   384,   171,    -1,
      25,   386,    -1,    51,   386,    -1,   194,    -1,   124,    -1,
     385,    -1,   124,   162,   386,   163,    -1,   124,   162,   386,
       8,   386,   163,    -1,   146,    -1,   170,    98,   170,   379,
     171,    26,   386,   171,    -1,   170,   378,     8,   386,   171,
      -1,   386,    -1,    -1
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
    1503,  1507,  1508,  1510,  1511,  1514,  1518,  1519,  1523,  1524,
    1528,  1532,  1536,  1541,  1542,  1543,  1546,  1548,  1549,  1550,
    1553,  1554,  1555,  1556,  1557,  1558,  1559,  1560,  1561,  1562,
    1563,  1564,  1565,  1566,  1567,  1568,  1569,  1570,  1571,  1572,
    1573,  1574,  1575,  1576,  1577,  1578,  1579,  1580,  1581,  1582,
    1583,  1584,  1585,  1586,  1587,  1588,  1589,  1590,  1591,  1592,
    1593,  1595,  1596,  1598,  1600,  1601,  1602,  1603,  1604,  1605,
    1606,  1607,  1608,  1609,  1610,  1611,  1612,  1613,  1614,  1615,
    1616,  1617,  1619,  1618,  1627,  1626,  1634,  1635,  1639,  1643,
    1647,  1653,  1657,  1663,  1665,  1669,  1671,  1675,  1680,  1681,
    1685,  1692,  1699,  1701,  1706,  1707,  1708,  1712,  1716,  1720,
    1721,  1722,  1723,  1727,  1733,  1738,  1747,  1748,  1751,  1754,
    1757,  1758,  1761,  1765,  1768,  1771,  1778,  1779,  1783,  1784,
    1786,  1790,  1791,  1792,  1793,  1794,  1795,  1796,  1797,  1798,
    1799,  1800,  1801,  1802,  1803,  1804,  1805,  1806,  1807,  1808,
    1809,  1810,  1811,  1812,  1813,  1814,  1815,  1816,  1817,  1818,
    1819,  1820,  1821,  1822,  1823,  1824,  1825,  1826,  1827,  1828,
    1829,  1830,  1831,  1832,  1833,  1834,  1835,  1836,  1837,  1838,
    1839,  1840,  1841,  1842,  1843,  1844,  1845,  1846,  1847,  1848,
    1849,  1850,  1851,  1852,  1853,  1854,  1855,  1856,  1857,  1858,
    1859,  1860,  1861,  1862,  1863,  1864,  1868,  1873,  1874,  1877,
    1878,  1879,  1883,  1884,  1885,  1889,  1890,  1891,  1895,  1896,
    1897,  1900,  1902,  1906,  1907,  1908,  1910,  1911,  1912,  1913,
    1914,  1915,  1916,  1917,  1918,  1919,  1922,  1927,  1928,  1929,
    1930,  1931,  1933,  1934,  1937,  1938,  1942,  1945,  1951,  1952,
    1953,  1954,  1955,  1956,  1957,  1962,  1964,  1968,  1969,  1972,
    1973,  1977,  1980,  1982,  1984,  1988,  1989,  1990,  1992,  1995,
    1999,  2000,  2001,  2004,  2005,  2006,  2007,  2008,  2010,  2011,
    2017,  2019,  2022,  2025,  2027,  2029,  2032,  2034,  2038,  2040,
    2043,  2046,  2052,  2054,  2057,  2058,  2063,  2066,  2070,  2070,
    2075,  2078,  2079,  2083,  2084,  2089,  2090,  2094,  2095,  2099,
    2100,  2105,  2107,  2112,  2113,  2114,  2115,  2116,  2117,  2118,
    2120,  2123,  2125,  2129,  2130,  2131,  2132,  2133,  2135,  2137,
    2139,  2143,  2144,  2145,  2149,  2152,  2155,  2158,  2162,  2166,
    2173,  2177,  2184,  2185,  2190,  2192,  2193,  2196,  2197,  2200,
    2201,  2205,  2206,  2210,  2211,  2212,  2213,  2215,  2218,  2221,
    2222,  2223,  2225,  2227,  2231,  2232,  2233,  2235,  2236,  2237,
    2241,  2243,  2246,  2248,  2249,  2250,  2251,  2254,  2256,  2257,
    2261,  2263,  2266,  2268,  2269,  2270,  2274,  2276,  2279,  2282,
    2284,  2286,  2290,  2291,  2293,  2294,  2300,  2301,  2303,  2305,
    2307,  2309,  2312,  2313,  2314,  2318,  2319,  2320,  2321,  2322,
    2323,  2324,  2328,  2329,  2333,  2342,  2349,  2350,  2356,  2357,
    2365,  2368,  2372,  2375,  2380,  2381,  2382,  2383,  2387,  2388,
    2392,  2394,  2395,  2397,  2400,  2402,  2407,  2413,  2415,  2419,
    2422,  2425,  2434,  2437,  2440,  2441,  2444,  2445,  2449,  2454,
    2458,  2464,  2472,  2473
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
     279,   280,   280,   280,   280,   281,   282,   282,   283,   283,
     284,   285,   286,   287,   287,   287,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   288,   288,   288,   288,   288,   288,   288,   288,
     288,   288,   289,   288,   290,   288,   288,   288,   291,   292,
     292,   293,   293,   294,   294,   295,   295,   296,   297,   297,
     298,   299,   300,   300,   301,   301,   301,   302,   302,   303,
     303,   303,   303,   304,   305,   305,   306,   306,   307,   307,
     308,   308,   309,   310,   310,   311,   311,   311,   312,   312,
     312,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   313,   313,   313,   313,
     313,   313,   313,   313,   313,   313,   314,   315,   315,   316,
     316,   316,   317,   317,   317,   318,   318,   318,   319,   319,
     319,   320,   320,   321,   321,   321,   321,   321,   321,   321,
     321,   321,   321,   321,   321,   321,   321,   322,   322,   322,
     322,   322,   322,   322,   322,   322,   323,   323,   324,   324,
     324,   324,   324,   324,   324,   325,   325,   326,   326,   327,
     327,   328,   328,   328,   328,   329,   329,   329,   329,   329,
     330,   330,   330,   331,   331,   331,   331,   331,   331,   331,
     332,   332,   333,   333,   333,   333,   334,   334,   335,   335,
     336,   336,   337,   337,   338,   338,   339,   339,   341,   340,
     342,   343,   343,   344,   344,   345,   345,   346,   346,   347,
     347,   348,   348,   349,   349,   349,   349,   349,   349,   349,
     349,   349,   349,   350,   350,   350,   350,   350,   350,   350,
     350,   351,   351,   351,   352,   352,   352,   352,   352,   352,
     353,   353,   354,   354,   355,   355,   355,   356,   356,   357,
     357,   358,   358,   359,   359,   359,   359,   359,   359,   360,
     360,   360,   360,   360,   361,   361,   361,   361,   361,   361,
     362,   362,   363,   363,   363,   363,   363,   363,   363,   363,
     364,   364,   365,   365,   365,   365,   366,   366,   367,   367,
     367,   367,   368,   368,   368,   368,   369,   369,   369,   369,
     369,   369,   370,   370,   370,   371,   371,   371,   371,   371,
     371,   371,   372,   372,   373,   374,   375,   375,   376,   376,
     377,   377,   378,   378,   379,   379,   379,   379,   380,   380,
     381,   381,   381,   381,   381,   381,   382,   383,   383,   384,
     384,   385,   386,   386,   386,   386,   386,   386,   386,   386,
     386,   386,   387,   387
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
       1,     1,     1,     1,     1,     1,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     0,     0,     1,
       1,     3,     0,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     2,
       2,     4,     3,     4,     1,     1,     3,     3,     1,     1,
       1,     1,     3,     3,     3,     2,     0,     1,     0,     1,
       0,     5,     3,     3,     1,     1,     1,     1,     3,     2,
       1,     1,     1,     1,     1,     2,     2,     4,     3,     4,
       2,     0,     5,     3,     3,     1,     3,     1,     2,     0,
       5,     3,     2,     0,     3,     0,     4,     2,     0,     3,
       3,     1,     0,     1,     2,     2,     4,     3,     3,     2,
       4,     2,     4,     1,     1,     1,     1,     1,     2,     4,
       3,     4,     3,     1,     1,     1,     1,     2,     4,     4,
       3,     1,     1,     3,     7,     6,     8,     9,     8,    10,
       7,     6,     1,     2,     4,     4,     1,     1,     4,     1,
       0,     1,     2,     1,     1,     2,     4,     3,     3,     0,
       1,     2,     4,     3,     2,     3,     6,     0,     1,     4,
       2,     0,     5,     3,     3,     1,     6,     4,     4,     2,
       2,     0,     5,     3,     3,     1,     2,     0,     5,     3,
       3,     1,     2,     2,     1,     2,     1,     4,     3,     3,
       6,     3,     1,     1,     1,     4,     4,     2,     2,     4,
       2,     2,     1,     3,     3,     5,     1,     2,     1,     4,
       3,     0,     1,     3,     3,     1,     1,     0,     0,     2,
       3,     1,     4,     5,     3,     3,     3,     3,     1,     2,
       0,     4,     2,     2,     1,     1,     1,     4,     6,     1,
       8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   558,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   631,     0,   619,   477,
       0,   483,   484,    19,   509,   607,    70,   485,     0,    52,
       0,     0,     0,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,   470,     0,     0,
       0,     0,   112,     0,     0,     0,   489,   491,   492,   486,
     487,     0,     0,   493,   488,     0,     0,   468,    20,    21,
      22,    24,    23,     0,   490,     0,     0,     0,   494,     0,
      69,    42,   611,   478,     0,     0,     4,    31,    33,    36,
     508,     0,   467,     0,     6,    90,     7,     8,     9,     0,
     275,     0,     0,     0,     0,   273,   339,   338,   347,   346,
       0,   264,   574,   469,     0,   511,   337,     0,   577,   274,
       0,     0,   575,   576,   573,   602,   606,     0,   327,   510,
      10,   470,     0,     0,    31,    90,   671,   274,   670,     0,
     668,   667,   341,     0,     0,   311,   312,   313,   314,   336,
     334,   333,   332,   331,   330,   329,   328,   470,     0,   681,
     469,     0,   294,   292,     0,   635,     0,   518,   263,   473,
       0,   681,   472,     0,   482,   614,   613,   474,     0,     0,
     476,   335,     0,     0,     0,   267,     0,    50,   269,     0,
       0,    56,    58,     0,     0,    60,     0,     0,     0,   705,
     709,     0,     0,    31,   704,     0,   706,     0,    62,     0,
      42,     0,     0,     0,    26,    27,   176,     0,     0,   175,
     114,   113,   181,    90,     0,     0,     0,     0,     0,   678,
     100,   110,   627,   631,   656,     0,   496,     0,     0,     0,
     654,     0,    15,     0,    35,     0,   270,   104,   111,   379,
     354,     0,   275,     0,   273,   274,     0,     0,   479,     0,
     480,     0,     0,     0,    82,     0,     0,    38,   169,     0,
      18,    89,     0,   109,    96,   108,    79,    80,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   619,    78,   610,   610,   641,     0,
       0,     0,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   293,   291,     0,   578,
     563,   610,     0,   569,   169,   610,     0,   612,   603,   627,
       0,     0,     0,   560,   555,   518,     0,     0,     0,     0,
     639,     0,   359,   517,   630,     0,     0,    38,     0,   169,
     262,     0,   615,   563,   571,   475,     0,    42,   149,     0,
      67,     0,     0,   268,     0,     0,     0,     0,     0,    59,
      77,    61,   702,   703,     0,   700,     0,     0,   682,     0,
     677,    63,     0,    76,    28,     0,    17,     0,     0,   177,
       0,    65,     0,     0,     0,    66,   672,     0,     0,     0,
       0,     0,   120,     0,   628,     0,     0,     0,     0,   495,
     655,   509,     0,     0,   653,   514,   652,    34,     5,    12,
      13,    64,   118,     0,     0,   348,     0,   518,     0,     0,
     261,   324,   582,    47,    41,    43,    44,    45,    46,     0,
     340,   512,   513,    32,     0,     0,     0,   520,   170,     0,
     342,    92,   116,   297,   299,   298,     0,     0,   295,   296,
     300,   302,   301,   316,   315,   318,   317,   319,   321,   322,
     320,   310,   309,   304,   305,   303,   306,   307,   308,   323,
     609,     0,     0,   645,     0,   518,   674,   580,   602,   102,
     106,     0,    98,     0,     0,   271,   277,   290,   289,   288,
     287,   286,   285,   284,   283,   282,   281,   280,     0,   565,
     564,     0,     0,     0,     0,     0,     0,   669,   553,   557,
     517,   559,     0,     0,   681,     0,   634,     0,   633,     0,
     618,   617,     0,     0,   565,   564,   265,   151,   153,   266,
       0,    42,   133,    51,   269,     0,     0,     0,     0,   145,
     145,    57,     0,     0,   698,   518,     0,   687,     0,     0,
       0,   516,     0,     0,   468,     0,    36,   498,   467,   505,
       0,   497,    40,   504,    85,     0,    25,    29,     0,   174,
     182,   344,   179,     0,     0,   665,   666,    11,    36,   691,
       0,     0,     0,   627,   624,     0,   358,   664,   663,   662,
       0,   658,     0,   659,   661,     0,     5,     0,     0,   373,
     374,   382,   381,     0,     0,   517,   353,   357,     0,     0,
     579,   563,   570,   608,     0,   680,   171,   466,   519,   168,
       0,   562,     0,     0,   118,   326,     0,   362,   363,     0,
     360,   517,   640,     0,   169,   120,   118,    94,   116,   619,
     278,     0,     0,   169,   567,   568,   581,   604,   605,     0,
       0,     0,   541,   525,   526,   527,     0,     0,     0,   534,
     533,   547,   518,     0,   555,   638,   637,     0,   616,   563,
     572,   481,     0,   155,     0,     0,    48,     0,     0,     0,
       0,   126,   127,   137,     0,    42,   135,    73,   145,     0,
     145,     0,     0,   707,     0,   517,   699,   701,   686,   685,
       0,   683,   499,   500,   524,     0,   518,   516,     0,     0,
     356,     0,   647,     0,    75,     0,    30,   178,   562,     0,
     673,    68,     0,     0,   679,   119,   121,   184,     0,     0,
     625,     0,   657,     0,    16,     0,   117,   184,     0,     0,
     350,     0,   675,     0,   565,   564,   683,     0,   172,    39,
     158,     0,   520,   561,   713,   562,   115,     0,   325,   644,
     643,   169,     0,     0,     0,     0,   118,   482,   566,   169,
       0,     0,   530,   531,   532,   535,   536,   545,     0,   518,
     541,     0,   529,   549,   517,   552,   554,   556,     0,   632,
     566,     0,     0,     0,     0,   152,    53,     0,   269,   128,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   139,
       0,   696,   697,     0,     0,   711,     0,   502,   517,   515,
       0,   507,     0,   518,     0,   506,   651,     0,   518,     0,
       0,     0,   180,   690,   694,   695,     0,   246,   629,   627,
     272,   276,     0,    14,   246,   385,     0,     0,   387,   380,
     383,     0,   378,     0,     0,     0,   169,   173,   688,   562,
     157,   712,     0,     0,   184,     0,     0,   601,   184,   184,
     562,     0,   279,   169,     0,   595,     0,   538,   517,   540,
       0,   528,     0,     0,   518,   546,   636,     0,    42,     0,
     148,   134,     0,   125,    71,   138,     0,     0,   141,     0,
     146,   147,    42,   140,   708,   684,     0,   523,   522,   501,
       0,   517,   355,   503,     0,   361,   517,   646,     0,    42,
     688,     0,   692,   122,     0,     0,   249,   250,   251,   254,
     253,   252,   244,     0,     0,     0,   101,   183,   185,     0,
     243,   247,     0,   246,     0,   660,   105,   376,     0,     0,
     349,   566,   169,     0,     0,   368,   156,   713,     0,   160,
     688,   246,   642,   600,   246,   246,     0,   184,     0,   594,
     544,   543,   537,     0,   539,   517,   548,    42,   154,    49,
      54,     0,   136,   142,    42,   144,     0,     0,   352,     0,
     650,   649,     0,     0,   368,   693,     0,     0,   123,   213,
     211,   468,    24,     0,   207,     0,   212,   223,     0,   221,
     226,     0,   225,     0,   224,     0,    90,   248,   187,     0,
     189,     0,   245,   626,   377,   375,   386,   384,   169,     0,
     598,   689,     0,     0,     0,   161,     0,     0,    97,   103,
     107,   688,   246,   596,     0,   551,     0,   150,     0,    42,
     131,    72,   143,   710,   521,     0,     0,     0,    86,     0,
       0,     0,   197,   201,     0,     0,   194,   436,   435,   432,
     434,   433,   452,   454,   453,   424,   414,   430,   429,   392,
     401,   402,   404,   403,   423,   407,   405,   406,   408,   409,
     410,   411,   412,   413,   415,   416,   417,   418,   419,   420,
     422,   421,   393,   394,   395,   397,   398,   400,   438,   439,
     448,   447,   446,   445,   444,   443,   431,   449,   440,   441,
     442,   425,   426,   427,   428,   450,   451,   455,   457,   456,
     458,   459,   437,   461,   460,   396,   463,   464,   399,   465,
     462,   391,   218,   388,     0,   195,   239,   240,   238,   231,
       0,   232,   196,   257,     0,     0,     0,     0,    90,     0,
     597,     0,    42,     0,   164,     0,   163,    42,     0,    99,
     542,     0,    42,   129,    55,     0,   351,   648,    42,    42,
     260,   124,     0,     0,   215,   208,     0,     0,     0,   220,
     222,     0,     0,   227,   234,   235,   233,     0,     0,   186,
       0,     0,     0,     0,   599,     0,   371,   520,     0,   165,
       0,   162,     0,    42,   550,     0,     0,     0,     0,   198,
      31,     0,   199,   200,     0,     0,   214,   217,   389,   390,
       0,   209,   236,   237,   229,   230,   228,   258,   255,   190,
     188,   259,     0,   372,   519,     0,   343,     0,   167,    93,
       0,     0,   132,    84,   345,     0,   246,   216,   219,     0,
     562,   192,     0,   369,   367,   166,    95,   130,    88,   205,
       0,   245,   256,     0,   562,   370,     0,    87,    74,     0,
       0,   204,   688,     0,     0,     0,   203,     0,   688,     0,
     202,   241,    42,   191,     0,     0,     0,   193,     0,   242,
      42,     0,    83
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    96,   626,   438,   144,   223,   224,
      98,    99,   100,   101,   102,   103,   266,   454,   455,   382,
     196,  1078,   388,  1011,  1298,   744,   745,  1308,   282,   145,
     456,   652,   795,   457,   472,   668,   422,   665,   458,   442,
     666,   284,   240,   257,   109,   654,   628,   612,   755,  1027,
     831,   711,  1204,  1081,   563,   717,   387,   571,   719,   932,
     558,   703,   706,   823,   781,   782,   466,   467,   228,   229,
     234,   867,   967,  1045,  1186,  1290,  1304,  1212,  1252,  1253,
    1254,  1033,  1034,  1035,  1213,  1219,  1261,  1038,  1039,  1043,
    1179,  1180,  1181,  1323,   968,   969,   970,   971,  1184,   972,
     110,   190,   383,   384,   111,   112,   113,   114,   115,   651,
     748,   446,   447,   853,   448,   854,   116,   117,   118,   589,
     119,   120,  1063,  1237,   121,   443,  1055,   444,   768,   633,
     882,   879,  1172,  1173,   122,   123,   124,   184,   191,   269,
     370,   125,   734,   593,   126,   735,   364,   649,   736,   690,
     805,   807,   808,   809,   692,   913,   914,   693,   539,   355,
     153,   154,   127,   784,   339,   340,   642,   128,   185,   147,
     130,   131,   132,   133,   134,   135,   136,   501,   137,   187,
     188,   425,   176,   177,   504,   505,   857,   858,   249,   250,
     620,   138,   417,   139,   140,   215,   241,   277,   397,   730,
     985,   610,   574,   575,   576,   216,   217,   892
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -950
static const yytype_int16 yypact[] =
{
    -950,   102,  -950,  -950,  3655,  9850,  9850,   -53,  9850,  9850,
    9850,  -950,  9850,  9850,  9850,  9850,  9850,  9850,  9850,  9850,
    9850,  9850,  9850,  9850,  2237,  2237,  3119,  9850,  2538,   -50,
     -47,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  9850,  -950,
     -47,   -33,   -25,    -7,   -47,  7549,   972,  7726,  -950,  1378,
    7903,    -3,  9850,   964,   108,    61,   174,    20,   111,   126,
     137,   155,  -950,   972,   162,   166,  -950,  -950,  -950,  -950,
    -950,   319,   362,  -950,  -950,   972,  8080,  -950,  -950,  -950,
    -950,  -950,  -950,   972,  -950,   195,   178,   972,  -950,  9850,
    -950,  -950,   185,   316,   357,   357,  -950,   347,   235,   446,
    -950,   215,  -950,    38,  -950,   360,  -950,  -950,  -950,   595,
    -950,   227,   229,   244,  8243,  -950,  -950,   359,  -950,   361,
     399,  -950,    71,   278,   322,  -950,  -950,    26,    12,  2131,
      73,   295,    79,    97,   297,    14,  -950,   145,  -950,   414,
    -950,   379,   309,   350,  -950,   360, 11120,  2519, 11120,  9850,
   11120, 11120,  3818,   462,   972,  -950,  -950,   460,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  1890,   354,
    -950,   384,   398,   398,  2237,  2561,   367,   523,  -950,   405,
    1890,   354,   408,   409,   380,    99,  -950,   430,    73,  8257,
    -950,  -950,  9850,  6487,    42, 11120,  7372,  -950,  9850,  9850,
     972,  -950,  -950,  9659,   382,  -950,  9836,  1378,  1378,   393,
    -950,   386,  1008,   553,  -950,   554,  -950,   972,  -950, 10013,
    -950, 10722,   972,    43,  -950,   -19,  -950,  1307,    45,  -950,
    -950,  -950,   556,   360,    47,  2237,  2237,  2237,   404,   415,
    -950,  -950,  2061,  3119,    65,    -8,  -950, 10027,  2237,   348,
    -950,   972,  -950,   -31,   235,   410, 11120,  -950,  -950,  -950,
     482,   577,   417, 11120,   421,   764,  3832,  9850,   204,   422,
     420,   204,   271,   233,  -950,   972,  1378,   429,  8434,  1378,
    -950,  -950,   516,  -950,  -950,  -950,  -950,  -950,  -950,  9850,
    9850,  9850,  8611,  9850,  9850,  9850,  9850,  9850,  9850,  9850,
    9850,  9850,  9850,  9850,  9850,  9850,  9850,  9850,  9850,  9850,
    9850,  9850,  9850,  9850,  2538,  -950,  9850,  9850,  9850,   390,
     972,   972,   360,   595,  2819,  9850,  9850,  9850,  9850,  9850,
    9850,  9850,  9850,  9850,  9850,  9850,  -950,  -950,   432,  -950,
     107,  9850,  9850,  -950,  8434,  9850,  9850,   185,   113,  2061,
     431,  8788, 10763,  -950,   433,   592,  1890,   438,   -44,   390,
     398,  8965,  -950,  9142,  -950,   440,   -20,  -950,   193,  8434,
    -950,   539,  -950,   114,  -950,  -950, 10827,  -950,  -950,  9850,
    -950,   531,  6664,   607,   445, 11013,   613,    78,    70,  -950,
    -950,  -950,  -950,  -950,  1378,   555,   457,   624,  -950,  2187,
    -950,  -950,  4009,  -950,   189,   964,  -950,   972,  9850,   398,
     108,  -950,  2187,   463,   563,  -950,   398,    80,    83,   164,
     466,   872,   526,   478,   398,    84,   479,   803,   972,  -950,
    -950,   590,  2990,   203,  -950,  -950,  -950,   235,  -950,  -950,
    -950,  -950,   533,   493,    23,  -950,   534,   647,   502,  1378,
      50,   622,    94,  -950,  -950,  -950,  -950,  -950,  -950, 10306,
    -950,  -950,  -950,  -950,    40,  2237,   503,   676, 11120,   680,
    -950,  -950,   578,  7712,  3644,  3818,  9850, 11079,  3012,  3468,
    4169,  4345,  4521,  4698,  4698,  4698,  4698,  2657,  2657,  2657,
    2657,  1271,  1271,   548,   548,   548,   460,   460,   460,  -950,
   11120,   512,   517, 10868,   522,   691,   -48,   530,   113,  -950,
    -950,   972,  -950,  1075,  9850,  -950,  3818,  3818,  3818,  3818,
    3818,  3818,  3818,  3818,  3818,  3818,  3818,  3818,  9850,   -48,
     532,   543, 10347,   558,   559, 10388,    87,  -950,  1807,  -950,
     972,  -950,   417,    50,   354,  2237, 11120,  2237, 10910,    72,
     115,  -950,   560,  9850,  -950,  -950,  -950,  6310,    82, 11120,
     -47,  -950,  -950,  -950,  9850,   377,  2187,   972,  6841,   561,
     562,  -950,    59,   581,  -950,   717,   574,  1141,  1378,  2187,
    2187,  2187,   580,    19,   612,   583,   242,  -950,   614,  -950,
     582,  -950,  -950,  -950,   657,   972,  -950,  -950, 10429,  -950,
    -950,  -950,   753,  2237,   588,  -950,  -950,  -950,  -950,   257,
     605,   796,   597,  2061,  2121,   759,  -950,  -950,  -950,  -950,
     596,  -950,  9850,  -950,  -950,  3301,  -950,   796,   603,  -950,
    -950,  -950,  -950,   776,  9850,   482,  -950,  -950,   620,   557,
    -950,   116,  -950,  -950,  1378,  -950,   398,  -950,  9319,  -950,
    2187,     8,   629,   796,   533,  3995,  9850,  -950,  -950,  9850,
    -950,  9850,  -950,   630,  8434,   526,   533,  -950,   578,  2538,
     398, 10473,   631,  8434,  -950,  -950,   127,  -950,  -950,   789,
     766,   766,  1807,  -950,  -950,  -950,   634,   191,   635,  -950,
    -950,  -950,   799,   637,   433,   398,   398,  9496,  -950,   130,
    -950,  -950, 10514,   246,   -47,  7372,  -950,   639,  4186,   641,
    2237,   687,   398,  -950,   801,  -950,  -950,  -950,  -950,   344,
    -950,    16,  1378,  -950,  1378,   555,  -950,  -950,  -950,   808,
     646,   650,  -950,  -950,   703,   649,   820,  2187,   694,   972,
     482,   972,  2187,   663,  -950,   685,  -950,  -950,     8,  2187,
     398,  -950,   872,   747,  -950,   831,  -950,  -950,    90,   675,
     398,  9673,  -950,  2623,  -950,  3478,   831,  -950,   373,   -35,
   11120,   724,  -950,  9850,   -48,   679,  -950,  2237, 11120,  -950,
    -950,   681,   843,  -950,  1378,     8,  -950,   682,  3995, 11120,
   10972,  8434,   686,   689,   690,   683,   533,   380,   688,  8434,
     698,  9850,  -950,  -950,  -950,  -950,  -950,   738,   693,   863,
    1807,   749,  -950,   482,  1807,  -950,  -950,  -950,  2237, 11120,
    -950,   -47,   858,   821,  7372,  -950,  -950,   720,  9850,   398,
     377,   723,  2187,  4363,   389,   725,  9850,    44,   180,  -950,
     742,  -950,  -950,  1200,   875,  -950,  2187,  -950,  2187,  -950,
     735,  -950,   786,   902,   740,  -950,   790,   748,   908,   796,
     750,   754,  -950,  -950,   916,   872,   796,  1265,  -950,  2061,
    -950,  3818,   760,  -950,  1384,  -950,    75,  9850,  -950,  -950,
    -950,  9850,  -950,  9850, 10555,   768,  8434,   398,   910,    37,
    -950,  -950,   273,   770,  -950,  9850,   782,  -950,  -950,  -950,
       8,   784,  -950,  8434,   787,  -950,  1807,  -950,  1807,  -950,
     788,  -950,   817,   791,   952,  -950,   398,   938,  -950,   793,
    -950,  -950,   798,  -950,  -950,  -950,   795,   800,  -950, 10681,
    -950,  -950,  -950,  -950,  -950,  -950,  1378,  -950,   855,  -950,
    2187,   482,  -950,  -950,  2187,  -950,  2187,  -950,   905,  -950,
     910,   872,  -950,  -950,  1378,   796,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  1108,   829,   642,  -950,  -950,  -950,   379,
     822,  -950,    48,  1283,    92,  -950,  -950,   834, 10596, 10640,
   11120,   812,  8434,   813,  1378,   879,  -950,  1378,   912,   974,
     910,  1509, 11120,  -950,  1541,  1644,   818,  -950,   819,  -950,
    -950,   871,  -950,  1807,  -950,   482,  -950,  -950,  6310,  -950,
    -950,  7018,  -950,  -950,  -950,  6310,   823,  2187,  -950,   880,
    -950,   881,   835,  4540,   879,  -950,   992,    33,  -950,  -950,
    -950,    52,   836,    58,  -950, 10155,  -950,  -950,    60,  -950,
    -950,   491,  -950,   838,  -950,   935,   360,  -950,  -950,  1378,
    -950,   379,  1283,  -950,  -950,  -950,  -950,  -950,  8434,   841,
    -950,  -950,   844,   842,   279,  1005,  2187,   853,  -950,  -950,
    -950,   910,  1724,  -950,  1807,  -950,   907,  6310,  7195,  -950,
    -950,  -950,  6310,  -950,  -950,  2187,  2187,   857,  -950,   865,
    2187,   796,  -950,  -950,  1441,  1108,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,   327,  -950,   829,  -950,  -950,  -950,  -950,  -950,
      56,   276,  -950,  1014,    64,   972,   935,  1023,   360,   868,
    -950,   283,  -950,   968,  1029,  2187,  -950,  -950,   873,  -950,
    -950,  1807,  -950,  -950,  -950,  4717,  -950,  -950,  -950,  -950,
    -950,  -950,   575,    35,  -950,  -950,  2187, 10155, 10155,   993,
    -950,   491,   491,   427,  -950,  -950,  -950,  2187,   975,  -950,
     882,    68,  2187,   972,  -950,   977,  -950,  1043,  4894,  1040,
    2187,  -950,  5071,  -950,  -950,  5248,   884,  5425,  5602,  -950,
     967,   914,  -950,  -950,   970,  1441,  -950,  -950,  -950,  -950,
     911,  -950,  1034,  -950,  -950,  -950,  -950,  -950,  1045,  -950,
    -950,  -950,   897,  -950,   296,   898,  -950,  2187,  -950,  -950,
    5779,   899,  -950,  -950,  -950,   972,  1283,  -950,  -950,  2187,
       8,  -950,  1002,  -950,  -950,  -950,  -950,  -950,   -12,   921,
     972,   336,  -950,   906,     8,  -950,   915,  -950,  -950,   796,
     920,  -950,   910,   917,   796,    76,  -950,   255,   910,  1009,
    -950,  -950,  -950,  -950,   255,   922,  5956,  -950,   913,  -950,
    -950,  6133,  -950
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -950,  -950,  -950,  -398,  -950,  -950,  -950,    -4,  -950,   692,
     -18,   854,   -90,  -950,  1335,  -950,  -211,  -950,     6,  -950,
    -950,  -950,  -950,  -950,  -950,  -204,  -950,  -950,  -139,    32,
      -2,  -950,  -950,    -1,  -950,  -950,  -950,  -950,     4,  -950,
    -950,   772,   767,   777,   973,   436,  -567,   434,   474,  -202,
    -950,   280,  -950,  -950,  -950,  -950,  -950,  -950,  -523,   198,
    -950,  -950,  -950,  -950,  -733,  -950,  -307,  -950,  -950,   708,
    -950,  -737,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,    27,  -950,  -950,  -950,  -950,  -950,   -46,  -950,
     171,  -790,  -950,  -195,  -950,  -949,  -947,  -937,   -55,  -950,
     -61,   -23,  1102,  -526,  -308,  -950,  -950,  2438,  1052,  -950,
    -950,  -612,  -950,  -950,  -950,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,   119,  -950,   376,  -950,  -950,  -950,  -950,  -950,
    -950,  -950,  -950,  -747,  -950,  1160,  1419,  -294,  -950,  -950,
     356,  1276,  -368,  -950,  -950,   418,  -341,  -771,  -950,  -950,
     464,  -506,   337,  -950,  -950,  -950,  -950,  -950,   468,  -950,
    -950,  -950,  -632,   269,  -153,  -124,  -108,  -950,  -950,    57,
    -950,  -950,  -950,  -950,    46,  -112,  -950,    66,  -950,  -950,
    -950,  -337,   924,  -950,  -950,  -950,  -950,  -950,   406,   570,
    -950,  -950,   927,  -950,  -950,  -275,   -82,  -168,  -247,  -950,
    -932,  -713,   439,  -950,  -950,   412,  -104,   182
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -682
static const yytype_int16 yytable[] =
{
      97,   258,   106,   107,   469,   261,   350,   193,   108,   402,
     104,   890,   536,   367,   541,   861,   515,   197,  1024,   783,
     499,   201,   343,   771,  1051,   348,  1052,   285,   262,   464,
     874,   592,   691,  1047,   372,   225,   105,   533,   709,   863,
     625,  1091,   204,  1255,   600,   213,   279,   721,   644,    11,
     379,   405,   893,   410,   253,   414,  1049,   254,  1067,   239,
    -210,   129,   552,   373,  1221,   244,  1095,   722,  1174,   630,
     930,   407,  1228,  -586,   186,   345,  1228,   338,    11,   239,
     374,   172,   173,   239,  1091,  1222,   567,   787,   603,   233,
    1306,   603,   614,   232,   413,   614,   569,   738,   614,   794,
     614,   338,     3,   392,   393,   239,   636,   357,   398,   836,
     837,   341,   880,   275,   276,  1047,   783,   149,    48,   365,
     189,   977,  -681,   192,    48,   275,   427,   452,   852,   429,
     247,   248,  -583,   341,   341,    55,    56,   198,   881,  1198,
    -584,   439,   440,    62,   320,   199,   265,   -85,   704,   705,
     354,   550,   952,   783,  -471,  -590,   246,   991,  -585,   322,
    -620,   994,   995,   200,   662,   780,   557,   996,  -587,   631,
     220,   639,   398,   233,   345,  -621,  -623,  -588,   230,  -159,
     321,   226,  -592,   511,   632,  -586,   428,   346,  -589,    97,
     839,  -622,    97,   371,   986,   834,   386,   838,   713,   378,
     471,   912,   381,   645,   404,  1092,  1093,   508,  -519,  1256,
     280,   732,   733,   400,   380,   406,   931,   411,    35,   415,
    1050,   259,   723,   342,  -210,   358,   508,  1223,   765,   901,
    1096,   360,  1175,   437,   726,   973,  1229,   366,  1025,   258,
    1270,   285,   973,   570,  -583,   342,   342,   508,  1320,   568,
     129,   604,  -584,   129,   605,   615,   508,   783,   679,   508,
    1072,   868,    97,  1053,  -593,   752,    35,  -590,   783,   811,
    -585,   463,  -620,   836,   837,   213,   758,   244,   239,   595,
    -587,   235,   779,   227,   409,   338,   346,  -621,  -623,  -588,
     572,   231,   416,   416,   419,   542,   236,   640,   105,   424,
    -589,  1224,   922,  -622,   988,   433,   244,   237,   915,   587,
    1193,   434,   821,   822,  1235,   506,   239,   239,  1225,   239,
     347,  1226,   587,   129,   338,   238,   641,  1292,   812,  1019,
     729,   609,   242,   275,   529,   606,   243,  1300,   663,  1301,
    1216,   259,   247,   248,   244,   638,   989,   753,   260,   434,
     708,   815,  1194,  1217,   933,   544,  1236,   792,   267,   973,
     186,   672,   973,   973,  1047,   507,   800,   554,    92,  1293,
    1218,   247,   248,   274,   856,   797,   663,   624,    97,   275,
    1317,   862,  -681,   502,   530,   278,  1324,   225,   562,   244,
     640,   281,   244,  1076,   268,   849,   698,   245,    97,   286,
    1000,   287,  1001,   597,   276,   507,   424,   531,   710,   247,
     248,   534,   462,   358,   551,  -681,   288,   555,   367,   641,
    -364,   244,   316,   619,   621,   699,   434,  1321,  1322,   667,
     244,  1262,  1263,    33,   105,   271,   835,   836,   837,   129,
     973,   700,   956,   957,   958,   959,   960,   961,    33,   461,
      35,   318,  1264,   870,   247,   248,   246,   247,   248,   129,
     317,    33,   319,    35,   925,   344,  1275,  -591,   909,  1265,
    1258,  1259,  1266,   398,   731,  -365,   587,    48,   937,   349,
     938,   927,   836,   837,   896,   435,   247,   248,   167,   587,
     587,   587,   904,   244,   251,   247,   248,  1075,   434,   270,
     272,   273,   353,    33,   833,    35,   251,   239,  1311,   314,
      78,    79,   942,    80,    81,    82,   276,   947,   143,   338,
     875,    75,   646,    77,   359,    78,    79,   508,    80,    81,
      82,   363,   974,   876,   689,   252,   694,   707,    78,    79,
     776,    80,    81,    82,   362,  -470,   877,   168,  -469,   368,
     369,   371,    92,    97,   390,   394,   395,  1303,   247,   248,
     587,   445,    33,   714,    97,    92,  -676,   399,  1200,   412,
     670,  1313,  1018,  1006,   716,   420,  1020,   421,  1021,   983,
      78,    79,   441,    80,    81,    82,  -681,    33,   450,   105,
     449,   746,   451,   311,   312,   313,   998,   314,   460,   -37,
     540,   470,   695,   538,   696,   528,   885,    92,   276,   543,
      33,   549,    35,   560,   129,   379,  -681,   564,   840,  -681,
     841,    97,   712,   106,   107,   129,   566,   577,    33,   108,
      35,   104,   578,   601,   573,   774,   602,  1176,   607,    78,
      79,  1177,    80,    81,    82,   611,    33,   587,   613,  1084,
     616,   622,   587,   627,   629,   635,   634,   105,   783,   587,
     750,  1041,   609,   864,    78,    79,    33,    80,    81,    82,
     424,   760,   783,   637,   647,  1059,   804,   804,   689,  1026,
     891,   824,   129,  -366,   648,   775,   470,    78,    79,   657,
      80,    81,    82,   650,   658,  1244,   660,   653,  1196,   661,
     664,    97,   673,   724,    97,    78,    79,  1008,    80,    81,
      82,   825,   553,    33,    92,   186,   143,  1206,  1207,    75,
     674,  1015,  1210,    78,    79,   725,    80,    81,    82,   676,
     773,   701,    92,   718,   720,   851,   677,   855,  1023,   776,
     105,   283,   587,    78,    79,   727,    80,    81,    82,  1249,
     737,  1189,   739,   740,   741,   742,   587,  1040,   587,   743,
     751,    97,   129,   106,   107,   129,   749,   829,   754,   108,
     757,   104,   761,   762,  1187,   609,   767,   351,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   769,
      78,    79,   772,    80,    81,    82,  1077,   105,   917,   785,
     791,   799,   801,  1082,   810,   813,   689,   814,   816,   830,
     689,   826,  1041,   828,   832,   430,   843,   844,    33,   436,
      97,   845,   129,   336,   337,   846,   847,  1241,   848,    97,
     920,   429,  1016,   859,   887,   802,   803,    33,   430,   866,
     436,   430,   436,   436,   860,   869,   883,   207,  1257,   886,
     587,   889,   888,   900,   587,   894,   587,   897,   903,  1267,
     906,   609,   898,   899,  1271,   105,  1048,    33,  1205,   905,
     907,   908,  1278,   208,    33,   916,   617,   618,   169,   169,
    1061,   129,   181,   891,   918,   338,   911,   712,   143,   919,
     129,    75,   921,    33,   924,    78,    79,   928,    80,    81,
      82,   936,   689,   181,   689,   934,   939,  1185,   940,  1295,
     941,   943,   944,   211,    78,    79,   946,    80,    81,    82,
    -245,  1302,   945,   949,   951,   950,   424,   587,   956,   957,
     958,   959,   960,   961,   975,   452,   984,   143,   982,  1003,
      75,   990,    77,    33,    78,    79,   209,    80,    81,    82,
     213,    78,    79,   993,    80,    81,    82,   997,   999,  1002,
    1005,  1042,  1004,   143,  1007,  1009,    75,  1012,   210,  1010,
      78,    79,  1013,    80,    81,    82,   587,  1017,  1022,  1037,
    1054,  1238,  1058,  1062,  1060,  1065,  1242,  1066,   211,  1071,
    1073,  1245,   212,  1074,  1083,   587,   587,  1247,  1248,   689,
     587,  1046,  1085,  1086,    97,  1090,  1087,    97,  1183,  1094,
    1182,    97,  1190,   143,  1191,  1192,    75,  1080,  1195,    97,
      78,    79,   169,    80,    81,    82,  1197,  1227,   169,  1201,
    1208,  1171,  1280,   207,   169,    33,  1232,  1178,  1209,  1234,
     105,  1239,  1240,    33,  1260,   213,  1243,   105,  1268,  1233,
    1273,  1274,  1269,  1277,  1285,   105,  1282,  -206,  1289,   208,
    1286,   181,   181,  1222,  1288,   129,   181,  1291,   129,  1294,
     689,  1297,   129,    97,    97,  1305,  1309,  1312,    97,    33,
     129,   169,  1325,  1188,  1203,  1314,  1330,   509,  1318,   169,
     169,   169,  1316,  1328,  1307,   512,   169,   596,   510,   793,
     323,   766,   169,  1230,   796,   587,   396,  1315,   222,   105,
     923,  1326,    78,    79,   105,    80,    81,    82,   599,  1331,
      78,    79,  1215,    80,    81,    82,   587,  1014,  1220,  1327,
     181,  1231,   209,   181,   129,   129,  1044,   587,   669,   129,
     194,   264,   587,  1089,   878,   806,    33,   910,    35,   143,
     587,  1272,    75,   902,   210,   850,    78,    79,   987,    80,
      81,    82,   817,   418,   842,   865,   207,   426,   181,  1064,
       0,     0,     0,     0,   211,     0,     0,     0,   212,    33,
       0,   239,     0,     0,   170,   170,   167,   587,   182,     0,
       0,     0,   208,     0,     0,     0,     0,   689,     0,   587,
       0,    97,     0,   169,     0,     0,     0,     0,  1250,     0,
     169,     0,    33,  1171,  1171,     0,   143,  1178,  1178,    75,
    1029,    77,     0,    78,    79,   207,    80,    81,    82,   239,
       0,     0,  1030,     0,    97,     0,     0,   105,    97,     0,
       0,    97,     0,    97,    97,   168,     0,     0,   181,   143,
      92,   208,    75,   586,  1031,     0,    78,    79,     0,    80,
    1032,    82,   129,     0,     0,   209,   586,     0,     0,     0,
     105,    33,     0,     0,   105,   608,    97,   105,     0,   105,
     105,  1299,   143,     0,     0,    75,     0,   210,     0,    78,
      79,     0,    80,    81,    82,   129,  1310,     0,   728,   129,
       0,     0,   129,   181,   129,   129,    11,   211,     0,     0,
       0,   212,   105,   308,   309,   310,   311,   312,   313,   169,
     314,     0,    97,     0,   209,     0,     0,    97,   170,     0,
       0,     0,     0,     0,   170,     0,     0,   129,     0,     0,
     170,   143,     0,     0,    75,     0,   210,     0,    78,    79,
       0,    80,    81,    82,     0,     0,     0,   935,   105,     0,
       0,     0,     0,   105,   954,     0,   211,   169,     0,   955,
     212,   956,   957,   958,   959,   960,   961,   962,    33,     0,
      35,     0,     0,   129,   214,     0,     0,   170,   129,   956,
     957,   958,   959,   960,   961,   170,   170,   170,     0,   169,
       0,   169,   170,   207,     0,     0,     0,     0,   170,     0,
       0,     0,     0,   963,   964,     0,   965,     0,   167,   169,
     586,     0,     0,     0,     0,    11,     0,     0,     0,   208,
       0,   181,   181,   586,   586,   586,     0,     0,     0,   966,
       0,     0,     0,   171,   171,     0,     0,   183,   143,    33,
       0,    75,     0,    77,     0,    78,    79,   169,    80,    81,
      82,     0,     0,     0,     0,   181,     0,   169,   169,     0,
       0,     0,     0,     0,   182,     0,     0,   168,     0,     0,
     408,   181,    92,   954,     0,     0,     0,     0,   955,     0,
     956,   957,   958,   959,   960,   961,   962,     0,   181,     0,
       0,     0,   209,     0,   586,     0,     0,   181,     0,   170,
      31,    32,     0,     0,     0,     0,   170,     0,     0,   143,
      37,     0,    75,   181,   210,     0,    78,    79,     0,    80,
      81,    82,   963,   964,     0,   965,     0,     0,     0,     0,
       0,     0,   214,   214,   211,     0,     0,   214,   212,     0,
      11,     0,     0,     0,     0,     0,     0,     0,   976,   590,
       0,     0,     0,     0,   169,     0,    66,    67,    68,    69,
      70,     0,   590,     0,     0,     0,   181,   583,   181,     0,
       0,     0,    11,    73,    74,     0,     0,   171,     0,     0,
       0,   586,     0,   171,     0,     0,   586,    84,     0,   171,
       0,     0,     0,   586,     0,     0,   608,   608,   954,     0,
      88,   214,     0,   955,   214,   956,   957,   958,   959,   960,
     961,   962,     0,     0,     0,   170,     0,     0,     0,     0,
       0,   169,     0,     0,     0,     0,     0,     0,   181,     0,
     954,     0,     0,     0,     0,   955,   171,   956,   957,   958,
     959,   960,   961,   962,   171,   171,   171,   963,   964,     0,
     965,   171,     0,     0,     0,     0,     0,   171,     0,     0,
       0,     0,   169,   170,     0,   591,     0,     0,     0,     0,
       0,     0,     0,  1068,   169,    11,   586,     0,   591,   963,
     964,     0,   965,     0,     0,     0,     0,   181,     0,     0,
     586,     0,   586,     0,     0,   170,     0,   170,     0,     0,
       0,     0,     0,   181,     0,  1069,     0,     0,     0,   608,
     181,     0,     0,   169,     0,   170,   590,     0,     0,   214,
       0,     0,     0,   183,   588,     0,     0,     0,     0,   590,
     590,   590,     0,   954,     0,     0,     0,   588,   955,     0,
     956,   957,   958,   959,   960,   961,   962,     0,     0,     0,
       0,     0,     0,   170,     0,    11,     0,     0,   171,     0,
       0,   756,     0,   170,   170,   171,     0,     0,     0,     0,
       0,     0,     0,     0,   214,     0,     0,   756,     0,     0,
     181,     0,   963,   964,   586,   965,     0,     0,   586,     0,
     586,     0,     0,     0,     0,   608,     0,     0,   181,   181,
     590,     0,     0,   786,     0,     0,     0,   181,  1070,     0,
       0,     0,     0,   954,   181,     0,     0,     0,   955,   182,
     956,   957,   958,   959,   960,   961,   962,     0,   181,     0,
       0,   181,   591,     0,     0,     0,     0,     0,     0,   680,
     681,     0,     0,     0,     0,   591,   591,   591,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   682,     0,
     170,   586,   963,   964,     0,   965,   683,   684,    33,     0,
       0,     0,     0,     0,   171,     0,   685,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   590,  1199,     0,
       0,   588,   590,   181,     0,     0,     0,     0,     0,   590,
       0,     0,   214,   214,   588,   588,   588,     0,     0,     0,
     586,     0,     0,     0,     0,     0,   591,     0,     0,     0,
       0,   686,   171,     0,     0,     0,     0,   170,     0,   586,
     586,     0,     0,   687,   586,   181,     0,     0,     0,   181,
       0,     0,    27,    28,     0,    78,    79,     0,    80,    81,
      82,    33,     0,    35,   171,     0,   171,     0,     0,     0,
       0,     0,     0,   688,     0,     0,     0,     0,   170,   214,
       0,     0,     0,     0,   171,   588,     0,     0,     0,     0,
     170,     0,   590,     0,     0,     0,     0,     0,     0,     0,
       0,   167,     0,     0,     0,     0,   590,     0,   590,     0,
       0,     0,     0,   591,     0,     0,     0,     0,   591,   948,
       0,     0,   171,     0,     0,   591,   953,     0,     0,   170,
       0,   143,   171,   171,    75,     0,    77,     0,    78,    79,
       0,    80,    81,    82,     0,     0,     0,     0,     0,   586,
      85,     0,     0,     0,     0,     0,     0,   214,     0,   214,
     356,     0,     0,     0,     0,    92,   181,     0,     0,     0,
     586,     0,   588,     0,     0,     0,     0,   588,     0,     0,
       0,   586,     0,     0,   588,     0,   586,     0,   183,     0,
       0,     0,     0,     0,   586,     0,     0,     0,     0,     0,
     590,     0,     0,     0,   590,     0,   590,     0,   591,     0,
       0,     0,     0,     0,     0,  1028,     0,     0,     0,   214,
       0,     0,   591,  1036,   591,     0,     0,     0,     0,   171,
       0,   586,    33,     0,    35,     0,     0,     0,     0,     0,
       0,     0,     0,   586,   324,   325,   326,   327,   328,   329,
     330,   331,   332,   333,   334,   335,     0,     0,     0,     0,
       0,     0,     0,   181,     0,     0,     0,   588,   181,     0,
       0,     0,   167,     0,     0,     0,     0,   590,   214,     0,
       0,   588,     0,   588,   423,     0,     0,     0,     0,     0,
     336,   337,    33,     0,    35,     0,   171,     0,     0,     0,
       0,     0,   143,     0,     0,    75,     0,    77,     0,    78,
      79,     0,    80,    81,    82,     0,   591,     0,     0,     0,
     591,     0,   591,     0,     0,     0,   590,     0,     0,   579,
     580,   168,   167,     0,     0,     0,    92,   171,     0,     0,
       0,     0,     0,     0,   759,   590,   590,     0,   581,   171,
     590,  1211,   338,     0,     0,  1036,    31,    32,    33,     0,
       0,     0,   143,     0,     0,    75,    37,    77,     0,    78,
      79,   214,    80,    81,    82,   588,     0,     0,     0,   588,
       0,   588,     0,     0,     0,     0,     0,     0,   171,   214,
       0,   168,     0,   591,     0,     0,    92,     0,     0,     0,
       0,     0,     0,     0,     0,   214,     0,     0,    33,     0,
      35,   582,    66,    67,    68,    69,    70,     0,     0,   214,
       0,     0,   214,   583,     0,     0,     0,     0,   143,    73,
      74,    75,     0,   584,     0,    78,    79,     0,    80,    81,
      82,     0,   591,    84,     0,     0,     0,     0,   167,     0,
       0,     0,   588,   585,     0,   590,    88,     0,     0,     0,
       0,   591,   591,     0,     0,     0,   591,     0,     0,     0,
    1214,     0,     0,     0,     0,     0,   590,     0,   143,     0,
       0,    75,     0,    77,   214,    78,    79,   590,    80,    81,
      82,     0,   590,     0,     0,     0,     0,     0,     0,     0,
     590,   588,     0,     0,     0,     0,     0,   168,     0,     0,
       0,     0,    92,     0,     0,     0,     0,     0,     0,     0,
     588,   588,     0,     0,     0,   588,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   590,     0,     0,
       0,     0,     0,   146,   148,     0,   150,   151,   152,   590,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,     0,     0,   175,   178,     0,     0,     0,  1028,
       0,   591,     0,     0,  1319,     0,   195,     0,     0,     0,
       0,     0,     0,   203,     0,   206,     0,     0,   219,     0,
     221,     0,   591,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   591,     0,     0,     0,     0,   591,     0,
       0,     0,     0,     0,   256,     0,   591,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   263,     0,     0,
     588,  1287,   351,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,     0,     0,     0,  1251,     0,     0,
       0,   588,     0,   591,     0,     0,     0,     0,     0,     0,
       0,     0,   588,     0,     0,   591,     0,   588,     0,     0,
     289,   290,   291,     0,     0,   588,     0,     0,   336,   337,
       0,     0,     0,     0,     0,     0,   292,   352,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,    33,
     314,    35,   588,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   588,     0,     0,   376,     0,     0,
     376,     0,   289,   290,   291,     0,   195,   385,     0,     0,
     338,     0,     0,     0,     0,     0,     0,     0,   292,   179,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,     0,   314,     0,     0,     0,     0,     0,     0,   143,
       0,   175,    75,   361,    77,   432,    78,    79,     0,    80,
      81,    82,     0,  -682,  -682,  -682,  -682,   306,   307,   308,
     309,   310,   311,   312,   313,   459,   314,     0,   180,     0,
       0,     0,     0,    92,     0,     0,   468,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   473,   474,   475,
     477,   478,   479,   480,   481,   482,   483,   484,   485,   486,
     487,   488,   489,   490,   491,   492,   493,   494,   495,   496,
     497,   498,     0,     0,   500,   500,   503,     0,     0,     0,
       0,     0,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,     0,     0,     0,     0,     0,   500,
     532,     0,   468,   500,   535,     0,     0,     0,     0,   516,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   546,
     872,   548,     0,     0,     0,     0,     0,   468,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   559,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   598,     0,     0,     0,
     513,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   655,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     141,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,   142,    65,    66,    67,    68,    69,    70,     0,
       0,     0,   256,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,   514,    77,   671,    78,    79,     0,
      80,    81,    82,     0,     0,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,   702,     0,     0,    92,    93,     0,    94,    95,   289,
     290,   291,   195,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   292,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,     0,   314,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     763,   314,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   770,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   778,     0,     0,     0,
       0,     0,     0,     0,   788,     0,     0,   789,     0,   790,
       0,     0,   468,     0,     0,     0,     0,     0,     0,     0,
       0,   468,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,   819,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     174,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,   623,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,   871,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   884,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   468,
     141,     0,     0,    59,    60,     0,     0,   468,     0,   871,
       0,     0,   142,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,     0,    77,   195,    78,    79,     0,
      80,    81,    82,     0,   929,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,     0,     0,     0,    92,    93,     0,    94,    95,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,   978,     0,     0,     0,   979,
       0,   980,     0,     0,   468,     0,     0,     0,     0,     0,
       0,     0,     0,   992,     0,     0,     0,     0,     0,     0,
       0,   468,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,     0,     0,     0,
      55,    56,    57,     0,    58,    59,    60,    61,    62,    63,
     468,     0,     0,     0,    64,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,    83,     0,    84,     0,     0,
       0,    85,     0,     0,     0,     0,     0,    86,    87,     0,
      88,    89,     0,    90,    91,   764,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,   468,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,     0,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,    49,    50,    51,
       0,    52,    53,    54,     0,     0,     0,    55,    56,    57,
       0,    58,    59,    60,    61,    62,    63,     0,     0,     0,
       0,    64,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,    83,     0,    84,     0,     0,     0,    85,     0,
       0,     0,     0,     0,    86,    87,     0,    88,    89,     0,
      90,    91,   873,    92,    93,   291,    94,    95,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,   292,
       0,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,     0,   314,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,    49,    50,    51,     0,    52,    53,
      54,     0,     0,     0,    55,    56,    57,     0,    58,    59,
      60,    61,    62,    63,     0,     0,     0,     0,    64,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,    83,
       0,    84,     0,     0,     0,    85,     0,     0,     0,     0,
       0,    86,    87,     0,    88,    89,     0,    90,    91,     0,
      92,    93,     0,    94,    95,     5,     6,     7,     8,     9,
       0,     0,     0,   292,    10,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314,     0,     0,
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
       0,    88,    89,     0,    90,    91,   453,    92,    93,     0,
      94,    95,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,     0,   314,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,     0,     0,     0,    55,    56,
      57,     0,    58,    59,    60,     0,    62,    63,     0,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,    83,     0,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,    90,    91,   594,    92,    93,     0,    94,    95,     5,
       6,     7,     8,     9,     0,     0,     0,     0,    10,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,     0,   314,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,   827,
      41,     0,    42,     0,    43,     0,     0,    44,     0,     0,
       0,    45,    46,    47,    48,     0,    50,    51,     0,    52,
       0,    54,     0,     0,     0,    55,    56,    57,     0,    58,
      59,    60,     0,    62,    63,     0,     0,     0,     0,    64,
      65,    66,    67,    68,    69,    70,     0,     0,     0,     0,
       0,     0,    71,     0,     0,     0,     0,   143,    73,    74,
      75,    76,    77,     0,    78,    79,     0,    80,    81,    82,
      83,     0,    84,     0,     0,     0,    85,     0,     0,     0,
       0,     0,    86,     0,     0,    88,    89,     0,    90,    91,
       0,    92,    93,     0,    94,    95,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,     0,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    11,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,     0,
       0,     0,    31,    32,    33,    34,    35,     0,    36,     0,
       0,     0,    37,    38,    39,    40,     0,    41,     0,    42,
       0,    43,   926,     0,    44,     0,     0,     0,    45,    46,
      47,    48,     0,    50,    51,     0,    52,     0,    54,     0,
       0,     0,    55,    56,    57,     0,    58,    59,    60,     0,
      62,    63,     0,     0,     0,     0,    64,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   143,    73,    74,    75,    76,    77,
       0,    78,    79,     0,    80,    81,    82,    83,     0,    84,
       0,     0,     0,    85,     0,     0,     0,     0,     0,    86,
       0,     0,    88,    89,     0,    90,    91,     0,    92,    93,
       0,    94,    95,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,     0,
     314,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   143,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,    83,     0,    84,     0,     0,     0,
      85,     0,     0,     0,     0,     0,    86,     0,     0,    88,
      89,     0,    90,    91,  1088,    92,    93,     0,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
    -682,  -682,  -682,  -682,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,  1246,    43,     0,     0,    44,     0,
       0,     0,    45,    46,    47,    48,     0,    50,    51,     0,
      52,     0,    54,     0,     0,     0,    55,    56,    57,     0,
      58,    59,    60,     0,    62,    63,     0,     0,     0,     0,
      64,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,   143,    73,
      74,    75,    76,    77,     0,    78,    79,     0,    80,    81,
      82,    83,     0,    84,     0,     0,     0,    85,     0,     0,
       0,     0,     0,    86,     0,     0,    88,    89,     0,    90,
      91,     0,    92,    93,     0,    94,    95,     5,     6,     7,
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
      86,     0,     0,    88,    89,     0,    90,    91,  1276,    92,
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
      88,    89,     0,    90,    91,  1279,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,  1281,    42,     0,    43,     0,     0,    44,
       0,     0,     0,    45,    46,    47,    48,     0,    50,    51,
       0,    52,     0,    54,     0,     0,     0,    55,    56,    57,
       0,    58,    59,    60,     0,    62,    63,     0,     0,     0,
       0,    64,    65,    66,    67,    68,    69,    70,     0,     0,
       0,     0,     0,     0,    71,     0,     0,     0,     0,   143,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,    83,     0,    84,     0,     0,     0,    85,     0,
       0,     0,     0,     0,    86,     0,     0,    88,    89,     0,
      90,    91,     0,    92,    93,     0,    94,    95,     5,     6,
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
       0,    86,     0,     0,    88,    89,     0,    90,    91,  1283,
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
       0,    88,    89,     0,    90,    91,  1284,    92,    93,     0,
      94,    95,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,     0,     0,     0,    55,    56,
      57,     0,    58,    59,    60,     0,    62,    63,     0,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,    83,     0,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,    90,    91,  1296,    92,    93,     0,    94,    95,     5,
       6,     7,     8,     9,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    71,     0,     0,     0,     0,   143,    73,    74,
      75,    76,    77,     0,    78,    79,     0,    80,    81,    82,
      83,     0,    84,     0,     0,     0,    85,     0,     0,     0,
       0,     0,    86,     0,     0,    88,    89,     0,    90,    91,
    1329,    92,    93,     0,    94,    95,     5,     6,     7,     8,
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
       0,     0,    88,    89,     0,    90,    91,  1332,    92,    93,
       0,    94,    95,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,   143,    73,    74,    75,    76,    77,     0,    78,    79,
       0,    80,    81,    82,    83,     0,    84,     0,     0,     0,
      85,     0,     0,     0,     0,     0,    86,     0,     0,    88,
      89,     0,    90,    91,     0,    92,    93,     0,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   377,     0,     0,     0,     0,     0,     0,
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
     561,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   715,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,     0,     0,     0,
       0,     0,    57,     0,    58,    59,    60,     0,     0,     0,
       0,     0,     0,     0,    64,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   143,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,     0,     0,    84,     0,     0,
       0,    85,     0,     0,     0,     0,     0,    86,     0,     0,
      88,    89,     0,    90,    91,     0,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1079,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,    71,     0,     0,     0,     0,   143,
      73,    74,    75,    76,    77,     0,    78,    79,     0,    80,
      81,    82,     0,     0,    84,     0,     0,     0,    85,     0,
       0,     0,     0,     0,    86,     0,     0,    88,    89,     0,
      90,    91,     0,    92,    93,     0,    94,    95,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1202,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,     0,     0,     0,     0,     0,    57,     0,    58,    59,
      60,     0,     0,     0,     0,     0,     0,     0,    64,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   143,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,     0,     0,    85,     0,     0,     0,     0,
       0,    86,     0,     0,    88,    89,     0,    90,    91,     0,
      92,    93,     0,    94,    95,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,   143,    73,    74,    75,    76,    77,     0,
      78,    79,     0,    80,    81,    82,     0,     0,    84,     0,
       0,     0,    85,     0,     0,     0,     0,     0,    86,     0,
       0,    88,    89,     0,    90,    91,     0,    92,    93,     0,
      94,    95,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      80,    81,    82,     0,     0,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,   202,   290,   291,    92,    93,     0,    94,    95,     5,
       6,     7,     8,     9,     0,     0,     0,   292,    10,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
       0,   314,     0,     0,     0,     0,     0,     0,    12,    13,
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
       0,     0,    86,     0,     0,    88,    89,     0,   205,     0,
       0,    92,    93,     0,    94,    95,     5,     6,     7,     8,
       9,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,    88,    89,     0,   218,     0,     0,    92,    93,
       0,    94,    95,     5,     6,     7,     8,     9,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,     0,     0,     0,     0,    31,
      32,    33,    34,    35,     0,     0,     0,     0,     0,    37,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   255,     0,     0,    48,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   141,     0,     0,    59,    60,     0,     0,     0,     0,
       0,     0,     0,   142,    65,    66,    67,    68,    69,    70,
       0,     0,     0,     0,     0,     0,    71,     0,     0,     0,
       0,   143,    73,    74,    75,     0,    77,     0,    78,    79,
       0,    80,    81,    82,     0,     0,    84,     0,     0,     0,
      85,     0,     0,     0,     0,     0,    86,     0,     0,    88,
      89,     0,   289,   290,   291,    92,    93,     0,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,   292,    10,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,     0,   314,     0,     0,     0,     0,     0,     0,    12,
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
      82,     0,     0,    84,     0,   315,     0,    85,     0,     0,
       0,     0,     0,    86,     0,     0,    88,    89,   375,     0,
       0,     0,    92,    93,     0,    94,    95,     5,     6,     7,
       8,     9,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   465,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   476,     0,     0,
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
      88,    89,     0,     0,     0,     0,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   513,
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
       0,     0,     0,    92,    93,     0,    94,    95,     5,     6,
       7,     8,     9,     0,     0,     0,     0,    10,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   545,     0,     0,     0,
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
       0,    86,     0,     0,    88,    89,     0,     0,     0,     0,
      92,    93,     0,    94,    95,     5,     6,     7,     8,     9,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   547,     0,     0,     0,     0,     0,     0,
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
       0,     0,    85,     0,     0,     0,     0,     0,    86,     0,
       0,    88,    89,     0,     0,     0,     0,    92,    93,     0,
      94,    95,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     777,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
      80,    81,    82,     0,     0,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,     0,     0,     0,    92,    93,     0,    94,    95,     5,
       6,     7,     8,     9,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   818,     0,     0,
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
       0,     0,    86,     0,     0,    88,    89,     0,   289,   290,
     291,    92,    93,     0,    94,    95,     5,     6,     7,     8,
       9,     0,     0,     0,   292,    10,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,     0,   314,     0,
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
       0,     0,     0,     0,   143,    73,    74,    75,   514,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,   389,     0,    85,     0,     0,     0,     0,     0,    86,
       0,     0,    88,    89,     0,   289,   290,   291,    92,    93,
       0,    94,    95,     5,     6,     7,     8,     9,     0,     0,
       0,   292,    10,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,     0,   314,     0,     0,     0,     0,
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
       0,    80,    81,    82,     0,     0,    84,     0,   391,     0,
      85,     0,     0,     0,     0,     0,    86,     0,     0,    88,
      89,     0,   289,   290,   291,    92,    93,     0,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,   292,    10,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,     0,   314,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,   431,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   141,     0,
       0,    59,    60,     0,     0,     0,     0,     0,     0,     0,
     142,    65,    66,    67,    68,    69,    70,     0,  1097,  1098,
    1099,  1100,  1101,    71,  1102,  1103,  1104,  1105,   143,    73,
      74,    75,     0,    77,     0,    78,    79,     0,    80,    81,
      82,     0,     0,    84,     0,   401,     0,    85,     0,     0,
       0,     0,     0,    86,     0,     0,    88,    89,     0,     0,
       0,     0,    92,    93,  1106,    94,    95,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1107,  1108,  1109,
    1110,  1111,  1112,  1113,     0,     0,    33,     0,     0,     0,
       0,     0,     0,     0,     0,  1114,  1115,  1116,  1117,  1118,
    1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,  1128,
    1129,  1130,  1131,  1132,  1133,  1134,  1135,  1136,  1137,  1138,
    1139,  1140,  1141,  1142,  1143,  1144,  1145,  1146,  1147,  1148,
    1149,  1150,  1151,  1152,  1153,  1154,     0,     0,  1155,  1156,
    1157,  1158,  1159,  1160,  1161,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1162,  1163,  1164,     0,
    1165,     0,     0,    78,    79,     0,    80,    81,    82,  1166,
       0,  1167,     0,     0,  1168,   289,   290,   291,     0,     0,
       0,     0,  1169,     0,  1170,     0,     0,     0,     0,     0,
       0,   292,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,     0,   314,   289,   290,   291,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   292,     0,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,     0,   314,   289,   290,   291,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   292,     0,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314,   289,   290,
     291,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   292,     0,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,     0,   314,     0,
     643,     0,   289,   290,   291,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   292,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   675,   314,   289,   290,   291,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   292,
       0,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   678,   314,   289,   290,   291,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     292,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   747,   314,   289,   290,   291,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   292,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,     0,   314,     0,   798,     0,   289,
     290,   291,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   292,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   820,   314,
     289,   290,   291,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   292,   930,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   981,
     314,   289,   290,   291,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   292,     0,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
    1056,   314,   289,   290,   291,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   292,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,     0,   314,     0,  1057,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   289,   290,   291,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   292,   931,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,     0,   314,   289,   290,   291,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   292,   403,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314,     0,   289,
     290,   291,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   537,   292,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,     0,   314,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   289,   290,   291,     0,     0,     0,     0,     0,     0,
     659,     0,     0,     0,     0,     0,     0,   292,   556,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
       0,   314,   289,   290,   291,     0,     0,     0,     0,     0,
       0,     0,   697,     0,     0,     0,     0,     0,   292,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,     0,   314,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   289,   290,
     291,     0,     0,     0,   895,     0,     0,     0,     0,     0,
       0,     0,     0,   565,   292,   656,   293,   294,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,     0,   314,   289,
     290,   291,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   292,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,     0,   314
};

static const yytype_int16 yycheck[] =
{
       4,    83,     4,     4,   279,    87,   145,    30,     4,   220,
       4,   782,   349,   181,   355,   748,   324,    40,   950,   651,
     314,    44,   130,   635,   973,   137,   973,   109,    89,   276,
     767,   399,   538,   970,   187,    53,     4,   344,   564,   752,
     438,     8,    46,     8,   412,    49,     8,   570,     8,    41,
       8,     8,   785,     8,    72,     8,     8,    75,   990,    63,
       8,     4,   369,   187,     8,    73,     8,     8,     8,    46,
      26,    90,     8,    61,    28,    61,     8,   121,    41,    83,
     188,    24,    25,    87,     8,    29,     8,   654,     8,    57,
     102,     8,     8,    73,   233,     8,    26,    78,     8,   666,
       8,   121,     0,   207,   208,   109,   447,   168,   212,    93,
      94,    61,   147,   144,   162,  1052,   748,   170,    98,   180,
     170,    46,   170,   170,    98,   144,    61,   171,   740,   137,
     138,   139,    61,    61,    61,   109,   110,   170,   173,  1071,
      61,   172,   173,   117,   118,   170,    89,   159,    66,    67,
     154,   171,   865,   785,   140,    61,   137,   894,    61,   127,
      61,   898,   899,   170,   505,   157,   377,   900,    61,   146,
     173,   121,   276,   141,    61,    61,    61,    61,   117,   171,
     154,    73,   170,   322,   161,   173,   121,   173,    61,   193,
     174,    61,   196,   121,   157,   718,   200,   720,   566,   193,
     282,   813,   196,   163,   222,   172,   173,   319,   171,   174,
     172,   579,   580,   217,   172,   172,   172,   172,    73,   172,
     172,   146,   163,   173,   172,   168,   338,   171,   626,   796,
     172,   174,   172,   251,   575,   867,   172,   180,   951,   321,
     172,   323,   874,   173,   173,   173,   173,   359,   172,   171,
     193,   171,   173,   196,   171,   171,   368,   889,   171,   371,
     997,   171,   266,   171,   170,     8,    73,   173,   900,    78,
     173,   275,   173,    93,    94,   279,   613,    73,   282,    90,
     173,   170,   650,   175,   227,   121,   173,   173,   173,   173,
     394,   117,   235,   236,   237,   356,   170,   450,   266,   242,
     173,    25,   828,   173,    31,   248,    73,   170,   814,   399,
      31,    78,    66,    67,    31,   319,   320,   321,    42,   323,
     175,    45,   412,   266,   121,   170,   450,    31,   137,   941,
     577,   421,   170,   144,   338,   171,   170,  1286,   506,  1286,
      13,   146,   138,   139,    73,   449,    73,    90,   170,    78,
     561,   692,    73,    26,   174,   359,    73,   664,   173,   991,
     314,   529,   994,   995,  1301,   319,   673,   371,   175,    73,
      43,   138,   139,    26,   742,   669,   544,   174,   382,   144,
    1312,   749,   140,   317,   338,   170,  1318,   405,   382,    73,
     543,    31,    73,  1005,    78,   736,   549,    78,   402,   172,
     906,   172,   908,   407,   162,   359,   349,   341,    31,   138,
     139,   345,   179,   356,   368,   173,   172,   371,   586,   543,
      61,    73,    61,   427,   428,   549,    78,   172,   173,   511,
      73,  1221,  1222,    71,   402,    78,    92,    93,    94,   382,
    1072,   549,   106,   107,   108,   109,   110,   111,    71,   178,
      73,   173,    25,   761,   138,   139,   137,   138,   139,   402,
      61,    71,   140,    73,   832,   170,  1237,   170,   809,    42,
    1217,  1218,    45,   577,   578,    61,   566,    98,   846,   170,
     848,    92,    93,    94,   791,   137,   138,   139,   111,   579,
     580,   581,   799,    73,   144,   138,   139,  1003,    78,    93,
      94,    95,    40,    71,   715,    73,   144,   511,   172,    49,
     148,   149,   853,   151,   152,   153,   162,   858,   141,   121,
     147,   144,   465,   146,   140,   148,   149,   639,   151,   152,
     153,     8,   869,   160,   538,   173,   540,   560,   148,   149,
     644,   151,   152,   153,   177,   140,   173,   170,   140,   140,
     170,   121,   175,   557,   172,   162,   170,  1290,   138,   139,
     650,    79,    71,   567,   568,   175,    13,    13,  1074,    13,
     513,  1304,   940,   914,   568,   171,   944,   162,   946,   886,
     148,   149,   172,   151,   152,   153,   140,    71,   171,   557,
      13,   595,   171,    45,    46,    47,   903,    49,   176,   170,
       8,   170,   545,   170,   547,   173,   774,   175,   162,   171,
      71,   171,    73,    82,   557,     8,   170,   172,   722,   173,
     724,   625,   565,   625,   625,   568,    13,   170,    71,   625,
      73,   625,     8,   170,    79,   639,    73,   146,   172,   148,
     149,   150,   151,   152,   153,   119,    71,   737,   170,  1017,
     171,    61,   742,   120,   161,     8,   122,   625,  1290,   749,
     603,   170,   752,   753,   148,   149,    71,   151,   152,   153,
     613,   614,  1304,   171,   171,   982,   680,   681,   682,   954,
     784,   704,   625,    61,     8,   639,   170,   148,   149,   177,
     151,   152,   153,    13,   177,  1201,   174,   119,  1066,     8,
     170,   705,   170,   122,   708,   148,   149,   918,   151,   152,
     153,   705,   173,    71,   175,   669,   141,  1085,  1086,   144,
     177,   932,  1090,   148,   149,     8,   151,   152,   153,   171,
     173,   171,   175,   172,   172,   739,   177,   741,   949,   843,
     708,   146,   832,   148,   149,   171,   151,   152,   153,   174,
     170,  1058,   140,   170,   140,   173,   846,   115,   848,   102,
     172,   765,   705,   765,   765,   708,    13,   710,   163,   765,
     173,   765,    13,   177,  1049,   865,   173,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    13,
     148,   149,   172,   151,   152,   153,  1007,   765,   821,   170,
     170,   170,    13,  1014,   170,   170,   810,     8,   171,   122,
     814,   172,   170,   172,    13,   245,     8,   171,    71,   249,
     824,   171,   765,    59,    60,   122,   177,  1195,     8,   833,
     824,   137,   936,   170,   777,    69,    70,    71,   268,     8,
     270,   271,   272,   273,   159,   170,   122,    25,  1216,   170,
     940,     8,   171,   170,   944,   173,   946,   171,   170,  1227,
     122,   951,   173,   173,  1232,   833,   970,    71,  1079,   171,
     177,     8,  1240,    51,    71,   818,    73,    74,    24,    25,
     984,   824,    28,   987,    26,   121,   137,   830,   141,    68,
     833,   144,   172,    71,   171,   148,   149,   172,   151,   152,
     153,    26,   906,    49,   908,   163,   171,  1046,   122,  1277,
       8,   171,   122,   166,   148,   149,     8,   151,   152,   153,
      98,  1289,   174,   173,     8,   171,   869,  1017,   106,   107,
     108,   109,   110,   111,   174,   171,    26,   141,   170,   122,
     144,   171,   146,    71,   148,   149,   124,   151,   152,   153,
     954,   148,   149,   171,   151,   152,   153,   173,   171,   171,
       8,   965,   171,   141,    26,   172,   144,   172,   146,   171,
     148,   149,   172,   151,   152,   153,  1066,   122,    73,   150,
     146,  1192,   170,   104,   171,    73,  1197,    13,   166,   171,
     171,  1202,   170,   122,   171,  1085,  1086,  1208,  1209,  1003,
    1090,   969,   122,   122,  1008,    13,   171,  1011,    73,   173,
     172,  1015,   171,   141,   170,   173,   144,  1011,    13,  1023,
     148,   149,   168,   151,   152,   153,   173,    13,   174,   122,
     173,  1035,  1243,    25,   180,    71,    13,  1041,   173,   171,
    1008,    73,    13,    71,    51,  1049,   173,  1015,    73,  1188,
      73,     8,   170,    13,   140,  1023,   172,    90,    13,    51,
      90,   207,   208,    29,   153,  1008,   212,   170,  1011,   171,
    1074,   172,  1015,  1077,  1078,    73,   155,   171,  1082,    71,
    1023,   227,    73,  1051,  1078,   170,   173,   320,   171,   235,
     236,   237,   172,   171,  1298,   323,   242,   405,   321,   665,
     127,   627,   248,  1185,   668,  1195,    98,  1309,   144,  1077,
     830,  1322,   148,   149,  1082,   151,   152,   153,   410,  1330,
     148,   149,  1095,   151,   152,   153,  1216,   929,  1174,  1324,
     276,  1186,   124,   279,  1077,  1078,   965,  1227,    63,  1082,
      38,    89,  1232,  1024,   768,   681,    71,   810,    73,   141,
    1240,  1233,   144,   797,   146,   737,   148,   149,   889,   151,
     152,   153,   694,   236,   725,   753,    25,   243,   314,   987,
      -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,   170,    71,
      -1,  1185,    -1,    -1,    24,    25,   111,  1277,    28,    -1,
      -1,    -1,    51,    -1,    -1,    -1,    -1,  1201,    -1,  1289,
      -1,  1205,    -1,   349,    -1,    -1,    -1,    -1,  1212,    -1,
     356,    -1,    71,  1217,  1218,    -1,   141,  1221,  1222,   144,
     112,   146,    -1,   148,   149,    25,   151,   152,   153,  1233,
      -1,    -1,   124,    -1,  1238,    -1,    -1,  1205,  1242,    -1,
      -1,  1245,    -1,  1247,  1248,   170,    -1,    -1,   394,   141,
     175,    51,   144,   399,   146,    -1,   148,   149,    -1,   151,
     152,   153,  1205,    -1,    -1,   124,   412,    -1,    -1,    -1,
    1238,    71,    -1,    -1,  1242,   421,  1280,  1245,    -1,  1247,
    1248,  1285,   141,    -1,    -1,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,  1238,  1300,    -1,   157,  1242,
      -1,    -1,  1245,   449,  1247,  1248,    41,   166,    -1,    -1,
      -1,   170,  1280,    42,    43,    44,    45,    46,    47,   465,
      49,    -1,  1326,    -1,   124,    -1,    -1,  1331,   168,    -1,
      -1,    -1,    -1,    -1,   174,    -1,    -1,  1280,    -1,    -1,
     180,   141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,   157,  1326,    -1,
      -1,    -1,    -1,  1331,    99,    -1,   166,   513,    -1,   104,
     170,   106,   107,   108,   109,   110,   111,   112,    71,    -1,
      73,    -1,    -1,  1326,    49,    -1,    -1,   227,  1331,   106,
     107,   108,   109,   110,   111,   235,   236,   237,    -1,   545,
      -1,   547,   242,    25,    -1,    -1,    -1,    -1,   248,    -1,
      -1,    -1,    -1,   148,   149,    -1,   151,    -1,   111,   565,
     566,    -1,    -1,    -1,    -1,    41,    -1,    -1,    -1,    51,
      -1,   577,   578,   579,   580,   581,    -1,    -1,    -1,   174,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,   141,    71,
      -1,   144,    -1,   146,    -1,   148,   149,   603,   151,   152,
     153,    -1,    -1,    -1,    -1,   611,    -1,   613,   614,    -1,
      -1,    -1,    -1,    -1,   314,    -1,    -1,   170,    -1,    -1,
     173,   627,   175,    99,    -1,    -1,    -1,    -1,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,   644,    -1,
      -1,    -1,   124,    -1,   650,    -1,    -1,   653,    -1,   349,
      69,    70,    -1,    -1,    -1,    -1,   356,    -1,    -1,   141,
      79,    -1,   144,   669,   146,    -1,   148,   149,    -1,   151,
     152,   153,   148,   149,    -1,   151,    -1,    -1,    -1,    -1,
      -1,    -1,   207,   208,   166,    -1,    -1,   212,   170,    -1,
      41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   174,   399,
      -1,    -1,    -1,    -1,   710,    -1,   125,   126,   127,   128,
     129,    -1,   412,    -1,    -1,    -1,   722,   136,   724,    -1,
      -1,    -1,    41,   142,   143,    -1,    -1,   168,    -1,    -1,
      -1,   737,    -1,   174,    -1,    -1,   742,   156,    -1,   180,
      -1,    -1,    -1,   749,    -1,    -1,   752,   753,    99,    -1,
     169,   276,    -1,   104,   279,   106,   107,   108,   109,   110,
     111,   112,    -1,    -1,    -1,   465,    -1,    -1,    -1,    -1,
      -1,   777,    -1,    -1,    -1,    -1,    -1,    -1,   784,    -1,
      99,    -1,    -1,    -1,    -1,   104,   227,   106,   107,   108,
     109,   110,   111,   112,   235,   236,   237,   148,   149,    -1,
     151,   242,    -1,    -1,    -1,    -1,    -1,   248,    -1,    -1,
      -1,    -1,   818,   513,    -1,   399,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   174,   830,    41,   832,    -1,   412,   148,
     149,    -1,   151,    -1,    -1,    -1,    -1,   843,    -1,    -1,
     846,    -1,   848,    -1,    -1,   545,    -1,   547,    -1,    -1,
      -1,    -1,    -1,   859,    -1,   174,    -1,    -1,    -1,   865,
     866,    -1,    -1,   869,    -1,   565,   566,    -1,    -1,   394,
      -1,    -1,    -1,   314,   399,    -1,    -1,    -1,    -1,   579,
     580,   581,    -1,    99,    -1,    -1,    -1,   412,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,    -1,    -1,   603,    -1,    41,    -1,    -1,   349,    -1,
      -1,   611,    -1,   613,   614,   356,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   449,    -1,    -1,   627,    -1,    -1,
     936,    -1,   148,   149,   940,   151,    -1,    -1,   944,    -1,
     946,    -1,    -1,    -1,    -1,   951,    -1,    -1,   954,   955,
     650,    -1,    -1,   653,    -1,    -1,    -1,   963,   174,    -1,
      -1,    -1,    -1,    99,   970,    -1,    -1,    -1,   104,   669,
     106,   107,   108,   109,   110,   111,   112,    -1,   984,    -1,
      -1,   987,   566,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,   579,   580,   581,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    61,    -1,
     710,  1017,   148,   149,    -1,   151,    69,    70,    71,    -1,
      -1,    -1,    -1,    -1,   465,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   737,   174,    -1,
      -1,   566,   742,  1049,    -1,    -1,    -1,    -1,    -1,   749,
      -1,    -1,   577,   578,   579,   580,   581,    -1,    -1,    -1,
    1066,    -1,    -1,    -1,    -1,    -1,   650,    -1,    -1,    -1,
      -1,   124,   513,    -1,    -1,    -1,    -1,   777,    -1,  1085,
    1086,    -1,    -1,   136,  1090,  1091,    -1,    -1,    -1,  1095,
      -1,    -1,    62,    63,    -1,   148,   149,    -1,   151,   152,
     153,    71,    -1,    73,   545,    -1,   547,    -1,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,    -1,    -1,   818,   644,
      -1,    -1,    -1,    -1,   565,   650,    -1,    -1,    -1,    -1,
     830,    -1,   832,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,    -1,    -1,   846,    -1,   848,    -1,
      -1,    -1,    -1,   737,    -1,    -1,    -1,    -1,   742,   859,
      -1,    -1,   603,    -1,    -1,   749,   866,    -1,    -1,   869,
      -1,   141,   613,   614,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,    -1,    -1,    -1,  1195,
     160,    -1,    -1,    -1,    -1,    -1,    -1,   722,    -1,   724,
     170,    -1,    -1,    -1,    -1,   175,  1212,    -1,    -1,    -1,
    1216,    -1,   737,    -1,    -1,    -1,    -1,   742,    -1,    -1,
      -1,  1227,    -1,    -1,   749,    -1,  1232,    -1,   669,    -1,
      -1,    -1,    -1,    -1,  1240,    -1,    -1,    -1,    -1,    -1,
     940,    -1,    -1,    -1,   944,    -1,   946,    -1,   832,    -1,
      -1,    -1,    -1,    -1,    -1,   955,    -1,    -1,    -1,   784,
      -1,    -1,   846,   963,   848,    -1,    -1,    -1,    -1,   710,
      -1,  1277,    71,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1289,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1309,    -1,    -1,    -1,   832,  1314,    -1,
      -1,    -1,   111,    -1,    -1,    -1,    -1,  1017,   843,    -1,
      -1,   846,    -1,   848,   123,    -1,    -1,    -1,    -1,    -1,
      59,    60,    71,    -1,    73,    -1,   777,    -1,    -1,    -1,
      -1,    -1,   141,    -1,    -1,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,   940,    -1,    -1,    -1,
     944,    -1,   946,    -1,    -1,    -1,  1066,    -1,    -1,    42,
      43,   170,   111,    -1,    -1,    -1,   175,   818,    -1,    -1,
      -1,    -1,    -1,    -1,   123,  1085,  1086,    -1,    61,   830,
    1090,  1091,   121,    -1,    -1,  1095,    69,    70,    71,    -1,
      -1,    -1,   141,    -1,    -1,   144,    79,   146,    -1,   148,
     149,   936,   151,   152,   153,   940,    -1,    -1,    -1,   944,
      -1,   946,    -1,    -1,    -1,    -1,    -1,    -1,   869,   954,
      -1,   170,    -1,  1017,    -1,    -1,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   970,    -1,    -1,    71,    -1,
      73,   124,   125,   126,   127,   128,   129,    -1,    -1,   984,
      -1,    -1,   987,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,  1066,   156,    -1,    -1,    -1,    -1,   111,    -1,
      -1,    -1,  1017,   166,    -1,  1195,   169,    -1,    -1,    -1,
      -1,  1085,  1086,    -1,    -1,    -1,  1090,    -1,    -1,    -1,
    1094,    -1,    -1,    -1,    -1,    -1,  1216,    -1,   141,    -1,
      -1,   144,    -1,   146,  1049,   148,   149,  1227,   151,   152,
     153,    -1,  1232,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1240,  1066,    -1,    -1,    -1,    -1,    -1,   170,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1085,  1086,    -1,    -1,    -1,  1090,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1277,    -1,    -1,
      -1,    -1,    -1,     5,     6,    -1,     8,     9,    10,  1289,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    -1,    -1,    26,    27,    -1,    -1,    -1,  1309,
      -1,  1195,    -1,    -1,  1314,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    47,    -1,    -1,    50,    -1,
      52,    -1,  1216,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1227,    -1,    -1,    -1,    -1,  1232,    -1,
      -1,    -1,    -1,    -1,    76,    -1,  1240,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    -1,    -1,
    1195,  1255,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    -1,    -1,  1212,    -1,    -1,
      -1,  1216,    -1,  1277,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1227,    -1,    -1,  1289,    -1,  1232,    -1,    -1,
       9,    10,    11,    -1,    -1,  1240,    -1,    -1,    59,    60,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   149,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    71,
      49,    73,  1277,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1289,    -1,    -1,   189,    -1,    -1,
     192,    -1,     9,    10,    11,    -1,   198,   199,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   111,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,   243,   144,   122,   146,   247,   148,   149,    -1,   151,
     152,   153,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,   267,    49,    -1,   170,    -1,
      -1,    -1,    -1,   175,    -1,    -1,   278,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,    -1,    -1,   316,   317,   318,    -1,    -1,    -1,
      -1,    -1,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,    -1,    -1,    -1,    -1,    -1,   341,
     342,    -1,   344,   345,   346,    -1,    -1,    -1,    -1,   351,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   361,
     177,   363,    -1,    -1,    -1,    -1,    -1,   369,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   379,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   408,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   476,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,   514,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,   528,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   553,    -1,    -1,   175,   176,    -1,   178,   179,     9,
      10,    11,   564,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     622,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   634,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   648,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   656,    -1,    -1,   659,    -1,   661,
      -1,    -1,   664,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   673,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,   697,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,   174,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,   761,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   773,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   791,
     111,    -1,    -1,   114,   115,    -1,    -1,   799,    -1,   801,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,   828,   148,   149,    -1,
     151,   152,   153,    -1,   836,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,    -1,    -1,    -1,   175,   176,    -1,   178,   179,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,   877,    -1,    -1,    -1,   881,
      -1,   883,    -1,    -1,   886,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   895,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   903,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,   116,   117,   118,
     982,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,   176,    -1,   178,
     179,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    -1,    -1,  1058,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    99,   100,   101,
      -1,   103,   104,   105,    -1,    -1,    -1,   109,   110,   111,
      -1,   113,   114,   115,   116,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,   167,    -1,   169,   170,    -1,
     172,   173,   174,   175,   176,    11,   178,   179,     3,     4,
       5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    99,   100,   101,    -1,   103,   104,
     105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,   167,    -1,   169,   170,    -1,   172,   173,    -1,
     175,   176,    -1,   178,   179,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    25,    12,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
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
      -1,    12,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   172,   173,   174,   175,   176,    -1,   178,   179,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    83,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,
      -1,   175,   176,    -1,   178,   179,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,   172,   173,    -1,   175,   176,
      -1,   178,   179,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,
     170,    -1,   172,   173,   174,   175,   176,    -1,   178,   179,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    87,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,
     173,    -1,   175,   176,    -1,   178,   179,     3,     4,     5,
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
      82,    -1,    84,    85,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,
      -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,   148,   149,    -1,   151,
     152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,    -1,   175,   176,    -1,   178,   179,     3,     4,
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
      -1,   169,   170,    -1,   172,   173,   174,   175,   176,    -1,
     178,   179,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   172,   173,   174,   175,   176,    -1,   178,   179,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
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
     156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,
     166,    -1,    -1,   169,   170,    -1,   172,   173,    -1,   175,
     176,    -1,   178,   179,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
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
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,
     169,   170,    -1,   172,   173,    -1,   175,   176,    -1,   178,
     179,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,
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
     152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,
      -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,
     172,   173,    -1,   175,   176,    -1,   178,   179,     3,     4,
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
     105,    -1,    -1,    -1,    -1,    -1,   111,    -1,   113,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,   172,   173,    -1,
     175,   176,    -1,   178,   179,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,   169,   170,    -1,   172,   173,    -1,   175,   176,    -1,
     178,   179,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   172,    10,    11,   175,   176,    -1,   178,   179,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    25,    12,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,
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
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,   172,    -1,
      -1,   175,   176,    -1,   178,   179,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   169,   170,    -1,   172,    -1,    -1,   175,   176,
      -1,   178,   179,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    95,    -1,    -1,    98,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,    -1,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,
     170,    -1,     9,    10,    11,   175,   176,    -1,   178,   179,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    25,    12,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    42,
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
     153,    -1,    -1,   156,    -1,   172,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,   169,   170,   171,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,
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
      -1,    -1,    -1,   175,   176,    -1,   178,   179,     3,     4,
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
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,
      -1,   166,    -1,    -1,   169,   170,    -1,    -1,    -1,    -1,
     175,   176,    -1,   178,   179,     3,     4,     5,     6,     7,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,
      -1,   169,   170,    -1,    -1,    -1,    -1,   175,   176,    -1,
     178,   179,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,    -1,    -1,    -1,   175,   176,    -1,   178,   179,     3,
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
      -1,    -1,   166,    -1,    -1,   169,   170,    -1,     9,    10,
      11,   175,   176,    -1,   178,   179,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    25,    12,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
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
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,   172,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,    -1,   169,   170,    -1,     9,    10,    11,   175,   176,
      -1,   178,   179,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    25,    12,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
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
      -1,   151,   152,   153,    -1,    -1,   156,    -1,   172,    -1,
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,
     170,    -1,     9,    10,    11,   175,   176,    -1,   178,   179,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    25,    12,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,     3,     4,
       5,     6,     7,   136,     9,    10,    11,    12,   141,   142,
     143,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,   172,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,    -1,   169,   170,    -1,    -1,
      -1,    -1,   175,   176,    49,   178,   179,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,
      65,    66,    67,    68,    -1,    -1,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,   142,   143,    -1,
     145,    -1,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,   159,     9,    10,    11,    -1,    -1,
      -1,    -1,   167,    -1,   169,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,    -1,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
     174,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
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
      45,    46,    47,   174,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,   174,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   174,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,   174,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     174,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,   174,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   172,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   172,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   171,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    25,   171,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,
      11,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    90,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49
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
     280,   284,   285,   286,   287,   288,   296,   297,   298,   300,
     301,   304,   314,   315,   316,   321,   324,   342,   347,   349,
     350,   351,   352,   353,   354,   355,   356,   358,   371,   373,
     374,   111,   123,   141,   187,   209,   287,   349,   287,   170,
     287,   287,   287,   340,   341,   287,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   111,   170,   191,
     315,   316,   349,   349,    31,   287,   362,   363,   287,   111,
     170,   191,   315,   316,   317,   348,   354,   359,   360,   170,
     281,   318,   170,   281,   282,   287,   200,   281,   170,   170,
     170,   281,   172,   287,   187,   172,   287,    25,    51,   124,
     146,   166,   170,   187,   194,   375,   385,   386,   172,   287,
     173,   287,   144,   188,   189,   190,    73,   175,   248,   249,
     117,   117,    73,   209,   250,   170,   170,   170,   170,   187,
     222,   376,   170,   170,    73,    78,   137,   138,   139,   368,
     369,   144,   173,   190,   190,    95,   287,   223,   376,   146,
     170,   376,   280,   287,   288,   349,   196,   173,    78,   319,
     368,    78,   368,   368,    26,   144,   162,   377,   170,     8,
     172,    31,   208,   146,   221,   376,   172,   172,   172,     9,
      10,    11,    25,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    49,   172,    61,    61,   173,   140,
     118,   154,   209,   224,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    59,    60,   121,   344,
     345,    61,   173,   346,   170,    61,   173,   175,   355,   170,
     208,    13,   287,    40,   187,   339,   170,   280,   349,   140,
     349,   122,   177,     8,   326,   280,   349,   377,   140,   170,
     320,   121,   344,   345,   346,   171,   287,    26,   198,     8,
     172,   198,   199,   282,   283,   287,   187,   236,   202,   172,
     172,   172,   386,   386,   162,   170,    98,   378,   386,    13,
     187,   172,   196,   172,   190,     8,   172,    90,   173,   349,
       8,   172,    13,   208,     8,   172,   349,   372,   372,   349,
     171,   162,   216,   123,   349,   361,   362,    61,   121,   137,
     369,    72,   287,   349,    78,   137,   369,   190,   186,   172,
     173,   172,   219,   305,   307,    79,   291,   292,   294,    13,
     171,   171,   171,   174,   197,   198,   210,   213,   218,   287,
     176,   178,   179,   187,   378,    31,   246,   247,   287,   375,
     170,   376,   214,   287,   287,   287,    26,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   287,   317,
     287,   357,   357,   287,   364,   365,   187,   354,   355,   222,
     223,   208,   221,    31,   145,   284,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   173,   187,
     354,   357,   287,   246,   357,   287,   361,   171,   170,   338,
       8,   326,   280,   171,   187,    31,   287,    31,   287,   171,
     171,   354,   246,   173,   187,   354,   171,   196,   240,   287,
      82,    26,   198,   234,   172,    90,    13,     8,   171,    26,
     173,   237,   386,    79,   382,   383,   384,   170,     8,    42,
      43,    61,   124,   136,   146,   166,   191,   192,   194,   299,
     315,   321,   322,   323,   174,    90,   189,   187,   287,   249,
     322,   170,    73,     8,   171,   171,   171,   172,   191,   192,
     381,   119,   227,   170,     8,   171,   171,    73,    74,   187,
     370,   187,    61,   174,   174,   183,   185,   120,   226,   161,
      46,   146,   161,   309,   122,     8,   326,   171,   386,   121,
     344,   345,   346,   174,     8,   163,   349,   171,     8,   327,
      13,   289,   211,   119,   225,   287,    26,   177,   177,   122,
     174,     8,   326,   377,   170,   217,   220,   376,   215,    63,
     349,   287,   377,   170,   177,   174,   171,   177,   174,   171,
      42,    43,    61,    69,    70,    79,   124,   136,   166,   187,
     329,   331,   334,   337,   187,   349,   349,   122,   344,   345,
     346,   171,   287,   241,    66,    67,   242,   281,   196,   283,
      31,   231,   349,   322,   187,    26,   198,   235,   172,   238,
     172,   238,     8,   163,   122,     8,   326,   171,   157,   378,
     379,   386,   322,   322,   322,   325,   328,   170,    78,   140,
     170,   140,   173,   102,   205,   206,   187,   174,   290,    13,
     349,   172,     8,    90,   163,   228,   315,   173,   361,   123,
     349,    13,   177,   287,   174,   183,   228,   173,   308,    13,
     287,   291,   172,   173,   187,   354,   386,    31,   287,   322,
     157,   244,   245,   342,   343,   170,   315,   226,   287,   287,
     287,   170,   246,   227,   226,   212,   225,   317,   174,   170,
     246,    13,    69,    70,   187,   330,   330,   331,   332,   333,
     170,    78,   137,   170,     8,   326,   171,   338,    31,   287,
     174,    66,    67,   243,   281,   198,   172,    83,   172,   349,
     122,   230,    13,   196,   238,    92,    93,    94,   238,   174,
     386,   386,   382,     8,   171,   171,   122,   177,     8,   326,
     325,   187,   291,   293,   295,   187,   322,   366,   367,   170,
     159,   244,   322,   381,   192,   385,     8,   251,   171,   170,
     284,   287,   177,   174,   251,   147,   160,   173,   304,   311,
     147,   173,   310,   122,   287,   377,   170,   349,   171,     8,
     327,   386,   387,   244,   173,   122,   246,   171,   173,   173,
     170,   226,   320,   170,   246,   171,   122,   177,     8,   326,
     332,   137,   291,   335,   336,   331,   349,   281,    26,    68,
     198,   172,   283,   231,   171,   322,    89,    92,   172,   287,
      26,   172,   239,   174,   163,   157,    26,   322,   322,   171,
     122,     8,   326,   171,   122,   174,     8,   326,   315,   173,
     171,     8,   381,   315,    99,   104,   106,   107,   108,   109,
     110,   111,   112,   148,   149,   151,   174,   252,   274,   275,
     276,   277,   279,   342,   361,   174,   174,    46,   287,   287,
     287,   174,   170,   246,    26,   380,   157,   343,    31,    73,
     171,   251,   287,   171,   251,   251,   244,   173,   246,   171,
     331,   331,   171,   122,   171,     8,   326,    26,   196,   172,
     171,   203,   172,   172,   239,   196,   386,   122,   322,   291,
     322,   322,    73,   196,   380,   381,   375,   229,   315,   112,
     124,   146,   152,   261,   262,   263,   315,   150,   267,   268,
     115,   170,   187,   269,   270,   253,   209,   277,   386,     8,
     172,   275,   276,   171,   146,   306,   174,   174,   170,   246,
     171,   386,   104,   302,   387,    73,    13,   380,   174,   174,
     174,   171,   251,   171,   122,   331,   291,   196,   201,    26,
     198,   233,   196,   171,   322,   122,   122,   171,   174,   302,
      13,     8,   172,   173,   173,     8,   172,     3,     4,     5,
       6,     7,     9,    10,    11,    12,    49,    62,    63,    64,
      65,    66,    67,    68,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   123,   124,   125,   126,   127,
     128,   129,   141,   142,   143,   145,   154,   156,   159,   167,
     169,   187,   312,   313,     8,   172,   146,   150,   187,   270,
     271,   272,   172,    73,   278,   208,   254,   375,   209,   246,
     171,   170,   173,    31,    73,    13,   322,   173,   380,   174,
     331,   122,    26,   198,   232,   196,   322,   322,   173,   173,
     322,   315,   257,   264,   321,   262,    13,    26,    43,   265,
     268,     8,    29,   171,    25,    42,    45,    13,     8,   172,
     376,   278,    13,   208,   171,    31,    73,   303,   196,    73,
      13,   322,   196,   173,   331,   196,    87,   196,   196,   174,
     187,   194,   258,   259,   260,     8,   174,   322,   313,   313,
      51,   266,   271,   271,    25,    42,    45,   322,    73,   170,
     172,   322,   376,    73,     8,   327,   174,    13,   322,   174,
     196,    85,   172,   174,   174,   140,    90,   321,   153,    13,
     255,   170,    31,    73,   171,   322,   174,   172,   204,   187,
     275,   276,   322,   244,   256,    73,   102,   205,   207,   155,
     187,   172,   171,   244,   170,   229,   172,   380,   171,   315,
     172,   172,   173,   273,   380,    73,   196,   273,   171,   174,
     173,   196,   174
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
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
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
#line 1507 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1514 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1518 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1524 "hphp.y"
    { (yyval).reset();;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1537 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1542 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1543 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1554 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1555 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1559 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1560 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1561 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1563 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1567 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1568 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1569 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1572 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1573 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1574 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1576 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1580 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1581 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1583 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1584 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1585 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1588 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1589 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1590 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1593 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1595 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1599 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1600 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1602 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1603 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1604 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1605 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1607 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1608 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1609 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1613 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1614 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1615 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1619 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),1);
                                         _p->popLabelInfo();;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY); ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1693 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1702 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1706 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1707 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1716 "hphp.y"
    { (yyval).reset();;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1720 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1721 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1722 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1723 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1730 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         (yyval).setText("");;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1747 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1748 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1753 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1754 "hphp.y"
    { (yyval).reset();;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1757 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1758 "hphp.y"
    { (yyval).reset();;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1761 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1765 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1768 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
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
#line 1778 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1779 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1783 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1787 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1793 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1794 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1795 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1796 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1797 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1798 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1800 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1801 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1803 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1804 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1805 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1806 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1807 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1808 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1809 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1810 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1811 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1812 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1814 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1815 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1816 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1817 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1818 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1819 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1820 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1821 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1822 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1823 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1826 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1827 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1829 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1832 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1833 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1836 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1837 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1839 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1840 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1841 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1843 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1850 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1851 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1852 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1853 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1854 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1857 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1858 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1859 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1861 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1863 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { (yyval).reset();;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval).reset();;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { (yyval).reset();;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval).reset();;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1946 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1951 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval).reset();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1968 "hphp.y"
    { (yyval).reset();;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval).reset();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1972 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { (yyval).reset();;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1979 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1983 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 1999 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2004 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval).reset();;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2028 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2039 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2045 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2054 "hphp.y"
    { (yyval).reset();;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2058 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2065 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2067 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { user_attribute_check(_p);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2072 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { (yyval).reset();;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2083 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2094 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2095 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2101 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2112 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2115 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2116 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2117 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2129 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2130 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2131 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2161 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2176 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2180 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2184 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2191 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2200 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2201 "hphp.y"
    { (yyval).reset();;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2206 "hphp.y"
    { (yyval)++;;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2217 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2226 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2235 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2236 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2242 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { (yyval).reset();;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2247 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2248 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2250 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2255 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2257 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2262 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2267 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2268 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2269 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2270 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2275 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2283 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2293 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2295 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2313 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2321 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2334 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2360 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]);
                                         only_in_hh_syntax(_p); ;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2367 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2381 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2387 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (5)]).text()); ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2420 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    {;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2426 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyval).setText("array"); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2434 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2441 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2444 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2454 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2464 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10605 "new_hphp.tab.cpp"
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
#line 2476 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

