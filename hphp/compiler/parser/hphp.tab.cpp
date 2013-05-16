
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
#define YYLAST   11117

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  180
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  207
/* YYNRULES -- Number of rules.  */
#define YYNRULES  712
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1332

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
    1609,  1614,  1618,  1623,  1625,  1627,  1631,  1635,  1637,  1639,
    1641,  1643,  1647,  1651,  1655,  1658,  1659,  1661,  1662,  1664,
    1665,  1671,  1675,  1679,  1681,  1683,  1685,  1687,  1691,  1694,
    1696,  1698,  1700,  1702,  1704,  1707,  1710,  1715,  1719,  1724,
    1727,  1728,  1734,  1738,  1742,  1744,  1748,  1750,  1753,  1754,
    1760,  1764,  1767,  1768,  1772,  1773,  1778,  1781,  1782,  1786,
    1790,  1792,  1793,  1795,  1798,  1801,  1806,  1810,  1814,  1817,
    1822,  1825,  1830,  1832,  1834,  1836,  1838,  1840,  1843,  1848,
    1852,  1857,  1861,  1863,  1865,  1867,  1869,  1872,  1877,  1882,
    1886,  1888,  1890,  1894,  1902,  1909,  1918,  1928,  1937,  1948,
    1956,  1963,  1965,  1968,  1973,  1978,  1980,  1982,  1987,  1989,
    1990,  1992,  1995,  1997,  1999,  2002,  2007,  2011,  2015,  2016,
    2018,  2021,  2026,  2030,  2033,  2037,  2044,  2045,  2047,  2052,
    2055,  2056,  2062,  2066,  2070,  2072,  2079,  2084,  2089,  2092,
    2095,  2096,  2102,  2106,  2110,  2112,  2115,  2116,  2122,  2126,
    2130,  2132,  2135,  2138,  2140,  2143,  2145,  2150,  2154,  2158,
    2165,  2169,  2171,  2173,  2175,  2180,  2185,  2188,  2191,  2196,
    2199,  2202,  2204,  2208,  2212,  2218,  2220,  2223,  2225,  2230,
    2234,  2235,  2237,  2241,  2245,  2247,  2249,  2250,  2251,  2254,
    2258,  2260,  2265,  2271,  2275,  2279,  2283,  2287,  2289,  2292,
    2293,  2298,  2301,  2304,  2306,  2308,  2310,  2315,  2322,  2324,
    2333,  2339,  2341
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
     170,   324,   171,    -1,    61,   324,   177,    -1,   166,   170,
     294,   171,    -1,   322,    -1,   298,    -1,   194,   140,   187,
      -1,   146,   140,   187,    -1,   192,    -1,    72,    -1,   372,
      -1,   320,    -1,   178,   367,   178,    -1,   179,   367,   179,
      -1,   136,   367,   137,    -1,   327,   325,    -1,    -1,     8,
      -1,    -1,     8,    -1,    -1,   327,     8,   321,   122,   321,
      -1,   327,     8,   321,    -1,   321,   122,   321,    -1,   321,
      -1,    69,    -1,    70,    -1,    79,    -1,   136,    78,   137,
      -1,   136,   137,    -1,    69,    -1,    70,    -1,   187,    -1,
     328,    -1,   187,    -1,    42,   329,    -1,    43,   329,    -1,
     124,   170,   331,   171,    -1,    61,   331,   177,    -1,   166,
     170,   334,   171,    -1,   332,   325,    -1,    -1,   332,     8,
     330,   122,   330,    -1,   332,     8,   330,    -1,   330,   122,
     330,    -1,   330,    -1,   333,     8,   330,    -1,   330,    -1,
     335,   325,    -1,    -1,   335,     8,    79,   122,   330,    -1,
      79,   122,   330,    -1,   333,   325,    -1,    -1,   170,   336,
     171,    -1,    -1,   338,     8,   187,   337,    -1,   187,   337,
      -1,    -1,   340,   338,   325,    -1,    41,   339,    40,    -1,
     341,    -1,    -1,   344,    -1,   121,   353,    -1,   121,   187,
      -1,   121,   173,   287,   174,    -1,    61,   356,   177,    -1,
     173,   287,   174,    -1,   349,   345,    -1,   170,   280,   171,
     345,    -1,   359,   345,    -1,   170,   280,   171,   345,    -1,
     353,    -1,   313,    -1,   351,    -1,   352,    -1,   346,    -1,
     348,   343,    -1,   170,   280,   171,   343,    -1,   315,   140,
     353,    -1,   350,   170,   246,   171,    -1,   170,   348,   171,
      -1,   313,    -1,   351,    -1,   352,    -1,   346,    -1,   348,
     344,    -1,   170,   280,   171,   344,    -1,   350,   170,   246,
     171,    -1,   170,   348,   171,    -1,   353,    -1,   346,    -1,
     170,   348,   171,    -1,   348,   121,   187,   376,   170,   246,
     171,    -1,   348,   121,   353,   170,   246,   171,    -1,   348,
     121,   173,   287,   174,   170,   246,   171,    -1,   170,   280,
     171,   121,   187,   376,   170,   246,   171,    -1,   170,   280,
     171,   121,   353,   170,   246,   171,    -1,   170,   280,   171,
     121,   173,   287,   174,   170,   246,   171,    -1,   315,   140,
     187,   376,   170,   246,   171,    -1,   315,   140,   353,   170,
     246,   171,    -1,   354,    -1,   357,   354,    -1,   354,    61,
     356,   177,    -1,   354,   173,   287,   174,    -1,   355,    -1,
      73,    -1,   175,   173,   287,   174,    -1,   287,    -1,    -1,
     175,    -1,   357,   175,    -1,   353,    -1,   347,    -1,   358,
     343,    -1,   170,   280,   171,   343,    -1,   315,   140,   353,
      -1,   170,   348,   171,    -1,    -1,   347,    -1,   358,   344,
      -1,   170,   280,   171,   344,    -1,   170,   348,   171,    -1,
     360,     8,    -1,   360,     8,   348,    -1,   360,     8,   123,
     170,   360,   171,    -1,    -1,   348,    -1,   123,   170,   360,
     171,    -1,   362,   325,    -1,    -1,   362,     8,   287,   122,
     287,    -1,   362,     8,   287,    -1,   287,   122,   287,    -1,
     287,    -1,   362,     8,   287,   122,    31,   348,    -1,   362,
       8,    31,   348,    -1,   287,   122,    31,   348,    -1,    31,
     348,    -1,   364,   325,    -1,    -1,   364,     8,   287,   122,
     287,    -1,   364,     8,   287,    -1,   287,   122,   287,    -1,
     287,    -1,   366,   325,    -1,    -1,   366,     8,   321,   122,
     321,    -1,   366,     8,   321,    -1,   321,   122,   321,    -1,
     321,    -1,   367,   368,    -1,   367,    78,    -1,   368,    -1,
      78,   368,    -1,    73,    -1,    73,    61,   369,   177,    -1,
      73,   121,   187,    -1,   138,   287,   174,    -1,   138,    72,
      61,   287,   177,   174,    -1,   139,   348,   174,    -1,   187,
      -1,    74,    -1,    73,    -1,   114,   170,   371,   171,    -1,
     115,   170,   348,   171,    -1,     7,   287,    -1,     6,   287,
      -1,     5,   170,   287,   171,    -1,     4,   287,    -1,     3,
     287,    -1,   348,    -1,   371,     8,   348,    -1,   315,   140,
     187,    -1,   167,   375,    13,   385,   172,    -1,   187,    -1,
     385,   187,    -1,   187,    -1,   187,   162,   380,   163,    -1,
     162,   377,   163,    -1,    -1,   385,    -1,   377,     8,   385,
      -1,   377,     8,   157,    -1,   377,    -1,   157,    -1,    -1,
      -1,    26,   385,    -1,   192,     8,   380,    -1,   192,    -1,
     192,    90,   384,   380,    -1,   192,    90,   192,     8,   380,
      -1,   192,    90,   192,    -1,   192,    90,   384,    -1,    79,
     122,   385,    -1,   382,     8,   381,    -1,   381,    -1,   382,
     325,    -1,    -1,   166,   170,   383,   171,    -1,    25,   385,
      -1,    51,   385,    -1,   194,    -1,   124,    -1,   384,    -1,
     124,   162,   385,   163,    -1,   124,   162,   385,     8,   385,
     163,    -1,   146,    -1,   170,    98,   170,   378,   171,    26,
     385,   171,    -1,   170,   377,     8,   385,   171,    -1,   385,
      -1,    -1
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
    1932,  1934,  1935,  1938,  1939,  1943,  1946,  1952,  1953,  1954,
    1955,  1956,  1957,  1958,  1963,  1965,  1969,  1970,  1973,  1974,
    1978,  1981,  1983,  1985,  1989,  1990,  1991,  1993,  1996,  2000,
    2001,  2002,  2005,  2006,  2007,  2008,  2009,  2011,  2012,  2018,
    2020,  2023,  2026,  2028,  2030,  2033,  2035,  2039,  2041,  2044,
    2048,  2055,  2057,  2060,  2061,  2066,  2069,  2073,  2073,  2078,
    2081,  2082,  2086,  2087,  2092,  2093,  2097,  2098,  2102,  2103,
    2107,  2109,  2113,  2114,  2115,  2116,  2117,  2118,  2119,  2120,
    2123,  2125,  2129,  2130,  2131,  2132,  2133,  2135,  2137,  2139,
    2143,  2144,  2145,  2149,  2152,  2155,  2158,  2161,  2164,  2170,
    2174,  2181,  2182,  2187,  2189,  2190,  2193,  2194,  2197,  2198,
    2202,  2203,  2207,  2208,  2209,  2210,  2211,  2214,  2217,  2218,
    2219,  2221,  2223,  2227,  2228,  2229,  2231,  2232,  2233,  2237,
    2239,  2242,  2244,  2245,  2246,  2247,  2250,  2252,  2253,  2257,
    2259,  2262,  2264,  2265,  2266,  2270,  2272,  2275,  2278,  2280,
    2282,  2286,  2287,  2289,  2290,  2296,  2297,  2299,  2301,  2303,
    2305,  2308,  2309,  2310,  2314,  2315,  2316,  2317,  2318,  2319,
    2320,  2324,  2325,  2329,  2338,  2345,  2346,  2352,  2353,  2361,
    2364,  2368,  2371,  2376,  2377,  2378,  2379,  2383,  2384,  2388,
    2390,  2391,  2393,  2396,  2398,  2403,  2409,  2411,  2415,  2418,
    2421,  2430,  2433,  2436,  2437,  2440,  2441,  2445,  2450,  2454,
    2460,  2468,  2469
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
     321,   321,   321,   321,   321,   322,   322,   323,   323,   323,
     323,   323,   323,   323,   324,   324,   325,   325,   326,   326,
     327,   327,   327,   327,   328,   328,   328,   328,   328,   329,
     329,   329,   330,   330,   330,   330,   330,   330,   330,   331,
     331,   332,   332,   332,   332,   333,   333,   334,   334,   335,
     335,   336,   336,   337,   337,   338,   338,   340,   339,   341,
     342,   342,   343,   343,   344,   344,   345,   345,   346,   346,
     347,   347,   348,   348,   348,   348,   348,   348,   348,   348,
     348,   348,   349,   349,   349,   349,   349,   349,   349,   349,
     350,   350,   350,   351,   351,   351,   351,   351,   351,   352,
     352,   353,   353,   354,   354,   354,   355,   355,   356,   356,
     357,   357,   358,   358,   358,   358,   358,   358,   359,   359,
     359,   359,   359,   360,   360,   360,   360,   360,   360,   361,
     361,   362,   362,   362,   362,   362,   362,   362,   362,   363,
     363,   364,   364,   364,   364,   365,   365,   366,   366,   366,
     366,   367,   367,   367,   367,   368,   368,   368,   368,   368,
     368,   369,   369,   369,   370,   370,   370,   370,   370,   370,
     370,   371,   371,   372,   373,   374,   374,   375,   375,   376,
     376,   377,   377,   378,   378,   378,   378,   379,   379,   380,
     380,   380,   380,   380,   380,   381,   382,   382,   383,   383,
     384,   385,   385,   385,   385,   385,   385,   385,   385,   385,
     385,   386,   386
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
       4,     3,     4,     1,     1,     3,     3,     1,     1,     1,
       1,     3,     3,     3,     2,     0,     1,     0,     1,     0,
       5,     3,     3,     1,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     2,     2,     4,     3,     4,     2,
       0,     5,     3,     3,     1,     3,     1,     2,     0,     5,
       3,     2,     0,     3,     0,     4,     2,     0,     3,     3,
       1,     0,     1,     2,     2,     4,     3,     3,     2,     4,
       2,     4,     1,     1,     1,     1,     1,     2,     4,     3,
       4,     3,     1,     1,     1,     1,     2,     4,     4,     3,
       1,     1,     3,     7,     6,     8,     9,     8,    10,     7,
       6,     1,     2,     4,     4,     1,     1,     4,     1,     0,
       1,     2,     1,     1,     2,     4,     3,     3,     0,     1,
       2,     4,     3,     2,     3,     6,     0,     1,     4,     2,
       0,     5,     3,     3,     1,     6,     4,     4,     2,     2,
       0,     5,     3,     3,     1,     2,     0,     5,     3,     3,
       1,     2,     2,     1,     2,     1,     4,     3,     3,     6,
       3,     1,     1,     1,     4,     4,     2,     2,     4,     2,
       2,     1,     3,     3,     5,     1,     2,     1,     4,     3,
       0,     1,     3,     3,     1,     1,     0,     0,     2,     3,
       1,     4,     5,     3,     3,     3,     3,     1,     2,     0,
       4,     2,     2,     1,     1,     1,     4,     6,     1,     8,
       5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   557,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   630,     0,   618,   476,
       0,   482,   483,    19,   508,   606,    70,   484,     0,    52,
       0,     0,     0,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,   469,     0,     0,
       0,     0,   112,     0,     0,     0,   488,   490,   491,   485,
     486,     0,     0,   492,   487,     0,     0,   467,    20,    21,
      22,    24,    23,     0,   489,     0,     0,     0,   493,     0,
      69,    42,   610,   477,     0,     0,     4,    31,    33,    36,
     507,     0,   466,     0,     6,    90,     7,     8,     9,     0,
     273,     0,     0,     0,     0,   271,   338,   337,   347,   346,
       0,   345,   573,   468,     0,   510,   336,     0,   576,   272,
       0,     0,   574,   575,   572,   601,   605,     0,   326,   509,
      10,   469,     0,     0,    31,    90,   670,   272,   669,     0,
     667,   666,   340,     0,     0,   310,   311,   312,   313,   335,
     333,   332,   331,   330,   329,   328,   327,   469,     0,   680,
     468,     0,   293,   291,     0,   634,     0,   517,   278,   472,
       0,   680,   471,     0,   481,   613,   612,   473,     0,     0,
     475,   334,     0,     0,     0,   265,     0,    50,   267,     0,
       0,    56,    58,     0,     0,    60,     0,     0,     0,   704,
     708,     0,     0,    31,   703,     0,   705,     0,    62,     0,
      42,     0,     0,     0,    26,    27,   176,     0,     0,   175,
     114,   113,   181,    90,     0,     0,     0,     0,     0,   677,
     100,   110,   626,   630,   655,     0,   495,     0,     0,     0,
     653,     0,    15,     0,    35,     0,   268,   104,   111,   378,
     353,     0,   273,     0,   271,   272,     0,     0,   478,     0,
     479,     0,     0,     0,    82,     0,     0,    38,   169,     0,
      18,    89,     0,   109,    96,   108,    79,    80,    81,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   618,    78,   609,   609,   640,     0,
       0,     0,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   292,   290,     0,   577,
     562,   609,     0,   568,   169,   609,     0,   611,   602,   626,
       0,     0,     0,   559,   554,   517,     0,     0,     0,     0,
     638,     0,   358,   516,   629,     0,     0,    38,     0,   169,
     261,     0,   614,   562,   570,   474,     0,    42,   149,     0,
      67,     0,     0,   266,     0,     0,     0,     0,     0,    59,
      77,    61,   701,   702,     0,   699,     0,     0,   681,     0,
     676,    63,     0,    76,    28,     0,    17,     0,     0,   177,
       0,    65,     0,     0,     0,    66,   671,     0,     0,     0,
       0,     0,   120,     0,   627,     0,     0,     0,     0,   494,
     654,   508,     0,     0,   652,   513,   651,    34,     5,    12,
      13,    64,   118,     0,     0,     0,   517,     0,     0,   262,
     323,   581,    47,    41,    43,    44,    45,    46,     0,   339,
     511,   512,    32,     0,     0,     0,   519,   170,     0,   341,
      92,   116,   296,   298,   297,     0,     0,   294,   295,   299,
     301,   300,   315,   314,   317,   316,   318,   320,   321,   319,
     309,   308,   303,   304,   302,   305,   306,   307,   322,   608,
       0,     0,   644,     0,   517,   673,   579,   601,   102,   106,
       0,    98,     0,     0,   269,   275,   289,   288,   287,   286,
     285,   284,   283,   282,   281,   280,   279,     0,   564,   563,
       0,     0,     0,     0,     0,     0,   668,   552,   556,   516,
     558,     0,     0,   680,     0,   633,     0,   632,     0,   617,
     616,     0,     0,   564,   563,   263,   151,   153,   264,     0,
      42,   133,    51,   267,     0,     0,     0,     0,   145,   145,
      57,     0,     0,   697,   517,     0,   686,     0,     0,     0,
     515,     0,     0,   467,     0,    36,   497,   466,   504,     0,
     496,    40,   503,    85,     0,    25,    29,     0,   174,   182,
     343,   179,     0,     0,   664,   665,    11,    36,   690,     0,
       0,     0,   626,   623,     0,   357,   663,   662,   661,     0,
     657,     0,   658,   660,     0,     5,     0,     0,   372,   373,
     381,   380,     0,     0,   516,   352,   356,     0,     0,   578,
     562,   569,   607,     0,   679,   171,   465,   518,   168,     0,
     561,     0,     0,   118,   325,     0,   361,   362,     0,   359,
     516,   639,     0,   169,   120,   118,    94,   116,   618,   276,
       0,     0,   169,   566,   567,   580,   603,   604,     0,     0,
       0,   540,   524,   525,   526,     0,     0,     0,   533,   532,
     546,   517,     0,   554,   637,   636,     0,   615,   562,   571,
     480,     0,   155,     0,     0,    48,     0,     0,     0,     0,
     126,   127,   137,     0,    42,   135,    73,   145,     0,   145,
       0,     0,   706,     0,   516,   698,   700,   685,   684,     0,
     682,   498,   499,   523,     0,   517,   515,     0,     0,   355,
       0,   646,     0,    75,     0,    30,   178,   561,     0,   672,
      68,     0,     0,   678,   119,   121,   184,     0,     0,   624,
       0,   656,     0,    16,     0,   117,   184,     0,     0,   349,
       0,   674,     0,   564,   563,   682,     0,   172,    39,   158,
       0,   519,   560,   712,   561,   115,     0,   324,   643,   642,
     169,     0,     0,     0,     0,   118,   481,   565,   169,     0,
       0,   529,   530,   531,   534,   535,   544,     0,   517,   540,
       0,   528,   548,   516,   551,   553,   555,     0,   631,   565,
       0,     0,     0,     0,   152,    53,     0,   267,   128,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   139,     0,
     695,   696,     0,     0,   710,     0,   501,   516,   514,     0,
     506,     0,   517,     0,   505,   650,     0,   517,     0,     0,
       0,   180,   689,   693,   694,     0,   246,   628,   626,   270,
     274,     0,    14,   246,   384,     0,     0,   386,   379,   382,
       0,   377,     0,     0,     0,   169,   173,   687,   561,   157,
     711,     0,     0,   184,     0,     0,   600,   184,   184,   561,
       0,   277,   169,     0,   594,     0,   537,   516,   539,     0,
     527,     0,     0,   517,   545,   635,     0,    42,     0,   148,
     134,     0,   125,    71,   138,     0,     0,   141,     0,   146,
     147,    42,   140,   707,   683,     0,   522,   521,   500,     0,
     516,   354,   502,     0,   360,   516,   645,     0,    42,   687,
       0,   691,   122,     0,     0,   249,   250,   251,   254,   253,
     252,   244,     0,     0,     0,   101,   183,   185,     0,   243,
     247,     0,   246,     0,   659,   105,   375,     0,     0,   348,
     565,   169,     0,     0,   367,   156,   712,     0,   160,   687,
     246,   641,   599,   246,   246,     0,   184,     0,   593,   543,
     542,   536,     0,   538,   516,   547,    42,   154,    49,    54,
       0,   136,   142,    42,   144,     0,     0,   351,     0,   649,
     648,     0,     0,   367,   692,     0,     0,   123,   213,   211,
     467,    24,     0,   207,     0,   212,   223,     0,   221,   226,
       0,   225,     0,   224,     0,    90,   248,   187,     0,   189,
       0,   245,   625,   376,   374,   385,   383,   169,     0,   597,
     688,     0,     0,     0,   161,     0,     0,    97,   103,   107,
     687,   246,   595,     0,   550,     0,   150,     0,    42,   131,
      72,   143,   709,   520,     0,     0,     0,    86,     0,     0,
       0,   197,   201,     0,     0,   194,   435,   434,   431,   433,
     432,   451,   453,   452,   423,   413,   429,   428,   391,   400,
     401,   403,   402,   422,   406,   404,   405,   407,   408,   409,
     410,   411,   412,   414,   415,   416,   417,   418,   419,   421,
     420,   392,   393,   394,   396,   397,   399,   437,   438,   447,
     446,   445,   444,   443,   442,   430,   448,   439,   440,   441,
     424,   425,   426,   427,   449,   450,   454,   456,   455,   457,
     458,   436,   460,   459,   395,   462,   463,   398,   464,   461,
     390,   218,   387,     0,   195,   239,   240,   238,   231,     0,
     232,   196,   257,     0,     0,     0,     0,    90,     0,   596,
       0,    42,     0,   164,     0,   163,    42,     0,    99,   541,
       0,    42,   129,    55,     0,   350,   647,    42,    42,   260,
     124,     0,     0,   215,   208,     0,     0,     0,   220,   222,
       0,     0,   227,   234,   235,   233,     0,     0,   186,     0,
       0,     0,     0,   598,     0,   370,   519,     0,   165,     0,
     162,     0,    42,   549,     0,     0,     0,     0,   198,    31,
       0,   199,   200,     0,     0,   214,   217,   388,   389,     0,
     209,   236,   237,   229,   230,   228,   258,   255,   190,   188,
     259,     0,   371,   518,     0,   342,     0,   167,    93,     0,
       0,   132,    84,   344,     0,   246,   216,   219,     0,   561,
     192,     0,   368,   366,   166,    95,   130,    88,   205,     0,
     245,   256,     0,   561,   369,     0,    87,    74,     0,     0,
     204,   687,     0,     0,     0,   203,     0,   687,     0,   202,
     241,    42,   191,     0,     0,     0,   193,     0,   242,    42,
       0,    83
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    96,   625,   438,   144,   223,   224,
      98,    99,   100,   101,   102,   103,   266,   453,   454,   382,
     196,  1077,   388,  1010,  1297,   743,   744,  1307,   282,   145,
     455,   651,   794,   456,   471,   667,   422,   664,   457,   442,
     665,   284,   240,   257,   109,   653,   627,   611,   754,  1026,
     830,   710,  1203,  1080,   562,   716,   387,   570,   718,   931,
     557,   702,   705,   822,   780,   781,   465,   466,   228,   229,
     234,   866,   966,  1044,  1185,  1289,  1303,  1211,  1251,  1252,
    1253,  1032,  1033,  1034,  1212,  1218,  1260,  1037,  1038,  1042,
    1178,  1179,  1180,  1322,   967,   968,   969,   970,  1183,   971,
     110,   190,   383,   384,   111,   112,   113,   114,   115,   650,
     747,   446,   852,   447,   853,   116,   117,   118,   588,   119,
     120,  1062,  1236,   121,   443,  1054,   444,   767,   632,   881,
     878,  1171,  1172,   122,   123,   124,   184,   191,   269,   370,
     125,   733,   592,   126,   734,   364,   648,   735,   689,   804,
     806,   807,   808,   691,   912,   913,   692,   538,   355,   153,
     154,   127,   783,   339,   340,   641,   128,   185,   147,   130,
     131,   132,   133,   134,   135,   136,   500,   137,   187,   188,
     425,   176,   177,   503,   504,   856,   857,   249,   250,   619,
     138,   417,   139,   140,   215,   241,   277,   397,   729,   984,
     609,   573,   574,   575,   216,   217,   891
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -951
static const yytype_int16 yypact[] =
{
    -951,   113,  -951,  -951,  3475,  9670,  9670,   -61,  9670,  9670,
    9670,  -951,  9670,  9670,  9670,  9670,  9670,  9670,  9670,  9670,
    9670,  9670,  9670,  9670,  1752,  1752,  2939,  9670,  2225,   -47,
     -32,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  9670,  -951,
     -32,     2,    23,   134,   -32,  7369,  1134,  7546,  -951,  1519,
    7723,   149,  9670,   706,    28,    62,   208,    27,   167,   188,
     209,   213,  -951,  1134,   218,   221,  -951,  -951,  -951,  -951,
    -951,   363,   321,  -951,  -951,  1134,  7900,  -951,  -951,  -951,
    -951,  -951,  -951,  1134,  -951,    -5,   225,  1134,  -951,  9670,
    -951,  -951,   204,   342,   405,   405,  -951,   371,   255,   192,
    -951,   232,  -951,    38,  -951,   381,  -951,  -951,  -951,   952,
    -951,   235,   242,   253,  8063,  -951,  -951,   386,  -951,   390,
     395,  -951,    29,   285,   320,  -951,  -951,   570,    24,  2224,
      86,   291,    92,    99,   294,    74,  -951,   191,  -951,   406,
    -951,   368,   307,   338,  -951,   381, 11068,  2302, 11068,  9670,
   11068, 11068,  3638,   456,  1134,  -951,  -951,   468,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  1013,   343,
    -951,   382,   402,   402,  1752, 10728,   351,   528,  -951,   401,
    1013,   343,   407,   409,   384,   101,  -951,   430,    86,  8077,
    -951,  -951,  9670,  6307,    39, 11068,  7192,  -951,  9670,  9670,
    1134,  -951,  -951,  9479,   387,  -951,  9656,  1519,  1519,   403,
    -951,   391,  1219,   551,  -951,   555,  -951,  1134,  -951,  9833,
    -951, 10542,  1134,    43,  -951,    37,  -951,   770,    45,  -951,
    -951,  -951,   563,   381,    46,  1752,  1752,  1752,   411,   410,
    -951,  -951,  2315,  2939,   212,   461,  -951,  9847,  1752,   419,
    -951,  1134,  -951,   -41,   255,   420, 11068,  -951,  -951,  -951,
     498,   578,   423, 11068,   425,   766,  3652,  9670,    -2,   426,
     472,    -2,   207,   230,  -951,  1134,  1519,   435,  8254,  1519,
    -951,  -951,   378,  -951,  -951,  -951,  -951,  -951,  -951,  9670,
    9670,  9670,  8431,  9670,  9670,  9670,  9670,  9670,  9670,  9670,
    9670,  9670,  9670,  9670,  9670,  9670,  9670,  9670,  9670,  9670,
    9670,  9670,  9670,  9670,  2225,  -951,  9670,  9670,  9670,   337,
    1134,  1134,   381,   952,  2639,  9670,  9670,  9670,  9670,  9670,
    9670,  9670,  9670,  9670,  9670,  9670,  -951,  -951,   367,  -951,
     104,  9670,  9670,  -951,  8254,  9670,  9670,   204,   109,  2315,
     436,  8608, 10583,  -951,   438,   614,  1013,   454,   -64,   337,
     402,  8785,  -951,  8962,  -951,   459,   -17,  -951,   206,  8254,
    -951,   422,  -951,   110,  -951,  -951, 10647,  -951,  -951,  9670,
    -951,   545,  6484,   624,   465, 10961,   620,    52,    71,  -951,
    -951,  -951,  -951,  -951,  1519,   568,   478,   642,  -951, 10655,
    -951,  -951,  3829,  -951,   186,   706,  -951,  1134,  9670,   402,
      28,  -951, 10655,   485,   585,  -951,   402,    58,    64,     9,
     488,  1026,   546,   494,   402,    69,   495,   856,  1134,  -951,
    -951,   606,  2810,   -29,  -951,  -951,  -951,   255,  -951,  -951,
    -951,  -951,   553,   523,     6,   564,   681,   521,  1519,   145,
     632,    88,  -951,  -951,  -951,  -951,  -951,  -951, 10126,  -951,
    -951,  -951,  -951,    59,  1752,   525,   686, 11068,   684,  -951,
    -951,   579,  7532,  3464,  3638,  9670, 11027,  2832,  3288,  3989,
    4165,  4341,  4518,  4518,  4518,  4518,  2478,  2478,  2478,  2478,
     597,   597,   399,   399,   399,   468,   468,   468,  -951, 11068,
     522,   527, 10824,   532,   694,   -22,   539,   109,  -951,  -951,
    1134,  -951,  2024,  9670,  -951,  3638,  3638,  3638,  3638,  3638,
    3638,  3638,  3638,  3638,  3638,  3638,  3638,  9670,   -22,   541,
     536, 10167,   543,   540, 10208,    72,  -951,  1869,  -951,  1134,
    -951,   423,   145,   343,  1752, 11068,  1752, 10865,   147,   114,
    -951,   550,  9670,  -951,  -951,  -951,  6130,    67, 11068,   -32,
    -951,  -951,  -951,  9670,   817, 10655,  1134,  6661,   544,   554,
    -951,    83,   600,  -951,   715,   556,  1307,  1519, 10655, 10655,
   10655,   560,    18,   589,   562,   151,  -951,   595,  -951,   567,
    -951,  -951,  -951,   635,  1134,  -951,  -951, 10249,  -951,  -951,
    -951,   730,  1752,   573,  -951,  -951,  -951,  -951,    68,   583,
     662,   574,  2315,  2359,   735,  -951,  -951,  -951,  -951,   580,
    -951,  9670,  -951,  -951,  3121,  -951,   662,   592,  -951,  -951,
    -951,  -951,   738,  9670,   677,  -951,  -951,   586,   601,  -951,
     115,  -951,  -951,  1519,  -951,   402,  -951,  9139,  -951, 10655,
      81,   621,   662,   553,  3815,  9670,  -951,  -951,  9670,  -951,
    9670,  -951,   622,  8254,   546,   553,  -951,   579,  2225,   402,
   10293,   623,  8254,  -951,  -951,   116,  -951,  -951,   754,  1050,
    1050,  1869,  -951,  -951,  -951,   625,    20,   628,  -951,  -951,
    -951,   762,   633,   438,   402,   402,  9316,  -951,   117,  -951,
    -951, 10334,   244,   -32,  7192,  -951,   599,  4006,   627,  1752,
     672,   402,  -951,   794,  -951,  -951,  -951,  -951,   487,  -951,
      17,  1519,  -951,  1519,   568,  -951,  -951,  -951,   804,   645,
     646,  -951,  -951,   696,   647,   812, 10655,   685,  1134,   744,
    1134, 10655,   659,  -951,   679,  -951,  -951,    81, 10655,   402,
    -951,  1026,  1097,  -951,   823,  -951,  -951,    78,   674,   402,
    9493,  -951,  2028,  -951,  3298,   823,  -951,   200,   -45, 11068,
     723,  -951,  9670,   -22,   683,  -951,  1752, 11068,  -951,  -951,
     690,   848,  -951,  1519,    81,  -951,   689,  3815, 11068, 10920,
    8254,   692,   693,   702,   708,   553,   384,   709,  8254,   705,
    9670,  -951,  -951,  -951,  -951,  -951,   763,   707,   875,  1869,
     755,  -951,   814,  1869,  -951,  -951,  -951,  1752, 11068,  -951,
     -32,   865,   826,  7192,  -951,  -951,   724,  9670,   402,   817,
     726, 10655,  4183,   492,   727,  9670,    15,    21,  -951,   737,
    -951,  -951,  1372,   879,  -951, 10655,  -951, 10655,  -951,   746,
    -951,   784,   900,   749,  -951,   793,   750,   918,   662,   759,
     765,  -951,  -951,   925,  1026,   662,   761,  -951,  2315,  -951,
    3638,   760,  -951,  1654,  -951,   140,  9670,  -951,  -951,  -951,
    9670,  -951,  9670, 10375,   768,  8254,   402,   913,    85,  -951,
    -951,   112,   773,  -951,  9670,   775,  -951,  -951,  -951,    81,
     786,  -951,  8254,   777,  -951,  1869,  -951,  1869,  -951,   796,
    -951,   819,   800,   949,  -951,   402,   938,  -951,   801,  -951,
    -951,   803,  -951,  -951,  -951,   811,   813,  -951, 10501,  -951,
    -951,  -951,  -951,  -951,  -951,  1519,  -951,   850,  -951, 10655,
     905,  -951,  -951, 10655,  -951, 10655,  -951,   916,  -951,   913,
    1026,  -951,  -951,  1519,   662,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  2066,   840,   827,  -951,  -951,  -951,   368,  1485,
    -951,    47,   844,    80,  -951,  -951,   845, 10416, 10460, 11068,
     824,  8254,   825,  1519,   895,  -951,  1519,   929,   998,   913,
    1702, 11068,  -951,  1732,  1851,   841,  -951,   842,  -951,  -951,
     892,  -951,  1869,  -951,   937,  -951,  -951,  6130,  -951,  -951,
    6838,  -951,  -951,  -951,  6130,   846, 10655,  -951,   897,  -951,
     902,   851,  4360,   895,  -951,  1015,    26,  -951,  -951,  -951,
      48,   853,    54,  -951,  9975,  -951,  -951,    55,  -951,  -951,
     466,  -951,   861,  -951,   961,   381,  -951,  -951,  1519,  -951,
     368,   844,  -951,  -951,  -951,  -951,  -951,  8254,   864,  -951,
    -951,   867,   868,   275,  1027, 10655,   870,  -951,  -951,  -951,
     913,  1982,  -951,  1869,  -951,   923,  6130,  7015,  -951,  -951,
    -951,  6130,  -951,  -951, 10655, 10655,   878,  -951,   880, 10655,
     662,  -951,  -951,  1433,  2066,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,   313,  -951,   840,  -951,  -951,  -951,  -951,  -951,    41,
     376,  -951,  1039,    56,  1134,   961,  1043,   381,   886,  -951,
     290,  -951,   985,  1052, 10655,  -951,  -951,   901,  -951,  -951,
    1869,  -951,  -951,  -951,  4537,  -951,  -951,  -951,  -951,  -951,
    -951,   278,    35,  -951,  -951, 10655,  9975,  9975,  1017,  -951,
     466,   466,   412,  -951,  -951,  -951, 10655,   997,  -951,   908,
      60, 10655,  1134,  -951,  1002,  -951,  1072,  4714,  1068, 10655,
    -951,  4891,  -951,  -951,  5068,   911,  5245,  5422,  -951,   999,
     947,  -951,  -951,  1000,  1433,  -951,  -951,  -951,  -951,   939,
    -951,  1065,  -951,  -951,  -951,  -951,  -951,  1086,  -951,  -951,
    -951,   936,  -951,   298,   940,  -951, 10655,  -951,  -951,  5599,
     935,  -951,  -951,  -951,  1134,   844,  -951,  -951, 10655,    81,
    -951,  1037,  -951,  -951,  -951,  -951,  -951,    -3,   960,  1134,
      10,  -951,   945,    81,  -951,   955,  -951,  -951,   662,   950,
    -951,   913,   946,   662,    61,  -951,   178,   913,  1054,  -951,
    -951,  -951,  -951,   178,   958,  5776,  -951,   957,  -951,  -951,
    5953,  -951
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -951,  -951,  -951,  -409,  -951,  -951,  -951,    -4,  -951,   739,
      12,   483,  -238,  -951,  1205,  -951,  -176,  -951,    11,  -951,
    -951,  -951,  -951,  -951,  -951,  -166,  -951,  -951,  -138,    32,
      -1,  -951,  -951,     4,  -951,  -951,  -951,  -951,     5,  -951,
    -951,   820,   828,   831,  1019,   480,  -559,   491,   530,  -159,
    -951,   329,  -951,  -951,  -951,  -951,  -951,  -951,  -529,   241,
    -951,  -951,  -951,  -951,  -736,  -951,  -319,  -951,  -951,   753,
    -951,  -729,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,    66,  -951,  -951,  -951,  -951,  -951,     3,  -951,
     217,  -816,  -951,  -151,  -951,  -950,  -942,  -943,   -14,  -951,
     -56,   -20,  1144,  -525,  -310,  -951,  -951,  2259,  1095,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,   163,  -951,   428,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -784,  -951,  1166,  1620,  -298,  -951,  -951,   393,
     643,  -103,  -951,  -951,   451,  -353,  -768,  -951,  -951,   513,
    -518,   388,  -951,  -951,  -951,  -951,  -951,   516,  -951,  -951,
    -951,  -618,   322,  -152,  -148,  -113,  -951,  -951,    57,  -951,
    -951,  -951,  -951,    -7,  -114,  -951,   146,  -951,  -951,  -951,
    -343,   968,  -951,  -951,  -951,  -951,  -951,   559,   869,  -951,
    -951,   978,  -951,  -951,  -275,   -82,  -169,  -248,  -951,  -931,
    -720,   496,  -951,  -951,   463,  -134,   239
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -681
static const yytype_int16 yytable[] =
{
      97,   258,   540,   106,   468,   261,   535,   350,   107,   108,
     193,   860,   367,   889,   514,   104,   498,   343,  1023,   690,
     197,   186,  1050,   348,   201,   532,  1046,   285,   463,   624,
    1051,   862,   782,   262,  1090,   372,   105,   873,   708,   373,
     720,   929,   204,  1254,   402,   213,   279,   379,   892,  1220,
     551,   405,   629,   410,   414,  1048,  -210,   338,  1066,   239,
     566,   129,  1094,  1173,  1227,   225,   602,   643,  1227,  1090,
    1221,   244,   602,   392,   393,   374,   751,   613,   398,   239,
     613,   172,   173,   239,   253,  -585,   613,   254,   613,   233,
    -582,   721,   338,   635,   786,   413,   737,   568,   810,  1305,
     232,   226,   879,   275,   338,   239,   793,   451,  1046,   149,
     835,   836,   357,     3,   835,   836,   955,   956,   957,   958,
     959,   960,    11,   189,   365,    48,    11,   407,   880,   782,
     338,   439,   440,   703,   704,   345,   247,   248,   192,  1197,
     276,   259,   398,   987,   951,   623,   265,   341,  -680,  -589,
     354,   661,   630,  -583,   549,   246,   -85,   811,   752,   322,
    -584,   586,  -619,   995,   990,  -586,   782,   631,   993,   994,
     345,  -620,   198,   233,   586,  -622,  -587,  -588,  -621,   230,
     605,   275,  1310,   608,   510,   988,   976,   930,   833,    97,
     837,   838,    97,   199,  -591,   932,   386,  -585,  1091,  1092,
     470,   556,  -582,   227,   378,   507,   341,   381,   341,  1255,
     280,   380,  1222,   400,  -470,   406,   764,   411,   415,  1049,
    -210,   725,   644,   567,   507,   358,  1095,  1174,  1228,   603,
    1024,   360,  1269,  1319,   404,   604,   900,   366,   779,   258,
     614,   285,   985,   678,   569,   507,   722,   346,   972,   867,
     129,  1052,  -159,   129,   507,   972,  -518,   507,  -592,   342,
     571,  -589,    97,   437,    35,  -583,   638,  1071,   371,   757,
     782,   462,  -584,   427,  -619,   213,   594,  -586,   239,    35,
     244,   782,   346,  -620,   409,   434,   259,  -622,  -587,  -588,
    -621,  -680,   416,   416,   419,   914,   591,   639,   105,   424,
     541,   640,   921,   244,   200,   433,  1192,   186,   434,   599,
     820,   821,   506,   276,   637,   505,   239,   239,   342,   239,
     342,  1234,   220,   129,  -680,   231,  1215,   586,   728,  1291,
     275,   529,  -680,   428,   528,  1299,   662,   235,   814,  1216,
     586,   586,   586,  1300,   791,   247,   248,   874,  1193,    33,
    1320,  1321,   506,   799,   276,   543,  1217,  1046,   236,   671,
     875,   550,  -680,  1235,   554,  -680,   347,   553,   247,   248,
     796,  1292,   972,   876,   662,   972,   972,   267,    97,   237,
    1316,    92,   848,   238,   707,   460,  1323,   999,   242,  1000,
     639,   243,    33,   561,   640,   260,   697,   274,    97,   275,
     698,  1223,   278,   596,  1261,  1262,   424,   286,    33,   461,
      35,   586,   281,   358,   287,   244,   367,   225,  1224,   143,
     268,  1225,    75,   618,   620,   288,    78,    79,   666,    80,
      81,    82,  1257,  1258,   105,   699,   244,  1263,    33,   129,
      35,   245,   398,   730,   311,   312,   313,  -363,   314,    33,
     869,   316,  1248,   972,  1264,   908,   317,  1265,   318,   129,
     319,   344,   712,   501,  -590,   251,    48,  -364,  1274,    78,
      79,   895,    80,    81,    82,   731,   732,   349,   244,   903,
     247,   248,   251,   271,  1074,    78,    79,   530,    80,    81,
      82,   533,   244,    33,   252,    35,   353,   434,   586,   941,
     246,   247,   248,   586,   946,   276,   239,   169,   169,   775,
     586,   181,    92,   608,   863,    78,    79,   314,    80,    81,
      82,   645,   359,   338,   507,   973,    78,    79,   362,    80,
      81,    82,   181,   688,   244,   693,   363,    33,   832,   706,
     527,  -469,    92,   247,   248,   244,   778,  -468,   469,   368,
     434,   371,    97,  1302,   369,  1199,   435,   247,   248,   390,
    1005,   395,   713,    97,  -675,   394,   982,  1312,   399,   669,
      78,    79,   421,    80,    81,    82,   412,   445,   715,   834,
     835,   836,   420,   997,   926,   835,   836,   839,   105,   840,
     745,   448,   441,   586,   449,   552,   450,    92,   429,   247,
     248,   694,   459,   695,   884,   -37,   469,   586,   537,   586,
     247,   248,  1175,   129,    78,    79,  1176,    80,    81,    82,
      97,   711,   539,   106,   129,   542,   608,   559,   107,   108,
     548,   774,   379,   565,   773,   104,  1040,   563,   855,   308,
     309,   310,   311,   312,   313,   861,   314,   572,   576,   890,
     577,   169,   270,   272,   273,   600,   105,   169,   601,   749,
     606,   186,  1058,   169,   612,   610,   615,   621,    48,   424,
     759,   782,    33,   626,    35,   803,   803,   688,  1025,    55,
      56,   129,  1243,   823,   628,   782,   633,    62,   320,   634,
     181,   181,   636,  -365,   647,   181,   646,   649,   652,   656,
      97,   586,   660,    97,   657,   586,   659,   586,   775,   663,
     169,   672,   608,   673,   675,   824,   717,   676,   169,   169,
     169,   700,   723,   724,   321,   169,   719,   726,   924,   738,
     736,   169,   739,    33,   850,   740,   854,   742,  1188,   105,
     741,  1007,   936,   748,   937,   750,   753,   756,   760,    78,
      79,   768,    80,    81,    82,  1014,   770,   761,   771,   181,
      97,   129,   181,   106,   129,   766,   828,   800,   107,   108,
     813,   825,  1022,  1186,   772,   104,    92,    33,   586,   351,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   784,   790,   798,   829,   809,   105,   181,   812,   827,
     916,  1015,    11,   143,   815,   688,    75,   831,    77,   688,
      78,    79,   842,    80,    81,    82,   843,   844,   845,    97,
     847,   129,   429,   851,   846,   336,   337,   586,    97,   858,
    1076,   865,   169,   886,   919,  1047,  1017,  1081,   859,   169,
    1019,    33,  1020,    35,   868,   882,   586,   586,   709,  1060,
     222,   586,   890,   885,    78,    79,   888,    80,    81,    82,
     953,   887,   893,   896,   105,   954,   897,   955,   956,   957,
     958,   959,   960,   961,   915,   898,   904,   181,   899,   902,
     129,   167,   585,   907,   906,   905,   711,   338,    33,   129,
      35,   917,   910,   911,   918,   585,   920,   923,    33,   927,
     933,   688,  1204,   688,   607,   935,   939,  1184,   940,   962,
     963,   143,   964,  1083,    75,   943,    77,   938,    78,    79,
     942,    80,    81,    82,   944,   424,   945,    33,   167,   616,
     617,   181,   948,   950,   974,   965,   949,   451,   981,   983,
     168,  1002,  1039,   408,   989,    92,   992,   169,   998,   213,
     955,   956,   957,   958,   959,   960,   586,  1004,   143,   996,
    1041,    75,  1195,    77,  1006,    78,    79,  1001,    80,    81,
      82,  1003,  1016,  1008,  1009,    78,    79,   586,    80,    81,
      82,  1205,  1206,  1011,  1018,  1012,  1209,   168,   586,  1021,
    1036,  1053,    92,   586,  1057,   169,  1059,  1040,   688,  1061,
    1045,   586,  1064,    97,    78,    79,    97,    80,    81,    82,
      97,  1065,  1070,  1072,  1073,  1237,  1075,  1082,    97,  1084,
    1241,  1079,  1086,    33,  1085,  1244,  1093,   169,  1089,   169,
    1170,  1246,  1247,  1181,  1182,  1189,  1177,  1190,   586,   105,
    1194,  1191,   590,  1196,   213,  1200,   105,   169,   585,  1232,
     586,  1207,  1226,  1208,   105,   590,  1231,  1233,  1238,   181,
     181,   585,   585,   585,   129,  1239,  1279,   129,  1259,   688,
    1267,   129,    97,    97,  1242,  1272,    28,    97,  1268,   129,
    1273,  1276,  1187,  1281,    33,   169,    35,  1284,  1202,  -206,
    1285,  1240,  1287,   181,  1221,   169,   169,    33,   283,  1288,
      78,    79,  1229,    80,    81,    82,  1290,  1296,   105,   181,
    1304,  1293,  1256,   105,   430,  1308,  1311,  1317,   436,   801,
     802,    33,  1315,  1266,   167,  1313,   181,  1324,  1270,  1327,
    1329,  1306,   585,   129,   129,   181,  1277,   430,   129,   436,
     430,   436,   436,   511,   595,  1325,   323,   795,   508,  1314,
    1271,   181,   509,  1330,   143,   792,   765,    75,   922,    77,
    1214,    78,    79,   598,    80,    81,    82,   143,    33,  1013,
      75,  1230,  1326,  1294,    78,    79,  1219,    80,    81,    82,
     239,  1043,   194,   356,   264,  1301,  1088,   849,    92,   901,
     170,   170,   169,   805,   182,   877,   688,   909,    78,    79,
      97,    80,    81,    82,   181,    33,   181,  1249,   590,   816,
     986,   426,  1170,  1170,   418,   864,  1177,  1177,     0,   585,
     841,   590,   590,   590,   585,  1063,     0,     0,   239,     0,
       0,   585,     0,    97,   607,   607,   105,    97,   143,     0,
      97,    75,    97,    97,   207,    78,    79,     0,    80,    81,
      82,     0,     0,     0,   214,     0,     0,     0,     0,   169,
       0,   129,     0,   211,     0,     0,   181,     0,     0,   105,
     208,     0,     0,   105,     0,    97,   105,     0,   105,   105,
    1298,     0,    78,    79,     0,    80,    81,    82,     0,     0,
      33,     0,   590,     0,   129,  1309,     0,     0,   129,     0,
     169,   129,     0,   129,   129,     0,     0,     0,     0,     0,
       0,   105,   169,     0,   585,     0,     0,   396,     0,     0,
       0,    97,     0,     0,     0,   181,    97,     0,   585,     0,
     585,     0,   207,     0,   170,     0,   129,     0,     0,     0,
     170,   181,     0,   209,     0,     0,   170,   607,   181,     0,
       0,   169,     0,     0,     0,     0,     0,   105,   208,     0,
     143,     0,   105,    75,     0,   210,     0,    78,    79,     0,
      80,    81,    82,     0,     0,     0,     0,     0,    33,   590,
       0,     0,   129,     0,   590,   211,     0,   129,     0,   212,
       0,   590,     0,   170,     0,     0,     0,   207,     0,     0,
       0,   170,   170,   170,     0,     0,     0,     0,   170,     0,
       0,     0,   214,   214,   170,     0,     0,   214,   181,     0,
       0,     0,   585,   208,     0,     0,   585,     0,   585,     0,
       0,   209,     0,   607,     0,     0,   181,   181,     0,     0,
       0,     0,     0,    33,     0,   181,     0,     0,   143,     0,
       0,    75,   181,   210,     0,    78,    79,     0,    80,    81,
      82,     0,     0,     0,   727,     0,   181,     0,     0,   181,
       0,     0,     0,   211,   590,     0,     0,   212,     0,     0,
     182,   214,     0,     0,   214,     0,     0,     0,   590,     0,
     590,     0,     0,     0,     0,     0,   209,     0,     0,   585,
       0,     0,    31,    32,     0,     0,     0,     0,     0,     0,
     207,     0,    37,   143,     0,   170,    75,     0,   210,     0,
      78,    79,   170,    80,    81,    82,     0,     0,     0,   934,
       0,   181,     0,     0,     0,     0,   208,     0,   211,     0,
       0,     0,   212,     0,   207,     0,     0,     0,   585,     0,
       0,     0,     0,     0,     0,     0,    33,     0,    66,    67,
      68,    69,    70,     0,     0,   589,     0,   585,   585,   582,
     208,     0,   585,   181,     0,    73,    74,   181,   589,     0,
       0,     0,   590,  -245,     0,     0,   590,     0,   590,    84,
      33,   955,   956,   957,   958,   959,   960,     0,     0,   214,
       0,     0,    88,     0,   587,     0,     0,     0,     0,   209,
       0,     0,     0,     0,     0,     0,     0,   587,     0,     0,
       0,     0,     0,     0,     0,     0,   143,     0,     0,    75,
     170,   210,     0,    78,    79,     0,    80,    81,    82,     0,
       0,     0,     0,   209,   171,   171,     0,     0,   183,     0,
       0,   211,     0,   214,     0,   212,     0,     0,     0,   590,
     143,     0,     0,    75,     0,   210,     0,    78,    79,     0,
      80,    81,    82,     0,     0,     0,     0,   585,   170,     0,
       0,     0,     0,     0,     0,   211,     0,     0,     0,   212,
       0,     0,     0,     0,   181,    11,     0,     0,   585,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   590,   585,
     170,     0,   170,     0,   585,     0,     0,     0,     0,     0,
       0,     0,   585,     0,     0,     0,     0,   590,   590,     0,
     170,   589,   590,     0,     0,     0,  1213,     0,     0,     0,
       0,     0,     0,    11,   589,   589,   589,     0,     0,     0,
       0,     0,     0,   953,     0,     0,     0,     0,   954,   585,
     955,   956,   957,   958,   959,   960,   961,     0,   170,     0,
     587,   585,     0,    11,     0,     0,   755,     0,   170,   170,
       0,   214,   214,   587,   587,   587,     0,     0,   171,     0,
       0,   181,   755,     0,   171,     0,   181,     0,     0,     0,
     171,   953,   962,   963,     0,   964,   954,     0,   955,   956,
     957,   958,   959,   960,   961,   589,     0,     0,   785,     0,
       0,     0,     0,    33,     0,    35,     0,     0,   975,     0,
       0,   953,     0,     0,   182,     0,   954,   590,   955,   956,
     957,   958,   959,   960,   961,     0,     0,   171,   214,     0,
     962,   963,     0,   964,   587,   171,   171,   171,   590,     0,
       0,     0,   171,   167,     0,     0,     0,     0,   171,   590,
       0,     0,     0,     0,   590,   170,  1067,     0,     0,     0,
     962,   963,   590,   964,     0,     0,     0,     0,     0,     0,
       0,     0,    11,   143,     0,     0,    75,  1286,    77,     0,
      78,    79,   589,    80,    81,    82,  1068,   589,     0,     0,
       0,   679,   680,     0,   589,     0,     0,     0,     0,   590,
       0,     0,   168,     0,     0,     0,   214,    92,   214,     0,
     681,   590,     0,     0,   183,     0,     0,     0,   682,   683,
      33,   587,   170,     0,     0,     0,   587,     0,   684,     0,
     953,     0,     0,   587,     0,   954,     0,   955,   956,   957,
     958,   959,   960,   961,     0,     0,     0,     0,     0,   171,
       0,     0,     0,     0,     0,     0,   171,     0,     0,     0,
       0,     0,     0,   170,     0,     0,     0,     0,   214,     0,
       0,     0,     0,   685,     0,   170,     0,   589,     0,   962,
     963,     0,   964,     0,     0,   686,     0,     0,     0,     0,
       0,   589,     0,   589,     0,     0,     0,    78,    79,     0,
      80,    81,    82,    11,   947,  1069,     0,     0,     0,     0,
       0,   952,     0,     0,   170,   687,   587,   289,   290,   291,
       0,     0,     0,     0,     0,     0,     0,   214,     0,     0,
     587,     0,   587,   292,     0,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314,     0,     0,
       0,   953,     0,     0,   171,     0,   954,   668,   955,   956,
     957,   958,   959,   960,   961,    33,     0,    35,     0,     0,
       0,     0,     0,     0,     0,   589,     0,     0,     0,   589,
       0,   589,     0,     0,     0,     0,     0,     0,     0,     0,
    1027,     0,     0,     0,     0,     0,     0,     0,  1035,     0,
     962,   963,   171,   964,     0,   167,     0,    33,     0,     0,
     214,     0,     0,     0,   587,     0,     0,     0,   587,     0,
     587,     0,     0,     0,     0,     0,  1198,     0,   214,     0,
       0,     0,     0,     0,   171,   143,   171,     0,    75,     0,
      77,     0,    78,    79,   214,    80,    81,    82,  1028,     0,
       0,     0,   589,     0,   171,     0,     0,     0,   214,     0,
    1029,   214,     0,     0,   168,     0,     0,     0,     0,    92,
       0,     0,     0,     0,     0,   871,     0,   143,     0,     0,
      75,     0,  1030,     0,    78,    79,     0,    80,  1031,    82,
       0,   587,   171,     0,     0,     0,     0,     0,     0,     0,
       0,   589,   171,   171,     0,     0,     0,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,     0,
     589,   589,     0,   214,     0,   589,  1210,     0,     0,     0,
    1035,     0,     0,     0,   146,   148,     0,   150,   151,   152,
     587,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   336,   337,   175,   178,     0,   183,   587,
     587,     0,     0,     0,   587,     0,    33,   195,    35,     0,
       0,     0,     0,     0,   203,     0,   206,     0,     0,   219,
       0,   221,     0,     0,     0,   351,   325,   326,   327,   328,
     329,   330,   331,   332,   333,   334,   335,     0,     0,   171,
       0,     0,     0,     0,     0,   256,   179,     0,     0,     0,
       0,     0,     0,     0,     0,   338,     0,     0,   263,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     589,   336,   337,     0,     0,     0,   143,     0,     0,    75,
       0,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,   589,     0,     0,     0,     0,    33,     0,    35,     0,
       0,     0,   589,     0,     0,   180,   171,   589,     0,   587,
      92,     0,     0,     0,     0,   589,     0,     0,   352,     0,
       0,     0,     0,     0,     0,     0,  1250,     0,     0,     0,
     587,     0,     0,   338,     0,     0,   167,     0,     0,     0,
      33,   587,    35,     0,     0,     0,   587,   171,   423,     0,
       0,     0,   589,     0,   587,     0,     0,     0,   376,   171,
       0,   376,     0,     0,   589,     0,   143,   195,   385,    75,
       0,    77,     0,    78,    79,     0,    80,    81,    82,     0,
     167,     0,     0,     0,  1027,     0,     0,     0,     0,  1318,
       0,   587,   758,     0,     0,   168,     0,     0,   171,     0,
      92,     0,     0,   587,     0,     0,     0,     0,     0,     0,
     143,     0,   175,    75,     0,    77,   432,    78,    79,     0,
      80,    81,    82,     0,  -681,  -681,  -681,  -681,   306,   307,
     308,   309,   310,   311,   312,   313,   458,   314,     0,   168,
       0,     0,     0,     0,    92,     0,     0,   467,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   472,   473,
     474,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,     0,     0,   499,   499,   502,     0,     0,
       0,     0,     0,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   525,   526,     0,     0,     0,     0,     0,
     499,   531,     0,   467,   499,   534,     0,     0,     0,     0,
     515,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     545,     0,   547,     0,     0,     0,     0,     0,   467,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   558,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   597,     0,     0,
     512,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   654,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     141,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,   142,    65,    66,    67,    68,    69,    70,     0,
       0,     0,   256,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,   513,    77,   670,    78,    79,     0,
      80,    81,    82,     0,     0,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,   701,     0,     0,    92,    93,     0,    94,    95,   289,
     290,   291,   195,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   292,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,     0,   314,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     762,   314,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   769,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   777,     0,     0,     0,
       0,     0,     0,     0,   787,     0,     0,   788,     0,   789,
       0,     0,   467,     0,     0,     0,     0,     0,     0,     0,
       0,   467,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     5,     6,     7,     8,     9,     0,     0,     0,
       0,    10,     0,     0,     0,   818,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     174,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,   622,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,   870,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   883,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   467,
     141,     0,     0,    59,    60,     0,     0,   467,     0,   870,
       0,     0,   142,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     143,    73,    74,    75,     0,    77,   195,    78,    79,     0,
      80,    81,    82,     0,   928,    84,     0,     0,     0,    85,
       0,     0,     0,     0,     0,    86,     0,     0,    88,    89,
       0,     0,     0,     0,    92,    93,     0,    94,    95,     0,
       0,     0,     0,     0,     5,     6,     7,     8,     9,     0,
       0,     0,     0,    10,     0,   977,     0,     0,     0,   978,
       0,   979,     0,     0,   467,     0,     0,     0,     0,     0,
       0,     0,     0,   991,     0,     0,     0,     0,     0,     0,
       0,   467,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,     0,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,     0,     0,     0,
      55,    56,    57,     0,    58,    59,    60,    61,    62,    63,
     467,     0,     0,     0,    64,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,    83,     0,    84,     0,     0,
       0,    85,     0,     0,     0,     0,     0,    86,    87,     0,
      88,    89,     0,    90,    91,   763,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,   467,   295,   296,   297,
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
      90,    91,   872,    92,    93,   291,    94,    95,     5,     6,
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
       0,    88,    89,     0,    90,    91,   452,    92,    93,     0,
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
       0,    90,    91,   593,    92,    93,     0,    94,    95,     5,
       6,     7,     8,     9,     0,     0,     0,     0,    10,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,     0,   314,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,     0,     0,     0,    31,    32,    33,    34,    35,
       0,    36,     0,     0,     0,    37,    38,    39,    40,   826,
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
       0,    43,   925,     0,    44,     0,     0,     0,    45,    46,
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
      89,     0,    90,    91,  1087,    92,    93,     0,    94,    95,
       5,     6,     7,     8,     9,     0,     0,     0,     0,    10,
    -681,  -681,  -681,  -681,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,     0,     0,     0,    31,    32,    33,    34,
      35,     0,    36,     0,     0,     0,    37,    38,    39,    40,
       0,    41,     0,    42,  1245,    43,     0,     0,    44,     0,
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
      86,     0,     0,    88,    89,     0,    90,    91,  1275,    92,
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
      88,    89,     0,    90,    91,  1278,    92,    93,     0,    94,
      95,     5,     6,     7,     8,     9,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,     0,     0,     0,    31,    32,    33,
      34,    35,     0,    36,     0,     0,     0,    37,    38,    39,
      40,     0,    41,  1280,    42,     0,    43,     0,     0,    44,
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
       0,    86,     0,     0,    88,    89,     0,    90,    91,  1282,
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
       0,    88,    89,     0,    90,    91,  1283,    92,    93,     0,
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
       0,    90,    91,  1295,    92,    93,     0,    94,    95,     5,
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
    1328,    92,    93,     0,    94,    95,     5,     6,     7,     8,
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
       0,     0,    88,    89,     0,    90,    91,  1331,    92,    93,
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
     560,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   714,     0,     0,
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
       0,     0,     0,     0,  1078,     0,     0,     0,     0,     0,
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
       0,  1201,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,   464,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   475,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,     0,     0,   512,
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
       0,     0,     0,     0,     0,     0,   544,     0,     0,     0,
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
       0,     0,     0,   546,     0,     0,     0,     0,     0,     0,
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
     776,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,     0,     0,     0,     0,   817,     0,     0,
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
       0,     0,     0,     0,   143,    73,    74,    75,   513,    77,
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
     142,    65,    66,    67,    68,    69,    70,     0,  1096,  1097,
    1098,  1099,  1100,    71,  1101,  1102,  1103,  1104,   143,    73,
      74,    75,     0,    77,     0,    78,    79,     0,    80,    81,
      82,     0,     0,    84,     0,   401,     0,    85,     0,     0,
       0,     0,     0,    86,     0,     0,    88,    89,     0,     0,
       0,     0,    92,    93,  1105,    94,    95,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1106,  1107,  1108,
    1109,  1110,  1111,  1112,     0,     0,    33,     0,     0,     0,
       0,     0,     0,     0,     0,  1113,  1114,  1115,  1116,  1117,
    1118,  1119,  1120,  1121,  1122,  1123,  1124,  1125,  1126,  1127,
    1128,  1129,  1130,  1131,  1132,  1133,  1134,  1135,  1136,  1137,
    1138,  1139,  1140,  1141,  1142,  1143,  1144,  1145,  1146,  1147,
    1148,  1149,  1150,  1151,  1152,  1153,     0,     0,  1154,  1155,
    1156,  1157,  1158,  1159,  1160,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1161,  1162,  1163,     0,
    1164,     0,     0,    78,    79,     0,    80,    81,    82,  1165,
       0,  1166,     0,     0,  1167,   289,   290,   291,     0,     0,
       0,     0,  1168,     0,  1169,     0,     0,     0,     0,     0,
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
     642,     0,   289,   290,   291,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   292,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   674,   314,   289,   290,   291,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   292,
       0,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   677,   314,   289,   290,   291,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     292,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   746,   314,   289,   290,   291,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   292,     0,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,     0,   314,     0,   797,     0,   289,
     290,   291,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   292,     0,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   819,   314,
     289,   290,   291,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   292,   929,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   980,
     314,   289,   290,   291,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   292,     0,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
    1055,   314,   289,   290,   291,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   292,     0,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,     0,   314,     0,  1056,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   289,   290,   291,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   292,   930,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,     0,   314,   578,   579,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   403,     0,   580,     0,     0,     0,
       0,     0,     0,     0,    31,    32,    33,     0,     0,     0,
       0,     0,     0,     0,    37,     0,     0,   289,   290,   291,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   292,   536,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314,     0,   581,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,   582,     0,     0,     0,     0,   143,    73,    74,    75,
       0,   583,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,     0,     0,     0,     0,     0,   555,     0,
       0,   584,     0,     0,    88,     0,     0,     0,     0,     0,
       0,     0,     0,   289,   290,   291,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   292,
     361,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,     0,   314,   289,   290,   291,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     292,     0,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,     0,   314,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   289,
     290,   291,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   292,   658,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,     0,   314,
     289,   290,   291,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   292,   696,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,     0,
     314,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   289,   290,   291,     0,
       0,     0,   894,     0,     0,     0,     0,     0,     0,     0,
       0,   564,   292,   655,   293,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,     0,   314,   289,   290,   291,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   292,     0,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,     0,   314
};

static const yytype_int16 yycheck[] =
{
       4,    83,   355,     4,   279,    87,   349,   145,     4,     4,
      30,   747,   181,   781,   324,     4,   314,   130,   949,   537,
      40,    28,   972,   137,    44,   344,   969,   109,   276,   438,
     972,   751,   650,    89,     8,   187,     4,   766,   563,   187,
     569,    26,    46,     8,   220,    49,     8,     8,   784,     8,
     369,     8,    46,     8,     8,     8,     8,   121,   989,    63,
       8,     4,     8,     8,     8,    53,     8,     8,     8,     8,
      29,    73,     8,   207,   208,   188,     8,     8,   212,    83,
       8,    24,    25,    87,    72,    61,     8,    75,     8,    57,
      61,     8,   121,   446,   653,   233,    78,    26,    78,   102,
      73,    73,   147,   144,   121,   109,   665,   171,  1051,   170,
      93,    94,   168,     0,    93,    94,   106,   107,   108,   109,
     110,   111,    41,   170,   180,    98,    41,    90,   173,   747,
     121,   172,   173,    66,    67,    61,   138,   139,   170,  1070,
     162,   146,   276,    31,   864,   174,    89,    61,   170,    61,
     154,   504,   146,    61,   171,   137,   159,   137,    90,   127,
      61,   399,    61,   899,   893,    61,   784,   161,   897,   898,
      61,    61,   170,   141,   412,    61,    61,    61,    61,   117,
     171,   144,   172,   421,   322,    73,    46,   172,   717,   193,
     719,   174,   196,   170,   170,   174,   200,   173,   172,   173,
     282,   377,   173,   175,   193,   319,    61,   196,    61,   174,
     172,   172,   171,   217,   140,   172,   625,   172,   172,   172,
     172,   574,   163,   171,   338,   168,   172,   172,   172,   171,
     950,   174,   172,   172,   222,   171,   795,   180,   157,   321,
     171,   323,   157,   171,   173,   359,   163,   173,   866,   171,
     193,   171,   171,   196,   368,   873,   171,   371,   170,   173,
     394,   173,   266,   251,    73,   173,   121,   996,   121,   612,
     888,   275,   173,    61,   173,   279,    90,   173,   282,    73,
      73,   899,   173,   173,   227,    78,   146,   173,   173,   173,
     173,   140,   235,   236,   237,   813,   399,   449,   266,   242,
     356,   449,   827,    73,   170,   248,    31,   314,    78,   412,
      66,    67,   319,   162,   448,   319,   320,   321,   173,   323,
     173,    31,   173,   266,   173,   117,    13,   565,   576,    31,
     144,   338,   140,   121,   338,  1285,   505,   170,   691,    26,
     578,   579,   580,  1285,   663,   138,   139,   147,    73,    71,
     172,   173,   359,   672,   162,   359,    43,  1300,   170,   528,
     160,   368,   170,    73,   371,   173,   175,   371,   138,   139,
     668,    73,   990,   173,   543,   993,   994,   173,   382,   170,
    1311,   175,   735,   170,   560,   178,  1317,   905,   170,   907,
     542,   170,    71,   382,   542,   170,   548,    26,   402,   144,
     548,    25,   170,   407,  1220,  1221,   349,   172,    71,   179,
      73,   649,    31,   356,   172,    73,   585,   405,    42,   141,
      78,    45,   144,   427,   428,   172,   148,   149,   510,   151,
     152,   153,  1216,  1217,   402,   548,    73,    25,    71,   382,
      73,    78,   576,   577,    45,    46,    47,    61,    49,    71,
     760,    61,   174,  1071,    42,   808,    61,    45,   173,   402,
     140,   170,   565,   317,   170,   144,    98,    61,  1236,   148,
     149,   790,   151,   152,   153,   578,   579,   170,    73,   798,
     138,   139,   144,    78,  1002,   148,   149,   341,   151,   152,
     153,   345,    73,    71,   173,    73,    40,    78,   736,   852,
     137,   138,   139,   741,   857,   162,   510,    24,    25,   643,
     748,    28,   175,   751,   752,   148,   149,    49,   151,   152,
     153,   464,   140,   121,   638,   868,   148,   149,   177,   151,
     152,   153,    49,   537,    73,   539,     8,    71,   714,   559,
     173,   140,   175,   138,   139,    73,   649,   140,   170,   140,
      78,   121,   556,  1289,   170,  1073,   137,   138,   139,   172,
     913,   170,   566,   567,    13,   162,   885,  1303,    13,   512,
     148,   149,   162,   151,   152,   153,    13,    79,   567,    92,
      93,    94,   171,   902,    92,    93,    94,   721,   556,   723,
     594,    13,   172,   831,   171,   173,   171,   175,   137,   138,
     139,   544,   176,   546,   773,   170,   170,   845,   170,   847,
     138,   139,   146,   556,   148,   149,   150,   151,   152,   153,
     624,   564,     8,   624,   567,   171,   864,    82,   624,   624,
     171,   638,     8,    13,   638,   624,   170,   172,   741,    42,
      43,    44,    45,    46,    47,   748,    49,    79,   170,   783,
       8,   168,    93,    94,    95,   170,   624,   174,    73,   602,
     172,   668,   981,   180,   170,   119,   171,    61,    98,   612,
     613,  1289,    71,   120,    73,   679,   680,   681,   953,   109,
     110,   624,  1200,   703,   161,  1303,   122,   117,   118,     8,
     207,   208,   171,    61,     8,   212,   171,    13,   119,   177,
     704,   939,     8,   707,   177,   943,   174,   945,   842,   170,
     227,   170,   950,   177,   171,   704,   172,   177,   235,   236,
     237,   171,   122,     8,   154,   242,   172,   171,   831,   140,
     170,   248,   170,    71,   738,   140,   740,   102,  1057,   707,
     173,   917,   845,    13,   847,   172,   163,   173,    13,   148,
     149,    13,   151,   152,   153,   931,    79,   177,   172,   276,
     764,   704,   279,   764,   707,   173,   709,    13,   764,   764,
       8,   172,   948,  1048,   173,   764,   175,    71,  1016,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,   170,   170,   170,   122,   170,   764,   314,   170,   172,
     820,   935,    41,   141,   171,   809,   144,    13,   146,   813,
     148,   149,     8,   151,   152,   153,   171,   171,   122,   823,
       8,   764,   137,    79,   177,    59,    60,  1065,   832,   170,
    1006,     8,   349,   776,   823,   969,   939,  1013,   159,   356,
     943,    71,   945,    73,   170,   122,  1084,  1085,    31,   983,
     144,  1089,   986,   170,   148,   149,     8,   151,   152,   153,
      99,   171,   173,   171,   832,   104,   173,   106,   107,   108,
     109,   110,   111,   112,   817,   173,   171,   394,   170,   170,
     823,   111,   399,     8,   177,   122,   829,   121,    71,   832,
      73,    26,   137,    79,    68,   412,   172,   171,    71,   172,
     163,   905,  1078,   907,   421,    26,   122,  1045,     8,   148,
     149,   141,   151,  1016,   144,   122,   146,   171,   148,   149,
     171,   151,   152,   153,   174,   868,     8,    71,   111,    73,
      74,   448,   173,     8,   174,   174,   171,   171,   170,    26,
     170,   122,   115,   173,   171,   175,   171,   464,   171,   953,
     106,   107,   108,   109,   110,   111,  1194,     8,   141,   173,
     964,   144,  1065,   146,    26,   148,   149,   171,   151,   152,
     153,   171,   122,   172,   171,   148,   149,  1215,   151,   152,
     153,  1084,  1085,   172,    79,   172,  1089,   170,  1226,    73,
     150,   146,   175,  1231,   170,   512,   171,   170,  1002,   104,
     968,  1239,    73,  1007,   148,   149,  1010,   151,   152,   153,
    1014,    13,   171,   171,   122,  1191,    79,   171,  1022,   122,
    1196,  1010,   171,    71,   122,  1201,   173,   544,    13,   546,
    1034,  1207,  1208,   172,    73,   171,  1040,   170,  1276,  1007,
      13,   173,   399,   173,  1048,   122,  1014,   564,   565,  1187,
    1288,   173,    13,   173,  1022,   412,    13,   171,    73,   576,
     577,   578,   579,   580,  1007,    13,  1242,  1010,    51,  1073,
      73,  1014,  1076,  1077,   173,    73,    63,  1081,   170,  1022,
       8,    13,  1050,   172,    71,   602,    73,   140,  1077,    90,
      90,  1194,   153,   610,    29,   612,   613,    71,   146,    13,
     148,   149,  1184,   151,   152,   153,   170,   172,  1076,   626,
      73,   171,  1215,  1081,   245,   155,   171,   171,   249,    69,
      70,    71,   172,  1226,   111,   170,   643,    73,  1231,   171,
     173,  1297,   649,  1076,  1077,   652,  1239,   268,  1081,   270,
     271,   272,   273,   323,   405,  1321,   127,   667,   320,  1308,
    1232,   668,   321,  1329,   141,   664,   626,   144,   829,   146,
    1094,   148,   149,   410,   151,   152,   153,   141,    71,   928,
     144,  1185,  1323,  1276,   148,   149,  1173,   151,   152,   153,
    1184,   964,    38,   170,    89,  1288,  1023,   736,   175,   796,
      24,    25,   709,   680,    28,   767,  1200,   809,   148,   149,
    1204,   151,   152,   153,   721,    71,   723,  1211,   565,   693,
     888,   243,  1216,  1217,   236,   752,  1220,  1221,    -1,   736,
     724,   578,   579,   580,   741,   986,    -1,    -1,  1232,    -1,
      -1,   748,    -1,  1237,   751,   752,  1204,  1241,   141,    -1,
    1244,   144,  1246,  1247,    25,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,    49,    -1,    -1,    -1,    -1,   776,
      -1,  1204,    -1,   166,    -1,    -1,   783,    -1,    -1,  1237,
      51,    -1,    -1,  1241,    -1,  1279,  1244,    -1,  1246,  1247,
    1284,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
      71,    -1,   649,    -1,  1237,  1299,    -1,    -1,  1241,    -1,
     817,  1244,    -1,  1246,  1247,    -1,    -1,    -1,    -1,    -1,
      -1,  1279,   829,    -1,   831,    -1,    -1,    98,    -1,    -1,
      -1,  1325,    -1,    -1,    -1,   842,  1330,    -1,   845,    -1,
     847,    -1,    25,    -1,   168,    -1,  1279,    -1,    -1,    -1,
     174,   858,    -1,   124,    -1,    -1,   180,   864,   865,    -1,
      -1,   868,    -1,    -1,    -1,    -1,    -1,  1325,    51,    -1,
     141,    -1,  1330,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    71,   736,
      -1,    -1,  1325,    -1,   741,   166,    -1,  1330,    -1,   170,
      -1,   748,    -1,   227,    -1,    -1,    -1,    25,    -1,    -1,
      -1,   235,   236,   237,    -1,    -1,    -1,    -1,   242,    -1,
      -1,    -1,   207,   208,   248,    -1,    -1,   212,   935,    -1,
      -1,    -1,   939,    51,    -1,    -1,   943,    -1,   945,    -1,
      -1,   124,    -1,   950,    -1,    -1,   953,   954,    -1,    -1,
      -1,    -1,    -1,    71,    -1,   962,    -1,    -1,   141,    -1,
      -1,   144,   969,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,   157,    -1,   983,    -1,    -1,   986,
      -1,    -1,    -1,   166,   831,    -1,    -1,   170,    -1,    -1,
     314,   276,    -1,    -1,   279,    -1,    -1,    -1,   845,    -1,
     847,    -1,    -1,    -1,    -1,    -1,   124,    -1,    -1,  1016,
      -1,    -1,    69,    70,    -1,    -1,    -1,    -1,    -1,    -1,
      25,    -1,    79,   141,    -1,   349,   144,    -1,   146,    -1,
     148,   149,   356,   151,   152,   153,    -1,    -1,    -1,   157,
      -1,  1048,    -1,    -1,    -1,    -1,    51,    -1,   166,    -1,
      -1,    -1,   170,    -1,    25,    -1,    -1,    -1,  1065,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,   125,   126,
     127,   128,   129,    -1,    -1,   399,    -1,  1084,  1085,   136,
      51,    -1,  1089,  1090,    -1,   142,   143,  1094,   412,    -1,
      -1,    -1,   939,    98,    -1,    -1,   943,    -1,   945,   156,
      71,   106,   107,   108,   109,   110,   111,    -1,    -1,   394,
      -1,    -1,   169,    -1,   399,    -1,    -1,    -1,    -1,   124,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   412,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,   144,
     464,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,    -1,    -1,   124,    24,    25,    -1,    -1,    28,    -1,
      -1,   166,    -1,   448,    -1,   170,    -1,    -1,    -1,  1016,
     141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,  1194,   512,    -1,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,    -1,   170,
      -1,    -1,    -1,    -1,  1211,    41,    -1,    -1,  1215,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1065,  1226,
     544,    -1,   546,    -1,  1231,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1239,    -1,    -1,    -1,    -1,  1084,  1085,    -1,
     564,   565,  1089,    -1,    -1,    -1,  1093,    -1,    -1,    -1,
      -1,    -1,    -1,    41,   578,   579,   580,    -1,    -1,    -1,
      -1,    -1,    -1,    99,    -1,    -1,    -1,    -1,   104,  1276,
     106,   107,   108,   109,   110,   111,   112,    -1,   602,    -1,
     565,  1288,    -1,    41,    -1,    -1,   610,    -1,   612,   613,
      -1,   576,   577,   578,   579,   580,    -1,    -1,   168,    -1,
      -1,  1308,   626,    -1,   174,    -1,  1313,    -1,    -1,    -1,
     180,    99,   148,   149,    -1,   151,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,   649,    -1,    -1,   652,    -1,
      -1,    -1,    -1,    71,    -1,    73,    -1,    -1,   174,    -1,
      -1,    99,    -1,    -1,   668,    -1,   104,  1194,   106,   107,
     108,   109,   110,   111,   112,    -1,    -1,   227,   643,    -1,
     148,   149,    -1,   151,   649,   235,   236,   237,  1215,    -1,
      -1,    -1,   242,   111,    -1,    -1,    -1,    -1,   248,  1226,
      -1,    -1,    -1,    -1,  1231,   709,   174,    -1,    -1,    -1,
     148,   149,  1239,   151,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,   141,    -1,    -1,   144,  1254,   146,    -1,
     148,   149,   736,   151,   152,   153,   174,   741,    -1,    -1,
      -1,    42,    43,    -1,   748,    -1,    -1,    -1,    -1,  1276,
      -1,    -1,   170,    -1,    -1,    -1,   721,   175,   723,    -1,
      61,  1288,    -1,    -1,   314,    -1,    -1,    -1,    69,    70,
      71,   736,   776,    -1,    -1,    -1,   741,    -1,    79,    -1,
      99,    -1,    -1,   748,    -1,   104,    -1,   106,   107,   108,
     109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,   349,
      -1,    -1,    -1,    -1,    -1,    -1,   356,    -1,    -1,    -1,
      -1,    -1,    -1,   817,    -1,    -1,    -1,    -1,   783,    -1,
      -1,    -1,    -1,   124,    -1,   829,    -1,   831,    -1,   148,
     149,    -1,   151,    -1,    -1,   136,    -1,    -1,    -1,    -1,
      -1,   845,    -1,   847,    -1,    -1,    -1,   148,   149,    -1,
     151,   152,   153,    41,   858,   174,    -1,    -1,    -1,    -1,
      -1,   865,    -1,    -1,   868,   166,   831,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   842,    -1,    -1,
     845,    -1,   847,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    99,    -1,    -1,   464,    -1,   104,    63,   106,   107,
     108,   109,   110,   111,   112,    71,    -1,    73,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   939,    -1,    -1,    -1,   943,
      -1,   945,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     954,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   962,    -1,
     148,   149,   512,   151,    -1,   111,    -1,    71,    -1,    -1,
     935,    -1,    -1,    -1,   939,    -1,    -1,    -1,   943,    -1,
     945,    -1,    -1,    -1,    -1,    -1,   174,    -1,   953,    -1,
      -1,    -1,    -1,    -1,   544,   141,   546,    -1,   144,    -1,
     146,    -1,   148,   149,   969,   151,   152,   153,   112,    -1,
      -1,    -1,  1016,    -1,   564,    -1,    -1,    -1,   983,    -1,
     124,   986,    -1,    -1,   170,    -1,    -1,    -1,    -1,   175,
      -1,    -1,    -1,    -1,    -1,   177,    -1,   141,    -1,    -1,
     144,    -1,   146,    -1,   148,   149,    -1,   151,   152,   153,
      -1,  1016,   602,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1065,   612,   613,    -1,    -1,    -1,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
    1084,  1085,    -1,  1048,    -1,  1089,  1090,    -1,    -1,    -1,
    1094,    -1,    -1,    -1,     5,     6,    -1,     8,     9,    10,
    1065,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    59,    60,    26,    27,    -1,   668,  1084,
    1085,    -1,    -1,    -1,  1089,    -1,    71,    38,    73,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    47,    -1,    -1,    50,
      -1,    52,    -1,    -1,    -1,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    -1,   709,
      -1,    -1,    -1,    -1,    -1,    76,   111,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   121,    -1,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    1194,    59,    60,    -1,    -1,    -1,   141,    -1,    -1,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,  1215,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,
      -1,    -1,  1226,    -1,    -1,   170,   776,  1231,    -1,  1194,
     175,    -1,    -1,    -1,    -1,  1239,    -1,    -1,   149,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1211,    -1,    -1,    -1,
    1215,    -1,    -1,   121,    -1,    -1,   111,    -1,    -1,    -1,
      71,  1226,    73,    -1,    -1,    -1,  1231,   817,   123,    -1,
      -1,    -1,  1276,    -1,  1239,    -1,    -1,    -1,   189,   829,
      -1,   192,    -1,    -1,  1288,    -1,   141,   198,   199,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
     111,    -1,    -1,    -1,  1308,    -1,    -1,    -1,    -1,  1313,
      -1,  1276,   123,    -1,    -1,   170,    -1,    -1,   868,    -1,
     175,    -1,    -1,  1288,    -1,    -1,    -1,    -1,    -1,    -1,
     141,    -1,   243,   144,    -1,   146,   247,   148,   149,    -1,
     151,   152,   153,    -1,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,   267,    49,    -1,   170,
      -1,    -1,    -1,    -1,   175,    -1,    -1,   278,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   289,   290,
     291,   292,   293,   294,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,    -1,    -1,   316,   317,   318,    -1,    -1,
      -1,    -1,    -1,   324,   325,   326,   327,   328,   329,   330,
     331,   332,   333,   334,   335,    -1,    -1,    -1,    -1,    -1,
     341,   342,    -1,   344,   345,   346,    -1,    -1,    -1,    -1,
     351,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     361,    -1,   363,    -1,    -1,    -1,    -1,    -1,   369,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   379,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   408,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   475,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,   513,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,   527,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,   552,    -1,    -1,   175,   176,    -1,   178,   179,     9,
      10,    11,   563,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     621,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   633,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   647,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   655,    -1,    -1,   658,    -1,   660,
      -1,    -1,   663,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   672,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,
      -1,    12,    -1,    -1,    -1,   696,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,   174,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,   760,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   772,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   790,
     111,    -1,    -1,   114,   115,    -1,    -1,   798,    -1,   800,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,   827,   148,   149,    -1,
     151,   152,   153,    -1,   835,   156,    -1,    -1,    -1,   160,
      -1,    -1,    -1,    -1,    -1,   166,    -1,    -1,   169,   170,
      -1,    -1,    -1,    -1,   175,   176,    -1,   178,   179,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,   876,    -1,    -1,    -1,   880,
      -1,   882,    -1,    -1,   885,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   894,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   902,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,    -1,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,   116,   117,   118,
     981,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,   167,    -1,
     169,   170,    -1,   172,   173,   174,   175,   176,    -1,   178,
     179,     3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,
      12,    -1,    -1,    -1,    -1,    -1,  1057,    29,    30,    31,
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
      43,    44,    45,    46,    47,    -1,    49,    42,    43,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   172,    -1,    61,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,    71,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   171,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,    -1,    -1,    -1,   171,    -1,
      -1,   166,    -1,    -1,   169,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
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
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,   122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    90,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49
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
     146,   166,   170,   187,   194,   374,   384,   385,   172,   287,
     173,   287,   144,   188,   189,   190,    73,   175,   248,   249,
     117,   117,    73,   209,   250,   170,   170,   170,   170,   187,
     222,   375,   170,   170,    73,    78,   137,   138,   139,   367,
     368,   144,   173,   190,   190,    95,   287,   223,   375,   146,
     170,   375,   280,   287,   288,   348,   196,   173,    78,   318,
     367,    78,   367,   367,    26,   144,   162,   376,   170,     8,
     172,    31,   208,   146,   221,   375,   172,   172,   172,     9,
      10,    11,    25,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    49,   172,    61,    61,   173,   140,
     118,   154,   209,   224,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    59,    60,   121,   343,
     344,    61,   173,   345,   170,    61,   173,   175,   354,   170,
     208,    13,   287,    40,   187,   338,   170,   280,   348,   140,
     348,   122,   177,     8,   325,   280,   348,   376,   140,   170,
     319,   121,   343,   344,   345,   171,   287,    26,   198,     8,
     172,   198,   199,   282,   283,   287,   187,   236,   202,   172,
     172,   172,   385,   385,   162,   170,    98,   377,   385,    13,
     187,   172,   196,   172,   190,     8,   172,    90,   173,   348,
       8,   172,    13,   208,     8,   172,   348,   371,   371,   348,
     171,   162,   216,   123,   348,   360,   361,    61,   121,   137,
     368,    72,   287,   348,    78,   137,   368,   190,   186,   172,
     173,   172,   219,   304,   306,    79,   291,   293,    13,   171,
     171,   171,   174,   197,   198,   210,   213,   218,   287,   176,
     178,   179,   187,   377,    31,   246,   247,   287,   374,   170,
     375,   214,   287,   287,   287,    26,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   316,   287,
     356,   356,   287,   363,   364,   187,   353,   354,   222,   223,
     208,   221,    31,   145,   284,   287,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   173,   187,   353,
     356,   287,   246,   356,   287,   360,   171,   170,   337,     8,
     325,   280,   171,   187,    31,   287,    31,   287,   171,   171,
     353,   246,   173,   187,   353,   171,   196,   240,   287,    82,
      26,   198,   234,   172,    90,    13,     8,   171,    26,   173,
     237,   385,    79,   381,   382,   383,   170,     8,    42,    43,
      61,   124,   136,   146,   166,   191,   192,   194,   298,   314,
     320,   321,   322,   174,    90,   189,   187,   287,   249,   321,
     170,    73,     8,   171,   171,   171,   172,   191,   192,   380,
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
     385,   321,   321,   321,   324,   327,   170,    78,   140,   170,
     140,   173,   102,   205,   206,   187,   174,   290,    13,   348,
     172,     8,    90,   163,   228,   314,   173,   360,   123,   348,
      13,   177,   287,   174,   183,   228,   173,   307,    13,   287,
      79,   172,   173,   187,   353,   385,    31,   287,   321,   157,
     244,   245,   341,   342,   170,   314,   226,   287,   287,   287,
     170,   246,   227,   226,   212,   225,   316,   174,   170,   246,
      13,    69,    70,   187,   329,   329,   330,   331,   332,   170,
      78,   137,   170,     8,   325,   171,   337,    31,   287,   174,
      66,    67,   243,   281,   198,   172,    83,   172,   348,   122,
     230,    13,   196,   238,    92,    93,    94,   238,   174,   385,
     385,   381,     8,   171,   171,   122,   177,     8,   325,   324,
     187,    79,   292,   294,   187,   321,   365,   366,   170,   159,
     244,   321,   380,   192,   384,     8,   251,   171,   170,   284,
     287,   177,   174,   251,   147,   160,   173,   303,   310,   147,
     173,   309,   122,   287,   376,   170,   348,   171,     8,   326,
     385,   386,   244,   173,   122,   246,   171,   173,   173,   170,
     226,   319,   170,   246,   171,   122,   177,     8,   325,   331,
     137,    79,   334,   335,   330,   348,   281,    26,    68,   198,
     172,   283,   231,   171,   321,    89,    92,   172,   287,    26,
     172,   239,   174,   163,   157,    26,   321,   321,   171,   122,
       8,   325,   171,   122,   174,     8,   325,   314,   173,   171,
       8,   380,   314,    99,   104,   106,   107,   108,   109,   110,
     111,   112,   148,   149,   151,   174,   252,   274,   275,   276,
     277,   279,   341,   360,   174,   174,    46,   287,   287,   287,
     174,   170,   246,    26,   379,   157,   342,    31,    73,   171,
     251,   287,   171,   251,   251,   244,   173,   246,   171,   330,
     330,   171,   122,   171,     8,   325,    26,   196,   172,   171,
     203,   172,   172,   239,   196,   385,   122,   321,    79,   321,
     321,    73,   196,   379,   380,   374,   229,   314,   112,   124,
     146,   152,   261,   262,   263,   314,   150,   267,   268,   115,
     170,   187,   269,   270,   253,   209,   277,   385,     8,   172,
     275,   276,   171,   146,   305,   174,   174,   170,   246,   171,
     385,   104,   301,   386,    73,    13,   379,   174,   174,   174,
     171,   251,   171,   122,   330,    79,   196,   201,    26,   198,
     233,   196,   171,   321,   122,   122,   171,   174,   301,    13,
       8,   172,   173,   173,     8,   172,     3,     4,     5,     6,
       7,     9,    10,    11,    12,    49,    62,    63,    64,    65,
      66,    67,    68,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   123,   124,   125,   126,   127,   128,
     129,   141,   142,   143,   145,   154,   156,   159,   167,   169,
     187,   311,   312,     8,   172,   146,   150,   187,   270,   271,
     272,   172,    73,   278,   208,   254,   374,   209,   246,   171,
     170,   173,    31,    73,    13,   321,   173,   379,   174,   330,
     122,    26,   198,   232,   196,   321,   321,   173,   173,   321,
     314,   257,   264,   320,   262,    13,    26,    43,   265,   268,
       8,    29,   171,    25,    42,    45,    13,     8,   172,   375,
     278,    13,   208,   171,    31,    73,   302,   196,    73,    13,
     321,   196,   173,   330,   196,    87,   196,   196,   174,   187,
     194,   258,   259,   260,     8,   174,   321,   312,   312,    51,
     266,   271,   271,    25,    42,    45,   321,    73,   170,   172,
     321,   375,    73,     8,   326,   174,    13,   321,   174,   196,
      85,   172,   174,   174,   140,    90,   320,   153,    13,   255,
     170,    31,    73,   171,   321,   174,   172,   204,   187,   275,
     276,   321,   244,   256,    73,   102,   205,   207,   155,   187,
     172,   171,   244,   170,   229,   172,   379,   171,   314,   172,
     172,   173,   273,   379,    73,   196,   273,   171,   174,   173,
     196,   174
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
#line 1934 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1945 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1947 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1952 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1955 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1956 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { (yyval).reset();;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval).reset();;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval).reset();;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 1973 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 1974 "hphp.y"
    { (yyval).reset();;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 1980 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 1984 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 1985 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 1989 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2000 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2002 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2005 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2006 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2011 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2019 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2020 "hphp.y"
    { (yyval).reset();;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2025 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2027 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2029 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2030 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2040 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2041 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2046 "hphp.y"
    { validate_shape_keyname((yyvsp[(3) - (5)]), _p);
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2050 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p);
                                         _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2056 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2057 "hphp.y"
    { (yyval).reset();;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2060 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2073 "hphp.y"
    { user_attribute_check(_p);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2075 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2078 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2082 "hphp.y"
    { (yyval).reset();;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2088 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2093 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2097 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2102 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2103 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
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
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2118 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2122 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2124 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
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
#line 2132 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2136 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2139 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2143 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2144 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2151 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2154 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2157 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2160 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2166 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2173 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2177 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2183 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2189 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2190 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2198 "hphp.y"
    { (yyval).reset();;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2203 "hphp.y"
    { (yyval)++;;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2207 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2208 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2213 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2220 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2222 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2227 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2228 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2230 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2232 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2233 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2238 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2239 "hphp.y"
    { (yyval).reset();;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2243 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2244 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2246 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2251 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2252 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2253 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2258 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2259 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2263 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2265 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2279 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2282 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2288 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2289 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2296 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2300 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2305 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2308 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2309 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2310 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2318 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2319 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2320 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2324 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2325 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2339 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2345 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2346 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2352 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]);
                                         only_in_hh_syntax(_p); ;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2364 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2368 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2371 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2376 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2377 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2378 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2379 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2383 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2390 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2395 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (5)]).text()); ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2397 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2405 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2416 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2418 "hphp.y"
    {;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2422 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyval).setText("array"); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2430 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2436 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2437 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2442 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2456 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2460 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2468 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2469 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10593 "new_hphp.tab.cpp"
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

