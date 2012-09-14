
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
#line 1 "../../../src/util/parser/hphp.y"

#include "parser.h"
#include <util/util.h>
#include <util/logger.h>

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

static void no_gap(Parser *_p) {
  if (_p->scanner().hasGap()) {
    HPHP_PARSER_ERROR("XHP: bad spacing", _p);
  }
}

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

static void prepare_continuation_call(Parser* _p, Token& rhs, const char* cname) {
  if (HPHP::hhvm) {
    Token fname;  fname.setText(std::string("hphp_continuation_") + cname);
    Token empty;
    _p->onCall(rhs, false, fname, empty, NULL, true);
  } else {
    Token name;   name.setText(CONTINUATION_OBJECT_NAME);
    Token var;    _p->onSynthesizedVariable(var, name);
    Token pn;     pn.setText(cname);
    Token pname;  _p->onName(pname, pn, Parser::VarName);
    Token empty;  empty = 1;
                  _p->onObjectMethodCall(rhs, var, pname, empty);
  }
}

static void on_yield_assign(Parser *_p, Token &out, Token &var, Token *expr) {
  Token yield;    _p->onYield(yield, expr, true);
  Token rhs;      prepare_continuation_call(_p, rhs, "receive");
  Token assign;   _p->onAssign(assign, var, rhs, 0);
  Token stmt;     _p->onExpStatement(stmt, assign);

  Token stmts0;   _p->onStatementListStart(stmts0);
  Token stmts1;   _p->addStatement(stmts1, stmts0, yield);
  Token stmts2;   _p->addStatement(stmts2, stmts1, stmt);

  _p->finishStatement(out, stmts2); out = 1;
}

static void on_yield_list_assign(Parser *_p, Token &out, Token &var,
                                 Token *expr) {
  Token yield;    _p->onYield(yield, expr, true);
  Token rhs;      prepare_continuation_call(_p, rhs, "receive");
  Token assign;   _p->onListAssignment(assign, var, &rhs);
  Token stmt;     _p->onExpStatement(stmt, assign);

  Token stmts0;   _p->onStatementListStart(stmts0);
  Token stmts1;   _p->addStatement(stmts1, stmts0, yield);
  Token stmts2;   _p->addStatement(stmts2, stmts1, stmt);

  _p->finishStatement(out, stmts2); out = 1;
}

void prepare_generator(Parser *_p, Token &stmt, Token &params, int count) {
  // 1. add prologue and epilogue to original body and store it back to "stmt"
  {
    Token scall;
    Token switchExp;
    {
      // hphp_unpack_continuation(v___cont__)
      Token name;    name.setText(CONTINUATION_OBJECT_NAME);
      Token var;     _p->onSynthesizedVariable(var, name);
      Token param1;  _p->onCallParam(param1, NULL, var, false);

      Token cname;   cname.setText("hphp_unpack_continuation");
      Token call;    _p->onCall(call, false, cname, param1, NULL, true);

      if (HPHP::hhvm) {
        switchExp = call;
      } else {
        _p->onExpStatement(scall, call);
        Token name;    name.setText(CONTINUATION_OBJECT_NAME);
        Token var;     _p->onSynthesizedVariable(var, name);
        Token pn;      pn.setText("getLabel");
        Token pname;   _p->onName(pname, pn, Parser::VarName);
        Token mcall;
        Token empty;   empty = 1;
                       _p->onObjectMethodCall(mcall, var, pname, empty);
        switchExp = mcall;
      }
    }
    Token sswitch;
    {
      _p->pushLabelScope();
      {
        Token cases;
        for (int i = count; i > 0; i--) {
          std::string si = boost::lexical_cast<std::string>(i);

          Token label;   label.setText(YIELD_LABEL_PREFIX + si);
          Token sgoto;   _p->onGoto(sgoto, label, false);
                         _p->addGoto(label.text(), _p->getLocation(), &sgoto);

          Token stmts0;  _p->onStatementListStart(stmts0);
          Token stmts1;  _p->addStatement(stmts1, stmts0, sgoto);
          Token stmts;   _p->finishStatement(stmts, stmts1); stmts = 1;

          Token snum;    snum.setText(si);
          Token num;     _p->onScalar(num, T_LNUMBER, snum);
          Token scase;   _p->onCase(scase, cases, &num, stmts);
          cases = scase;
        }
        _p->onSwitch(sswitch, switchExp, cases);
      }
      _p->popLabelScope();
    }
    Token sdone;
    {
      Token mcall;  prepare_continuation_call(_p, mcall, "done");
      _p->onExpStatement(sdone, mcall);
    }
    {
      Token stmts0;  _p->onStatementListStart(stmts0);
      Token stmts1;  _p->addStatement(stmts1, stmts0, scall);
      Token stmts2;  _p->addStatement(stmts2, stmts1, sswitch);
      Token stmts3;  _p->addStatement(stmts3, stmts2, stmt);
      Token stmts4;  _p->addStatement(stmts4, stmts3, sdone);

      stmt.reset();
      _p->finishStatement(stmt, stmts4); stmt = 1;
    }
  }

  // 2. prepare a single continuation parameter list and store it in "params"
  {
    Token type;    type.setText("Continuation");
    Token var;     var.setText(CONTINUATION_OBJECT_NAME);
    params.reset();
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
    _p->onFunction(out, ret, ref, name, params, scont, attr);
    origGenFunc = out;
  }
}

void transform_yield(Parser *_p, Token &stmts, int index,
                     Token *expr, bool assign) {
  Token update;
  {
    // hphp_pack_continuation(v___cont__, label, value)

    Token name;    name.setText(CONTINUATION_OBJECT_NAME);
    Token var;     _p->onSynthesizedVariable(var, name);
    Token param1;  _p->onCallParam(param1, NULL, var, false);

    Token snum;    snum.setText(boost::lexical_cast<std::string>(index));
    Token num;     _p->onScalar(num, T_LNUMBER, snum);
                   _p->onCallParam(param1, &param1, num, false);

    if (expr) {
      _p->onCallParam(param1, &param1, *expr, false);
    } else {
      Token tnull; scalar_null(_p, tnull);
      _p->onCallParam(param1, &param1, tnull, false);
    }

    Token cname;   cname.setText("hphp_pack_continuation");
    Token call;    _p->onCall(call, false, cname, param1, NULL, true);
    _p->onExpStatement(update, call);
  }

  Token lname;   lname.setText(YIELD_LABEL_PREFIX +
                               boost::lexical_cast<std::string>(index));
  Token label;   _p->onLabel(label, lname);
                 _p->addLabel(lname.text(), _p->getLocation(), &label);

  Token stmts0;  _p->onStatementListStart(stmts0);

  if (!expr) {
    Token mcall;   prepare_continuation_call(_p, mcall, "done");
    Token sdone;   _p->onExpStatement(sdone, mcall);
    Token tmp;     _p->addStatement(tmp, stmts0, sdone);
    stmts0 = tmp;
  }

  Token ret;     _p->onReturn(ret, NULL, false);
  Token stmts1;  _p->addStatement(stmts1, stmts0, update);
  Token stmts2;  _p->addStatement(stmts2, stmts1, ret);
  Token stmts3;  _p->addStatement(stmts3, stmts2, label);

  if (assign) {
    _p->finishStatement(stmts, stmts3); stmts = 1;
  } else {
    Token fcall;  prepare_continuation_call(_p, fcall, "raised");
    Token fstmt;  _p->onExpStatement(fstmt, fcall);
    Token stmts4; _p->addStatement(stmts4, stmts3, fstmt);
    _p->finishStatement(stmts, stmts4); stmts = 1;
  }

}

// convert a foreach (by ref or not) to a normal for statement with
// an iterator object.
void transform_foreach(Parser *_p, Token &out, Token &arr, Token &name,
                       Token &value, Token &stmt, int count,
                       bool hasValue, bool byRef) {
  out.reset();

  std::string loopvar = FOREACH_VAR_PREFIX;
  loopvar += boost::lexical_cast<std::string>(count);

  Token init;
  {
    Token cname;    cname.setText(byRef ?
                                  "hphp_get_mutable_iterator" :
                                  "hphp_get_iterator");
    Token param1;   _p->onCallParam(param1, NULL, arr, 0);
    Token call;     _p->onCall(call, 0, cname, param1, NULL);
    Token lname;    lname.setText(loopvar);
    Token var;      _p->onSynthesizedVariable(var, lname);
    Token assign;   _p->onAssign(assign, var, call, false);

    if (byRef) {
      // hphp_get_mutable_iterator will reset the array's internal pointer.
      _p->onExprListElem(init, NULL, assign);
    } else {
      // We have to reset the iterator's pointer ourselves.
      Token rname;    rname.setText("rewind");
      Token empty;    empty = 1;
      Token rcall;    _p->onObjectMethodCall(rcall, assign, rname, empty);

      _p->onExprListElem(init, NULL, rcall);
    }
  }

  Token cond;
  {
    Token lname;    lname.setText(loopvar);
    Token var;      _p->onSynthesizedVariable(var, lname);
    Token pn;       pn.setText("valid");
    Token pname;    _p->onName(pname, pn, Parser::VarName);
    Token empty;    empty = 1;
    Token valid;    _p->onObjectMethodCall(valid, var, pname, empty);
    _p->onExprListElem(cond, NULL, valid);
  }

  Token step;
  {
    Token lname;    lname.setText(loopvar);
    Token var;      _p->onSynthesizedVariable(var, lname);
    Token pn;       pn.setText("next");
    Token pname;    _p->onName(pname, pn, Parser::VarName);
    Token empty;    empty = 1;
    Token next;     _p->onObjectMethodCall(next, var, pname, empty);
    _p->onExprListElem(step, NULL, next);
  }

  {
    Token stmts0;   _p->onStatementListStart(stmts0);

    if (hasValue) {
      Token skset;
      {
        Token lname;  lname.setText(loopvar);
        Token var;    _p->onSynthesizedVariable(var, lname);
        Token pn;     pn->setText("key");
        Token pname;  _p->onName(pname, pn, Parser::VarName);
        Token empty;  empty = 1;
        Token call;   _p->onObjectMethodCall(call, var, pname, empty);
        Token kset;   _p->onAssign(kset, name, call, false);
        _p->onExpStatement(skset, kset);
      }
      Token stmts1; _p->addStatement(stmts1, stmts0, skset);

      Token svset;
      {
        Token lname;  lname.setText(loopvar);
        Token var;    _p->onSynthesizedVariable(var, lname);
        Token pn;     pn.setText(byRef ? "currentRef" : "current");
        Token pname;  _p->onName(pname, pn, Parser::VarName);
        Token empty;  empty = 1;
        Token call;   _p->onObjectMethodCall(call, var, pname, empty);
        Token vset;   _p->onAssign(vset, value, call, byRef);
        _p->onExpStatement(svset, vset);
      }
      Token stmts2; _p->addStatement(stmts2, stmts1, svset);

      Token stmts3; _p->addStatement(stmts3, stmts2, stmt);
      stmt.reset();
      _p->finishStatement(stmt, stmts3); stmt = 1;
    } else {
      Token svset;
      {
        Token lname;  lname.setText(loopvar);
        Token var;    _p->onSynthesizedVariable(var, lname);
        Token pn;     pn.setText(byRef ? "currentRef" : "current");
        Token pname;  _p->onName(pname, pn, Parser::VarName);
        Token empty;  empty = 1;
        Token call;   _p->onObjectMethodCall(call, var, pname, empty);
        Token vset;   _p->onAssign(vset, name, call, byRef);
        _p->onExpStatement(svset, vset);
      }
      Token stmts1; _p->addStatement(stmts1, stmts0, svset);

      Token stmts2; _p->addStatement(stmts2, stmts1, stmt);
      stmt.reset();
      _p->finishStatement(stmt, stmts2); stmt = 1;
    }
  }

  _p->onFor(out, init, cond, step, stmt);
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

///////////////////////////////////////////////////////////////////////////////

static int yylex(YYSTYPE *token, HPHP::Location *loc, Parser *_p) {
  return _p->scan(token, loc);
}


/* Line 189 of yacc.c  */
#line 950 "hphp.tab.cpp"

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
     T_STRICT_INT_MAP = 394,
     T_STRICT_STR_MAP = 395,
     T_STRICT_ERROR = 396,
     T_FINALLY = 397
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
#line 1147 "hphp.tab.cpp"

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
#define YYLAST   9438

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  172
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  188
/* YYNRULES -- Number of rules.  */
#define YYNRULES  667
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1221

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   397

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   170,     2,   167,    47,    31,   171,
     162,   163,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   164,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   169,    30,     2,   168,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   165,    29,   166,    50,     2,     2,     2,
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
     154,   155,   156,   157,   158,   159,   160,   161
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    11,    13,    15,    17,
      22,    26,    27,    34,    35,    41,    45,    48,    52,    54,
      56,    59,    63,    68,    70,    74,    76,    79,    83,    85,
      88,    92,    98,   103,   106,   107,   109,   111,   113,   115,
     119,   127,   138,   139,   146,   147,   156,   157,   168,   169,
     176,   179,   183,   186,   190,   193,   197,   201,   205,   211,
     220,   224,   228,   232,   238,   240,   242,   243,   253,   259,
     274,   280,   284,   288,   291,   294,   304,   305,   306,   312,
     314,   315,   317,   318,   320,   321,   333,   334,   347,   348,
     349,   359,   360,   361,   372,   373,   381,   382,   391,   392,
     399,   400,   408,   410,   412,   414,   416,   418,   421,   424,
     427,   428,   431,   432,   435,   436,   438,   442,   444,   448,
     451,   452,   454,   457,   459,   464,   466,   471,   473,   478,
     480,   485,   489,   495,   499,   504,   509,   515,   521,   526,
     527,   529,   531,   538,   539,   547,   548,   551,   552,   556,
     557,   561,   563,   565,   566,   570,   575,   582,   588,   594,
     601,   610,   618,   620,   621,   623,   626,   630,   635,   639,
     641,   643,   646,   651,   655,   661,   663,   667,   670,   671,
     672,   677,   678,   684,   687,   688,   699,   700,   712,   713,
     718,   722,   726,   730,   736,   739,   742,   743,   750,   756,
     761,   765,   767,   769,   770,   775,   780,   782,   784,   786,
     788,   790,   792,   794,   799,   801,   803,   807,   810,   811,
     812,   816,   817,   819,   823,   825,   827,   829,   831,   835,
     840,   845,   850,   852,   854,   857,   860,   863,   867,   871,
     873,   875,   877,   879,   883,   885,   887,   889,   890,   892,
     895,   897,   899,   901,   903,   905,   907,   911,   917,   919,
     923,   929,   934,   938,   940,   942,   943,   945,   947,   954,
     958,   963,   970,   974,   977,   981,   985,   989,   993,   997,
    1001,  1005,  1009,  1013,  1017,  1021,  1024,  1027,  1030,  1033,
    1037,  1041,  1045,  1049,  1053,  1057,  1061,  1065,  1069,  1073,
    1077,  1081,  1085,  1089,  1093,  1097,  1100,  1103,  1106,  1109,
    1113,  1117,  1121,  1125,  1129,  1133,  1137,  1141,  1145,  1149,
    1155,  1160,  1162,  1165,  1168,  1171,  1174,  1177,  1180,  1183,
    1186,  1189,  1191,  1193,  1197,  1200,  1201,  1213,  1215,  1217,
    1222,  1227,  1232,  1234,  1236,  1240,  1245,  1246,  1250,  1255,
    1257,  1260,  1265,  1268,  1269,  1270,  1279,  1280,  1282,  1283,
    1289,  1290,  1293,  1294,  1296,  1298,  1302,  1304,  1308,  1310,
    1312,  1313,  1318,  1319,  1324,  1326,  1327,  1332,  1333,  1338,
    1340,  1342,  1344,  1346,  1348,  1350,  1352,  1354,  1356,  1358,
    1360,  1362,  1364,  1366,  1368,  1370,  1372,  1374,  1376,  1378,
    1380,  1382,  1384,  1386,  1388,  1390,  1392,  1394,  1396,  1398,
    1400,  1402,  1404,  1406,  1408,  1410,  1412,  1414,  1416,  1418,
    1420,  1422,  1424,  1426,  1428,  1430,  1432,  1434,  1436,  1438,
    1440,  1442,  1444,  1446,  1448,  1450,  1452,  1454,  1456,  1458,
    1460,  1462,  1464,  1466,  1468,  1470,  1472,  1474,  1476,  1478,
    1480,  1482,  1484,  1489,  1491,  1493,  1495,  1498,  1500,  1502,
    1504,  1506,  1508,  1510,  1512,  1515,  1519,  1522,  1526,  1527,
    1528,  1530,  1532,  1536,  1537,  1539,  1541,  1543,  1545,  1547,
    1549,  1551,  1553,  1555,  1557,  1559,  1563,  1566,  1568,  1570,
    1573,  1576,  1581,  1583,  1587,  1591,  1593,  1595,  1597,  1599,
    1603,  1607,  1611,  1614,  1615,  1617,  1618,  1624,  1628,  1632,
    1634,  1636,  1638,  1640,  1644,  1647,  1649,  1651,  1653,  1655,
    1657,  1660,  1663,  1668,  1672,  1675,  1676,  1682,  1686,  1690,
    1692,  1696,  1698,  1701,  1702,  1706,  1707,  1712,  1715,  1716,
    1720,  1724,  1726,  1727,  1729,  1731,  1733,  1735,  1740,  1745,
    1749,  1753,  1759,  1763,  1768,  1772,  1774,  1776,  1778,  1783,
    1788,  1792,  1798,  1803,  1807,  1809,  1814,  1819,  1823,  1830,
    1837,  1846,  1853,  1860,  1862,  1865,  1870,  1875,  1877,  1879,
    1884,  1886,  1887,  1889,  1892,  1894,  1899,  1904,  1908,  1912,
    1918,  1922,  1926,  1931,  1936,  1940,  1946,  1950,  1953,  1957,
    1964,  1965,  1967,  1972,  1975,  1976,  1982,  1986,  1990,  1992,
    1999,  2004,  2009,  2012,  2015,  2018,  2020,  2023,  2025,  2030,
    2034,  2038,  2045,  2049,  2051,  2053,  2055,  2060,  2065,  2068,
    2071,  2076,  2079,  2082,  2084,  2088,  2092,  2094,  2097,  2099,
    2104,  2107,  2108,  2113,  2116,  2120,  2122,  2126,  2127,  2130,
    2134,  2136,  2142,  2146,  2149,  2152,  2155,  2157,  2159,  2168,
    2175,  2181,  2183,  2185,  2187,  2189,  2191,  2193
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     173,     0,    -1,   174,    -1,   174,   175,    -1,    -1,   186,
      -1,   198,    -1,   201,    -1,   208,    -1,   116,   162,   163,
     164,    -1,   141,   180,   164,    -1,    -1,   141,   180,   165,
     176,   174,   166,    -1,    -1,   141,   165,   177,   174,   166,
      -1,   104,   178,   164,    -1,   183,   164,    -1,   178,     8,
     179,    -1,   179,    -1,   180,    -1,   144,   180,    -1,   180,
      90,    71,    -1,   144,   180,    90,    71,    -1,    71,    -1,
     180,   144,    71,    -1,   180,    -1,   144,   180,    -1,   141,
     144,   180,    -1,   180,    -1,   144,   180,    -1,   141,   144,
     180,    -1,   183,     8,   350,    13,   310,    -1,    99,   350,
      13,   310,    -1,   184,   185,    -1,    -1,   186,    -1,   198,
      -1,   201,    -1,   208,    -1,   165,   184,   166,    -1,    65,
     162,   275,   163,   186,   230,   232,    -1,    65,   162,   275,
     163,    26,   184,   231,   233,    68,   164,    -1,    -1,    82,
     162,   275,   163,   187,   224,    -1,    -1,    81,   188,   186,
      82,   162,   275,   163,   164,    -1,    -1,    84,   162,   274,
     164,   274,   164,   274,   163,   189,   222,    -1,    -1,    91,
     162,   275,   163,   190,   227,    -1,    95,   164,    -1,    95,
     275,   164,    -1,    97,   164,    -1,    97,   275,   164,    -1,
     100,   164,    -1,   100,   275,   164,    -1,   145,    95,   164,
      -1,   145,   275,   164,    -1,   329,    13,   145,   275,   164,
      -1,   123,   162,   341,   163,    13,   145,   275,   164,    -1,
     105,   238,   164,    -1,   111,   240,   164,    -1,    80,   273,
     164,    -1,   113,   162,   348,   163,   164,    -1,   164,    -1,
      75,    -1,    -1,    86,   162,   275,    90,   221,   220,   163,
     191,   223,    -1,    88,   162,   226,   163,   225,    -1,   101,
     165,   184,   166,   102,   162,   303,    73,   163,   165,   184,
     166,   192,   195,    -1,   101,   165,   184,   166,   193,    -1,
     103,   275,   164,    -1,    96,    71,   164,    -1,   275,   164,
      -1,    71,    26,    -1,   192,   102,   162,   303,    73,   163,
     165,   184,   166,    -1,    -1,    -1,   194,   161,   165,   184,
     166,    -1,   193,    -1,    -1,    31,    -1,    -1,    98,    -1,
      -1,   197,   196,   351,   199,   162,   234,   163,   355,   165,
     184,   166,    -1,    -1,   327,   197,   196,   351,   200,   162,
     234,   163,   355,   165,   184,   166,    -1,    -1,    -1,   214,
     211,   202,   215,   216,   165,   203,   241,   166,    -1,    -1,
      -1,   327,   214,   211,   204,   215,   216,   165,   205,   241,
     166,    -1,    -1,   118,   212,   206,   217,   165,   241,   166,
      -1,    -1,   327,   118,   212,   207,   217,   165,   241,   166,
      -1,    -1,   154,   213,   209,   165,   241,   166,    -1,    -1,
     327,   154,   213,   210,   165,   241,   166,    -1,   351,    -1,
     146,    -1,   351,    -1,   351,    -1,   117,    -1,   110,   117,
      -1,   109,   117,    -1,   119,   303,    -1,    -1,   120,   218,
      -1,    -1,   119,   218,    -1,    -1,   303,    -1,   218,     8,
     303,    -1,   303,    -1,   219,     8,   303,    -1,   122,   221,
      -1,    -1,   329,    -1,    31,   329,    -1,   186,    -1,    26,
     184,    85,   164,    -1,   186,    -1,    26,   184,    87,   164,
      -1,   186,    -1,    26,   184,    83,   164,    -1,   186,    -1,
      26,   184,    89,   164,    -1,    71,    13,   310,    -1,   226,
       8,    71,    13,   310,    -1,   165,   228,   166,    -1,   165,
     164,   228,   166,    -1,    26,   228,    92,   164,    -1,    26,
     164,   228,    92,   164,    -1,   228,    93,   275,   229,   184,
      -1,   228,    94,   229,   184,    -1,    -1,    26,    -1,   164,
      -1,   230,    66,   162,   275,   163,   186,    -1,    -1,   231,
      66,   162,   275,   163,    26,   184,    -1,    -1,    67,   186,
      -1,    -1,    67,    26,   184,    -1,    -1,   235,     8,   157,
      -1,   235,    -1,   157,    -1,    -1,   328,   359,    73,    -1,
     328,   359,    31,    73,    -1,   328,   359,    31,    73,    13,
     310,    -1,   328,   359,    73,    13,   310,    -1,   235,     8,
     328,   359,    73,    -1,   235,     8,   328,   359,    31,    73,
      -1,   235,     8,   328,   359,    31,    73,    13,   310,    -1,
     235,     8,   328,   359,    73,    13,   310,    -1,   237,    -1,
      -1,   275,    -1,    31,   329,    -1,   237,     8,   275,    -1,
     237,     8,    31,   329,    -1,   238,     8,   239,    -1,   239,
      -1,    73,    -1,   167,   329,    -1,   167,   165,   275,   166,
      -1,   240,     8,    73,    -1,   240,     8,    73,    13,   310,
      -1,    73,    -1,    73,    13,   310,    -1,   241,   242,    -1,
      -1,    -1,   267,   243,   271,   164,    -1,    -1,   269,   357,
     244,   271,   164,    -1,   272,   164,    -1,    -1,   268,   197,
     196,   351,   162,   245,   234,   163,   355,   266,    -1,    -1,
     327,   268,   197,   196,   351,   162,   246,   234,   163,   355,
     266,    -1,    -1,   148,   247,   252,   164,    -1,   149,   260,
     164,    -1,   151,   262,   164,    -1,   104,   219,   164,    -1,
     104,   219,   165,   248,   166,    -1,   248,   249,    -1,   248,
     250,    -1,    -1,   182,   140,    71,   155,   219,   164,    -1,
     251,    90,   268,    71,   164,    -1,   251,    90,   269,   164,
      -1,   182,   140,    71,    -1,    71,    -1,   254,    -1,    -1,
     252,     8,   253,   254,    -1,   255,   297,   257,   258,    -1,
     146,    -1,    56,    -1,    53,    -1,    58,    -1,    55,    -1,
     303,    -1,   112,    -1,   152,   165,   256,   166,    -1,    57,
      -1,   309,    -1,   256,     8,   309,    -1,    13,   310,    -1,
      -1,    -1,    51,   259,   153,    -1,    -1,   261,    -1,   260,
       8,   261,    -1,   150,    -1,   263,    -1,    71,    -1,   115,
      -1,   162,   264,   163,    -1,   162,   264,   163,    45,    -1,
     162,   264,   163,    25,    -1,   162,   264,   163,    42,    -1,
     263,    -1,   265,    -1,   265,    45,    -1,   265,    25,    -1,
     265,    42,    -1,   264,     8,   264,    -1,   264,    29,   264,
      -1,    71,    -1,   146,    -1,   150,    -1,   164,    -1,   165,
     184,   166,    -1,   269,    -1,   112,    -1,   269,    -1,    -1,
     270,    -1,   269,   270,    -1,   106,    -1,   107,    -1,   108,
      -1,   111,    -1,   110,    -1,   109,    -1,   271,     8,    73,
      -1,   271,     8,    73,    13,   310,    -1,    73,    -1,    73,
      13,   310,    -1,   272,     8,   350,    13,   310,    -1,    99,
     350,    13,   310,    -1,   273,     8,   275,    -1,   275,    -1,
     273,    -1,    -1,   276,    -1,   329,    -1,   123,   162,   341,
     163,    13,   275,    -1,   329,    13,   275,    -1,   329,    13,
      31,   329,    -1,   329,    13,    31,    63,   305,   308,    -1,
      63,   305,   308,    -1,    62,   275,    -1,   329,    24,   275,
      -1,   329,    23,   275,    -1,   329,    22,   275,    -1,   329,
      21,   275,    -1,   329,    20,   275,    -1,   329,    19,   275,
      -1,   329,    18,   275,    -1,   329,    17,   275,    -1,   329,
      16,   275,    -1,   329,    15,   275,    -1,   329,    14,   275,
      -1,   329,    60,    -1,    60,   329,    -1,   329,    59,    -1,
      59,   329,    -1,   275,    27,   275,    -1,   275,    28,   275,
      -1,   275,     9,   275,    -1,   275,    11,   275,    -1,   275,
      10,   275,    -1,   275,    29,   275,    -1,   275,    31,   275,
      -1,   275,    30,   275,    -1,   275,    44,   275,    -1,   275,
      42,   275,    -1,   275,    43,   275,    -1,   275,    45,   275,
      -1,   275,    46,   275,    -1,   275,    47,   275,    -1,   275,
      41,   275,    -1,   275,    40,   275,    -1,    42,   275,    -1,
      43,   275,    -1,    48,   275,    -1,    50,   275,    -1,   275,
      33,   275,    -1,   275,    32,   275,    -1,   275,    35,   275,
      -1,   275,    34,   275,    -1,   275,    36,   275,    -1,   275,
      39,   275,    -1,   275,    37,   275,    -1,   275,    38,   275,
      -1,   275,    49,   305,    -1,   162,   276,   163,    -1,   275,
      25,   275,    26,   275,    -1,   275,    25,    26,   275,    -1,
     347,    -1,    58,   275,    -1,    57,   275,    -1,    56,   275,
      -1,    55,   275,    -1,    54,   275,    -1,    53,   275,    -1,
      52,   275,    -1,    64,   306,    -1,    51,   275,    -1,   312,
      -1,   278,    -1,   168,   307,   168,    -1,    12,   275,    -1,
      -1,   197,   196,   162,   277,   234,   163,   355,   281,   165,
     184,   166,    -1,   283,    -1,   279,    -1,   124,   162,   342,
     163,    -1,   279,    61,   337,   169,    -1,   280,    61,   337,
     169,    -1,   278,    -1,   349,    -1,   162,   276,   163,    -1,
     104,   162,   282,   163,    -1,    -1,   282,     8,    73,    -1,
     282,     8,    31,    73,    -1,    73,    -1,    31,    73,    -1,
      36,   294,   284,    37,    -1,   288,    46,    -1,    -1,    -1,
     288,    37,   285,   290,    36,    46,   286,   287,    -1,    -1,
     146,    -1,    -1,   288,   291,    13,   289,   292,    -1,    -1,
     290,   293,    -1,    -1,   297,    -1,   147,    -1,   165,   275,
     166,    -1,   147,    -1,   165,   275,   166,    -1,   283,    -1,
     300,    -1,    -1,   294,    26,   295,   300,    -1,    -1,   294,
      43,   296,   300,    -1,   300,    -1,    -1,   297,    26,   298,
     300,    -1,    -1,   297,    43,   299,   300,    -1,    71,    -1,
      64,    -1,    98,    -1,    99,    -1,   100,    -1,   145,    -1,
     101,    -1,   102,    -1,   161,    -1,   103,    -1,    65,    -1,
      66,    -1,    68,    -1,    67,    -1,    82,    -1,    83,    -1,
      81,    -1,    84,    -1,    85,    -1,    86,    -1,    87,    -1,
      88,    -1,    89,    -1,    49,    -1,    90,    -1,    91,    -1,
      92,    -1,    93,    -1,    94,    -1,    95,    -1,    97,    -1,
      96,    -1,    80,    -1,    12,    -1,   117,    -1,   118,    -1,
     119,    -1,   120,    -1,    63,    -1,    62,    -1,   112,    -1,
       5,    -1,     7,    -1,     6,    -1,     4,    -1,     3,    -1,
     141,    -1,   104,    -1,   105,    -1,   114,    -1,   115,    -1,
     116,    -1,   111,    -1,   110,    -1,   109,    -1,   108,    -1,
     107,    -1,   106,    -1,   113,    -1,   123,    -1,   124,    -1,
       9,    -1,    11,    -1,    10,    -1,   125,    -1,   127,    -1,
     126,    -1,   128,    -1,   129,    -1,   143,    -1,   142,    -1,
     154,    -1,   156,    -1,   181,   162,   236,   163,    -1,   304,
      -1,   111,    -1,   335,    -1,   182,   352,    -1,   146,    -1,
     182,    -1,   146,    -1,   339,    -1,   111,    -1,   146,    -1,
     180,    -1,   144,   180,    -1,   141,   144,   180,    -1,   162,
     163,    -1,   162,   275,   163,    -1,    -1,    -1,    78,    -1,
     344,    -1,   162,   236,   163,    -1,    -1,    69,    -1,    70,
      -1,    79,    -1,   128,    -1,   129,    -1,   143,    -1,   125,
      -1,   156,    -1,   126,    -1,   127,    -1,   142,    -1,   136,
      78,   137,    -1,   136,   137,    -1,   309,    -1,   181,    -1,
      42,   310,    -1,    43,   310,    -1,   124,   162,   313,   163,
      -1,   311,    -1,   182,   140,    71,    -1,   146,   140,    71,
      -1,   181,    -1,    72,    -1,   349,    -1,   309,    -1,   170,
     344,   170,    -1,   171,   344,   171,    -1,   136,   344,   137,
      -1,   315,   314,    -1,    -1,     8,    -1,    -1,   315,     8,
     310,   122,   310,    -1,   315,     8,   310,    -1,   310,   122,
     310,    -1,   310,    -1,    69,    -1,    70,    -1,    79,    -1,
     136,    78,   137,    -1,   136,   137,    -1,    69,    -1,    70,
      -1,    71,    -1,   316,    -1,    71,    -1,    42,   317,    -1,
      43,   317,    -1,   124,   162,   319,   163,    -1,    61,   319,
     169,    -1,   320,   314,    -1,    -1,   320,     8,   318,   122,
     318,    -1,   320,     8,   318,    -1,   318,   122,   318,    -1,
     318,    -1,   321,     8,   318,    -1,   318,    -1,   321,   314,
      -1,    -1,   162,   322,   163,    -1,    -1,   324,     8,    71,
     323,    -1,    71,   323,    -1,    -1,   326,   324,   314,    -1,
      41,   325,    40,    -1,   327,    -1,    -1,   334,    -1,   301,
      -1,   332,    -1,   333,    -1,   330,    61,   337,   169,    -1,
     330,   165,   275,   166,    -1,   329,   121,    71,    -1,   329,
     121,   334,    -1,   329,   121,   165,   275,   166,    -1,   302,
     140,   334,    -1,   331,   162,   236,   163,    -1,   162,   329,
     163,    -1,   301,    -1,   332,    -1,   333,    -1,   330,    61,
     337,   169,    -1,   330,   165,   275,   166,    -1,   329,   121,
      71,    -1,   329,   121,   165,   275,   166,    -1,   331,   162,
     236,   163,    -1,   162,   329,   163,    -1,   334,    -1,   330,
      61,   337,   169,    -1,   330,   165,   275,   166,    -1,   162,
     329,   163,    -1,   329,   121,    71,   162,   236,   163,    -1,
     329,   121,   334,   162,   236,   163,    -1,   329,   121,   165,
     275,   166,   162,   236,   163,    -1,   302,   140,    71,   162,
     236,   163,    -1,   302,   140,   334,   162,   236,   163,    -1,
     335,    -1,   338,   335,    -1,   335,    61,   337,   169,    -1,
     335,   165,   275,   166,    -1,   336,    -1,    73,    -1,   167,
     165,   275,   166,    -1,   275,    -1,    -1,   167,    -1,   338,
     167,    -1,   334,    -1,   340,    61,   337,   169,    -1,   340,
     165,   275,   166,    -1,   339,   121,    71,    -1,   339,   121,
     334,    -1,   339,   121,   165,   275,   166,    -1,   302,   140,
     334,    -1,   162,   329,   163,    -1,   340,    61,   337,   169,
      -1,   340,   165,   275,   166,    -1,   339,   121,    71,    -1,
     339,   121,   165,   275,   166,    -1,   162,   329,   163,    -1,
     341,     8,    -1,   341,     8,   329,    -1,   341,     8,   123,
     162,   341,   163,    -1,    -1,   329,    -1,   123,   162,   341,
     163,    -1,   343,   314,    -1,    -1,   343,     8,   275,   122,
     275,    -1,   343,     8,   275,    -1,   275,   122,   275,    -1,
     275,    -1,   343,     8,   275,   122,    31,   329,    -1,   343,
       8,    31,   329,    -1,   275,   122,    31,   329,    -1,    31,
     329,    -1,   344,   345,    -1,   344,    78,    -1,   345,    -1,
      78,   345,    -1,    73,    -1,    73,    61,   346,   169,    -1,
      73,   121,    71,    -1,   138,   275,   166,    -1,   138,    72,
      61,   275,   169,   166,    -1,   139,   329,   166,    -1,    71,
      -1,    74,    -1,    73,    -1,   114,   162,   348,   163,    -1,
     115,   162,   329,   163,    -1,     7,   275,    -1,     6,   275,
      -1,     5,   162,   275,   163,    -1,     4,   275,    -1,     3,
     275,    -1,   329,    -1,   348,     8,   329,    -1,   302,   140,
      71,    -1,    71,    -1,   357,    71,    -1,    71,    -1,    71,
      36,   356,    37,    -1,    36,   353,    -1,    -1,    71,    36,
     354,    40,    -1,   357,    37,    -1,   357,     8,   353,    -1,
     357,    -1,   357,     8,   354,    -1,    -1,    26,   357,    -1,
      71,     8,   356,    -1,    71,    -1,    71,    90,    71,     8,
     356,    -1,    71,    90,    71,    -1,    25,   357,    -1,    51,
     357,    -1,    71,   352,    -1,   124,    -1,   146,    -1,   162,
      98,   162,   354,   163,    26,   357,   163,    -1,   162,    98,
     358,    26,   357,   163,    -1,   162,   357,     8,   354,   163,
      -1,    53,    -1,    58,    -1,    57,    -1,    55,    -1,    56,
      -1,   357,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   998,   998,  1003,  1005,  1008,  1010,  1011,  1012,  1013,
    1014,  1016,  1016,  1018,  1018,  1020,  1021,  1026,  1028,  1031,
    1032,  1033,  1034,  1038,  1039,  1043,  1045,  1046,  1050,  1052,
    1053,  1057,  1060,  1065,  1067,  1070,  1071,  1072,  1073,  1076,
    1077,  1081,  1086,  1086,  1090,  1090,  1094,  1093,  1097,  1097,
    1100,  1101,  1102,  1103,  1104,  1105,  1106,  1107,  1108,  1109,
    1111,  1112,  1113,  1114,  1115,  1116,  1119,  1117,  1123,  1125,
    1133,  1136,  1137,  1141,  1142,  1149,  1155,  1159,  1159,  1165,
    1166,  1170,  1171,  1175,  1180,  1179,  1190,  1189,  1203,  1205,
    1202,  1223,  1225,  1221,  1242,  1241,  1250,  1248,  1260,  1259,
    1270,  1268,  1280,  1281,  1285,  1288,  1291,  1292,  1293,  1296,
    1298,  1301,  1302,  1305,  1306,  1309,  1310,  1314,  1315,  1320,
    1321,  1324,  1325,  1329,  1330,  1334,  1335,  1339,  1340,  1344,
    1345,  1350,  1351,  1356,  1357,  1358,  1359,  1362,  1365,  1367,
    1370,  1371,  1375,  1377,  1380,  1383,  1386,  1387,  1390,  1391,
    1395,  1397,  1398,  1399,  1403,  1405,  1407,  1410,  1413,  1416,
    1419,  1423,  1430,  1431,  1434,  1435,  1436,  1438,  1443,  1444,
    1447,  1448,  1449,  1453,  1454,  1456,  1457,  1461,  1465,  1468,
    1468,  1472,  1471,  1475,  1479,  1477,  1491,  1488,  1500,  1500,
    1502,  1504,  1506,  1508,  1512,  1513,  1514,  1517,  1523,  1526,
    1532,  1535,  1539,  1541,  1541,  1546,  1551,  1555,  1556,  1557,
    1558,  1559,  1560,  1561,  1563,  1567,  1568,  1573,  1574,  1578,
    1578,  1580,  1584,  1586,  1592,  1597,  1598,  1600,  1604,  1605,
    1606,  1607,  1611,  1612,  1613,  1614,  1615,  1616,  1618,  1623,
    1626,  1627,  1631,  1632,  1635,  1636,  1639,  1640,  1643,  1644,
    1648,  1649,  1650,  1651,  1652,  1653,  1656,  1658,  1660,  1661,
    1664,  1666,  1670,  1671,  1675,  1676,  1680,  1681,  1684,  1686,
    1687,  1688,  1691,  1693,  1694,  1695,  1696,  1697,  1698,  1699,
    1700,  1701,  1702,  1703,  1704,  1705,  1706,  1707,  1708,  1709,
    1710,  1711,  1712,  1713,  1714,  1715,  1716,  1717,  1718,  1719,
    1720,  1721,  1722,  1723,  1724,  1725,  1726,  1727,  1728,  1729,
    1730,  1731,  1732,  1733,  1734,  1736,  1737,  1739,  1741,  1742,
    1743,  1744,  1745,  1746,  1747,  1748,  1749,  1750,  1751,  1752,
    1753,  1754,  1755,  1756,  1757,  1758,  1758,  1765,  1766,  1770,
    1774,  1776,  1781,  1782,  1783,  1787,  1788,  1792,  1793,  1794,
    1795,  1799,  1802,  1808,  1809,  1808,  1817,  1818,  1822,  1821,
    1824,  1827,  1828,  1831,  1835,  1838,  1841,  1848,  1849,  1852,
    1853,  1853,  1855,  1855,  1859,  1860,  1860,  1862,  1862,  1866,
    1867,  1868,  1869,  1870,  1871,  1872,  1873,  1874,  1875,  1876,
    1877,  1878,  1879,  1880,  1881,  1882,  1883,  1884,  1885,  1886,
    1887,  1888,  1889,  1890,  1891,  1892,  1893,  1894,  1895,  1896,
    1897,  1898,  1899,  1900,  1901,  1902,  1903,  1904,  1905,  1906,
    1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,  1915,  1916,
    1917,  1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,  1926,
    1927,  1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,  1936,
    1937,  1938,  1942,  1947,  1949,  1950,  1954,  1955,  1958,  1959,
    1962,  1963,  1964,  1966,  1968,  1969,  1975,  1976,  1977,  1981,
    1982,  1983,  1986,  1988,  1992,  1993,  1994,  1996,  1997,  1998,
    1999,  2000,  2001,  2002,  2003,  2004,  2007,  2012,  2013,  2014,
    2015,  2016,  2018,  2021,  2024,  2029,  2030,  2031,  2032,  2033,
    2034,  2035,  2040,  2042,  2045,  2046,  2049,  2052,  2054,  2056,
    2060,  2061,  2062,  2064,  2067,  2071,  2072,  2073,  2076,  2077,
    2078,  2079,  2080,  2082,  2085,  2087,  2090,  2093,  2095,  2097,
    2100,  2102,  2105,  2107,  2110,  2111,  2115,  2118,  2122,  2122,
    2127,  2130,  2131,  2134,  2135,  2136,  2137,  2138,  2140,  2141,
    2143,  2145,  2147,  2150,  2152,  2156,  2157,  2158,  2159,  2161,
    2162,  2164,  2166,  2168,  2172,  2173,  2175,  2176,  2180,  2183,
    2186,  2192,  2196,  2203,  2204,  2209,  2211,  2212,  2215,  2216,
    2219,  2220,  2224,  2225,  2229,  2230,  2232,  2234,  2236,  2238,
    2240,  2243,  2247,  2249,  2251,  2253,  2255,  2259,  2260,  2261,
    2263,  2264,  2265,  2269,  2271,  2274,  2276,  2277,  2278,  2279,
    2282,  2284,  2285,  2289,  2290,  2292,  2293,  2299,  2300,  2302,
    2304,  2306,  2308,  2311,  2312,  2313,  2317,  2318,  2319,  2320,
    2321,  2322,  2323,  2327,  2328,  2332,  2341,  2342,  2348,  2349,
    2354,  2355,  2360,  2361,  2362,  2366,  2367,  2371,  2372,  2376,
    2377,  2378,  2380,  2388,  2389,  2390,  2401,  2402,  2403,  2405,
    2407,  2412,  2413,  2414,  2415,  2416,  2419,  2420
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
  "T_TRAIT", "T_INSTEADOF", "T_TRAIT_C", "T_VARARG", "T_STRICT_INT_MAP",
  "T_STRICT_STR_MAP", "T_STRICT_ERROR", "T_FINALLY", "'('", "')'", "';'",
  "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept", "start",
  "top_statement_list", "top_statement", "$@1", "$@2", "use_declarations",
  "use_declaration", "namespace_name", "namespace_string",
  "class_namespace_string", "constant_declaration", "inner_statement_list",
  "inner_statement", "statement", "$@3", "$@4", "$@5", "$@6", "$@7",
  "additional_catches", "finally", "$@8", "optional_finally",
  "is_reference", "function_loc", "function_declaration_statement", "$@9",
  "$@10", "class_declaration_statement", "$@11", "$@12", "$@13", "$@14",
  "$@15", "$@16", "trait_declaration_statement", "$@17", "$@18",
  "class_decl_name", "interface_decl_name", "trait_decl_name",
  "class_entry_type", "extends_from", "implements_list",
  "interface_extends_list", "interface_list", "trait_list",
  "foreach_optional_arg", "foreach_variable", "for_statement",
  "foreach_statement", "while_statement", "declare_statement",
  "declare_list", "switch_case_list", "case_list", "case_separator",
  "elseif_list", "new_elseif_list", "else_single", "new_else_single",
  "parameter_list", "non_empty_parameter_list",
  "function_call_parameter_list", "non_empty_fcall_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "class_statement_list", "class_statement", "$@19", "$@20", "$@21",
  "$@22", "$@23", "trait_rules", "trait_precedence_rule",
  "trait_alias_rule", "trait_alias_rule_method", "xhp_attribute_stmt",
  "$@24", "xhp_attribute_decl", "xhp_attribute_decl_type",
  "xhp_attribute_enum", "xhp_attribute_default",
  "xhp_attribute_is_required", "$@25", "xhp_category_stmt",
  "xhp_category_decl", "xhp_children_stmt", "xhp_children_paren_expr",
  "xhp_children_decl_expr", "xhp_children_decl_tag", "method_body",
  "variable_modifiers", "method_modifiers", "non_empty_member_modifiers",
  "member_modifier", "class_variable_declaration",
  "class_constant_declaration", "expr_list", "for_expr", "expr",
  "expr_no_variable", "$@26", "array_literal", "dim_expr", "dim_expr_base",
  "lexical_vars", "lexical_var_list", "xhp_tag", "xhp_tag_body", "$@27",
  "$@28", "xhp_opt_end_label", "xhp_attributes", "$@29", "xhp_children",
  "xhp_attribute_name", "xhp_attribute_value", "xhp_child", "xhp_label",
  "$@30", "$@31", "xhp_label_ws", "$@32", "$@33", "xhp_bareword",
  "simple_function_call", "static_class_name",
  "fully_qualified_class_name", "fully_qualified_class_name_no_typeargs",
  "class_name_reference", "exit_expr", "backticks_expr", "ctor_arguments",
  "common_scalar", "static_scalar", "static_class_constant", "scalar",
  "static_array_pair_list", "possible_comma",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_scalar_list_ae",
  "attribute_static_scalar_list", "non_empty_user_attribute_list",
  "user_attribute_list", "$@34", "non_empty_user_attributes",
  "optional_user_attributes", "variable", "dimmable_variable",
  "callable_variable", "object_method_call", "class_method_call",
  "variable_without_objects", "reference_variable", "compound_variable",
  "dim_offset", "simple_indirect_reference", "variable_no_calls",
  "dimmable_variable_no_calls", "assignment_list", "array_pair_list",
  "non_empty_array_pair_list", "encaps_list", "encaps_var",
  "encaps_var_offset", "internal_functions", "variable_list",
  "class_constant", "sm_name_with_type", "sm_name_with_typevar",
  "sm_typeargs_opt", "sm_type_list_gt", "sm_type_list",
  "sm_opt_return_type", "sm_typevar_list", "sm_type", "sm_cast_fix",
  "sm_type_opt", 0
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
     396,   397,    40,    41,    59,   123,   125,    36,    96,    93,
      34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   172,   173,   174,   174,   175,   175,   175,   175,   175,
     175,   176,   175,   177,   175,   175,   175,   178,   178,   179,
     179,   179,   179,   180,   180,   181,   181,   181,   182,   182,
     182,   183,   183,   184,   184,   185,   185,   185,   185,   186,
     186,   186,   187,   186,   188,   186,   189,   186,   190,   186,
     186,   186,   186,   186,   186,   186,   186,   186,   186,   186,
     186,   186,   186,   186,   186,   186,   191,   186,   186,   186,
     186,   186,   186,   186,   186,   192,   192,   194,   193,   195,
     195,   196,   196,   197,   199,   198,   200,   198,   202,   203,
     201,   204,   205,   201,   206,   201,   207,   201,   209,   208,
     210,   208,   211,   211,   212,   213,   214,   214,   214,   215,
     215,   216,   216,   217,   217,   218,   218,   219,   219,   220,
     220,   221,   221,   222,   222,   223,   223,   224,   224,   225,
     225,   226,   226,   227,   227,   227,   227,   228,   228,   228,
     229,   229,   230,   230,   231,   231,   232,   232,   233,   233,
     234,   234,   234,   234,   235,   235,   235,   235,   235,   235,
     235,   235,   236,   236,   237,   237,   237,   237,   238,   238,
     239,   239,   239,   240,   240,   240,   240,   241,   241,   243,
     242,   244,   242,   242,   245,   242,   246,   242,   247,   242,
     242,   242,   242,   242,   248,   248,   248,   249,   250,   250,
     251,   251,   252,   253,   252,   254,   254,   255,   255,   255,
     255,   255,   255,   255,   255,   256,   256,   257,   257,   259,
     258,   258,   260,   260,   261,   262,   262,   262,   263,   263,
     263,   263,   264,   264,   264,   264,   264,   264,   264,   265,
     265,   265,   266,   266,   267,   267,   268,   268,   269,   269,
     270,   270,   270,   270,   270,   270,   271,   271,   271,   271,
     272,   272,   273,   273,   274,   274,   275,   275,   276,   276,
     276,   276,   276,   276,   276,   276,   276,   276,   276,   276,
     276,   276,   276,   276,   276,   276,   276,   276,   276,   276,
     276,   276,   276,   276,   276,   276,   276,   276,   276,   276,
     276,   276,   276,   276,   276,   276,   276,   276,   276,   276,
     276,   276,   276,   276,   276,   276,   276,   276,   276,   276,
     276,   276,   276,   276,   276,   276,   276,   276,   276,   276,
     276,   276,   276,   276,   276,   277,   276,   276,   276,   278,
     279,   279,   280,   280,   280,   281,   281,   282,   282,   282,
     282,   283,   284,   285,   286,   284,   287,   287,   289,   288,
     288,   290,   290,   291,   292,   292,   293,   293,   293,   294,
     295,   294,   296,   294,   297,   298,   297,   299,   297,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   300,   300,   300,
     300,   300,   301,   302,   302,   302,   303,   303,   304,   304,
     305,   305,   305,   305,   305,   305,   306,   306,   306,   307,
     307,   307,   308,   308,   309,   309,   309,   309,   309,   309,
     309,   309,   309,   309,   309,   309,   309,   310,   310,   310,
     310,   310,   310,   311,   311,   312,   312,   312,   312,   312,
     312,   312,   313,   313,   314,   314,   315,   315,   315,   315,
     316,   316,   316,   316,   316,   317,   317,   317,   318,   318,
     318,   318,   318,   318,   319,   319,   320,   320,   320,   320,
     321,   321,   322,   322,   323,   323,   324,   324,   326,   325,
     327,   328,   328,   329,   329,   329,   329,   329,   329,   329,
     329,   329,   329,   329,   329,   330,   330,   330,   330,   330,
     330,   330,   330,   330,   331,   331,   331,   331,   332,   332,
     332,   333,   333,   334,   334,   335,   335,   335,   336,   336,
     337,   337,   338,   338,   339,   339,   339,   339,   339,   339,
     339,   339,   340,   340,   340,   340,   340,   341,   341,   341,
     341,   341,   341,   342,   342,   343,   343,   343,   343,   343,
     343,   343,   343,   344,   344,   344,   344,   345,   345,   345,
     345,   345,   345,   346,   346,   346,   347,   347,   347,   347,
     347,   347,   347,   348,   348,   349,   350,   350,   351,   351,
     352,   352,   353,   353,   353,   354,   354,   355,   355,   356,
     356,   356,   356,   357,   357,   357,   357,   357,   357,   357,
     357,   358,   358,   358,   358,   358,   359,   359
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     1,     1,     4,
       3,     0,     6,     0,     5,     3,     2,     3,     1,     1,
       2,     3,     4,     1,     3,     1,     2,     3,     1,     2,
       3,     5,     4,     2,     0,     1,     1,     1,     1,     3,
       7,    10,     0,     6,     0,     8,     0,    10,     0,     6,
       2,     3,     2,     3,     2,     3,     3,     3,     5,     8,
       3,     3,     3,     5,     1,     1,     0,     9,     5,    14,
       5,     3,     3,     2,     2,     9,     0,     0,     5,     1,
       0,     1,     0,     1,     0,    11,     0,    12,     0,     0,
       9,     0,     0,    10,     0,     7,     0,     8,     0,     6,
       0,     7,     1,     1,     1,     1,     1,     2,     2,     2,
       0,     2,     0,     2,     0,     1,     3,     1,     3,     2,
       0,     1,     2,     1,     4,     1,     4,     1,     4,     1,
       4,     3,     5,     3,     4,     4,     5,     5,     4,     0,
       1,     1,     6,     0,     7,     0,     2,     0,     3,     0,
       3,     1,     1,     0,     3,     4,     6,     5,     5,     6,
       8,     7,     1,     0,     1,     2,     3,     4,     3,     1,
       1,     2,     4,     3,     5,     1,     3,     2,     0,     0,
       4,     0,     5,     2,     0,    10,     0,    11,     0,     4,
       3,     3,     3,     5,     2,     2,     0,     6,     5,     4,
       3,     1,     1,     0,     4,     4,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     1,     3,     2,     0,     0,
       3,     0,     1,     3,     1,     1,     1,     1,     3,     4,
       4,     4,     1,     1,     2,     2,     2,     3,     3,     1,
       1,     1,     1,     3,     1,     1,     1,     0,     1,     2,
       1,     1,     1,     1,     1,     1,     3,     5,     1,     3,
       5,     4,     3,     1,     1,     0,     1,     1,     6,     3,
       4,     6,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     3,     2,     0,    11,     1,     1,     4,
       4,     4,     1,     1,     3,     4,     0,     3,     4,     1,
       2,     4,     2,     0,     0,     8,     0,     1,     0,     5,
       0,     2,     0,     1,     1,     3,     1,     3,     1,     1,
       0,     4,     0,     4,     1,     0,     4,     0,     4,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     2,     3,     2,     3,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     2,
       2,     4,     1,     3,     3,     1,     1,     1,     1,     3,
       3,     3,     2,     0,     1,     0,     5,     3,     3,     1,
       1,     1,     1,     3,     2,     1,     1,     1,     1,     1,
       2,     2,     4,     3,     2,     0,     5,     3,     3,     1,
       3,     1,     2,     0,     3,     0,     4,     2,     0,     3,
       3,     1,     0,     1,     1,     1,     1,     4,     4,     3,
       3,     5,     3,     4,     3,     1,     1,     1,     4,     4,
       3,     5,     4,     3,     1,     4,     4,     3,     6,     6,
       8,     6,     6,     1,     2,     4,     4,     1,     1,     4,
       1,     0,     1,     2,     1,     4,     4,     3,     3,     5,
       3,     3,     4,     4,     3,     5,     3,     2,     3,     6,
       0,     1,     4,     2,     0,     5,     3,     3,     1,     6,
       4,     4,     2,     2,     2,     1,     2,     1,     4,     3,
       3,     6,     3,     1,     1,     1,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     1,     2,     1,     4,
       2,     0,     4,     2,     3,     1,     3,     0,     2,     3,
       1,     5,     3,     2,     2,     2,     1,     1,     8,     6,
       5,     1,     1,     1,     1,     1,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     2,     1,     0,     0,     0,     0,     0,     0,
       0,   538,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   468,     0,
     474,   475,    23,   496,   578,    65,   476,     0,    44,     0,
       0,     0,     0,     0,     0,     0,     0,    83,     0,     0,
       0,     0,     0,     0,     0,     0,   454,     0,     0,     0,
       0,   106,     0,     0,     0,   480,   482,   483,   477,   478,
       0,     0,   484,   479,     0,     0,   459,     0,   481,     0,
      64,    34,   582,   469,     0,     0,     3,    25,   495,   458,
       0,     5,    82,     6,     7,     8,     0,     0,   266,   332,
     338,     0,   337,   544,     0,   453,   498,   331,     0,   267,
       0,     0,   545,   546,   543,   573,   577,     0,   321,   497,
      23,   454,     0,     0,    82,   632,   267,   631,     0,   629,
     628,   334,   424,   423,   420,   422,   421,   440,   442,   441,
     412,   402,   418,   417,   380,   389,   390,   392,   391,   379,
     411,   395,   393,   394,   396,   397,   398,   399,   400,   401,
     403,   404,   405,   406,   407,   408,   410,   409,   381,   382,
     383,   385,   386,   388,   426,   427,   436,   435,   434,   433,
     432,   431,   419,   437,   428,   429,   430,   413,   414,   415,
     416,   438,   439,   443,   445,   444,   446,   447,   425,   449,
     448,   384,   450,   451,   387,   360,   369,     0,     0,   305,
     306,   307,   308,   330,   328,   327,   326,   325,   324,   323,
     322,     0,     0,     0,   288,   286,   273,   461,     0,     0,
     462,     0,   463,     0,   473,   584,   460,     0,     0,   329,
       0,    74,     0,   263,     0,     0,   265,     0,     0,     0,
      50,     0,     0,    52,     0,     0,     0,   636,   656,   657,
       0,     0,     0,    54,     0,    34,     0,     0,     0,    18,
      19,   170,     0,     0,   169,   108,   107,   175,     0,     0,
       0,     0,     0,   638,    94,   104,   600,   604,   617,     0,
     486,     0,     0,     0,   615,     0,    13,     0,    26,     0,
       0,    98,   105,     0,   266,   267,     0,     0,   470,     0,
     471,     0,     0,     0,     0,   163,     0,    16,    81,     0,
     103,    88,   102,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    73,
     581,   581,     0,     0,     0,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   287,
     285,     0,   581,     0,   163,   581,     0,   583,   574,   600,
       0,     0,     0,   370,   372,     0,     0,   540,   535,   505,
       0,     0,     0,   464,     0,     0,   163,   272,     0,   581,
       0,   466,     0,     0,     0,    62,     0,     0,   264,     0,
       0,     0,     0,     0,    51,    72,    53,   641,   653,   654,
       0,   655,     0,     0,     0,   637,    55,     0,    71,    20,
       0,    15,     0,     0,   171,     0,    60,     0,     0,    61,
     633,     0,     0,     0,     0,     0,   114,     0,   601,     0,
       0,   608,     0,   505,     0,     0,   485,   616,   496,     0,
       0,   614,   501,   613,    27,     4,    10,    11,    56,    57,
       0,   318,   554,    39,    33,    35,    36,    37,    38,     0,
     333,   499,   500,    24,     0,     0,   162,   164,     0,   335,
      84,   110,   291,   293,   292,     0,     0,   289,   290,   294,
     296,   295,   310,   309,   312,   311,   313,   315,   316,   314,
     304,   303,   298,   299,   297,   300,   301,   302,   317,   580,
       0,     0,   635,   552,   573,    96,   100,     0,    91,     0,
       0,   269,   284,   283,   282,   281,   280,   279,   278,   277,
     276,   275,   274,   549,     0,   550,     0,     0,     0,     0,
       0,     0,   630,     0,     0,   351,   353,   352,     0,   363,
     374,   533,   537,   504,   539,     0,   465,   591,   590,     0,
     587,     0,   588,     0,     0,   467,     0,   262,     0,    42,
     265,     0,     0,     0,     0,    48,   641,   640,     0,   661,
     664,   665,   663,   662,     0,     0,     0,     0,     0,     0,
       0,     0,   488,     0,   487,    32,   492,    77,     0,    17,
      21,     0,   168,   176,   173,     0,     0,   626,   627,     9,
     650,     0,     0,     0,   600,   597,     0,   612,     0,   339,
     504,   603,   623,   625,   624,     0,   619,     0,   620,   622,
       0,     4,   178,   579,   165,   452,     0,     0,   542,     0,
       0,   112,   320,     0,   340,   341,   163,   163,   114,     0,
      86,   110,     0,   270,     0,   163,     0,   163,   547,   548,
     553,   575,   576,     0,   371,   373,   362,   358,   375,   377,
       0,     0,   525,   510,   511,   519,   512,     0,     0,   518,
     531,   505,     0,   535,   472,     0,   585,   586,    34,   143,
       0,     0,     0,     0,   120,   121,   131,     0,    34,   129,
      68,     0,     0,     0,   643,     0,   645,     0,     0,   489,
     490,   503,     0,     0,     0,     0,    70,     0,    22,   172,
       0,   634,    63,     0,     0,   639,     0,     0,   457,    28,
     641,   113,   115,   178,     0,     0,   598,     0,     0,   607,
       0,   606,   618,     0,    14,     0,   247,     0,   166,    31,
     152,     0,   151,   541,   667,   542,   109,     0,     0,   319,
       0,     0,     0,   178,     0,   112,   473,    58,     0,   551,
       0,     0,     0,     0,     0,     0,   515,   516,   517,   520,
     521,   529,     0,   505,   525,     0,   514,   504,   532,   534,
     536,   589,   145,   147,     0,    34,   127,    43,   265,   122,
       0,     0,     0,     0,   139,   139,    49,     0,   645,   644,
       0,     0,     0,   660,   509,     0,   505,   494,   493,     0,
       0,   174,   649,   652,     0,    29,   456,     0,   247,   602,
     600,     0,   268,   611,   610,     0,     0,    12,     0,     0,
     250,   251,   252,   255,   254,   253,   245,   188,     0,     0,
      99,   177,   179,     0,   244,   248,     0,   247,   167,   647,
     542,   666,     0,     0,   111,    89,   571,   572,   178,   247,
     542,     0,   271,   568,   163,   569,     0,   366,     0,   368,
     361,   364,     0,   359,   376,   378,     0,   523,   504,   524,
       0,   513,   530,   149,     0,     0,    40,     0,     0,     0,
     119,    66,   132,     0,   139,     0,   139,     0,   642,     0,
       0,   646,   659,     0,   491,   504,   502,     0,    34,     0,
      30,   116,    95,     0,     0,     0,   605,   621,     0,     0,
     117,     0,   224,     0,   222,   226,   227,     0,     0,   225,
       0,    82,   249,   181,     0,   183,     0,   246,     0,   346,
     150,   667,     0,   154,   647,   178,   247,   101,     0,    92,
       0,   354,     0,     0,   528,   527,   522,     0,     0,     0,
       0,   146,    45,     0,    46,     0,   130,     0,     0,     0,
       0,     0,   133,     0,   508,   507,     0,     0,   651,   599,
      59,   609,     0,     0,   192,   196,   208,   210,   207,   214,
     209,   212,   457,     0,     0,   202,     0,   211,     0,   190,
     239,   240,   241,   232,     0,   233,   191,   258,     0,     0,
       0,     0,    82,   648,     0,     0,     0,   155,     0,     0,
     247,    97,   647,   178,   570,   356,   367,   365,     0,     0,
      34,     0,     0,   128,     0,    34,   125,    67,     0,   135,
       0,   140,   141,    34,   134,   658,     0,     0,    78,   261,
     118,     0,     0,   203,   189,   218,   223,     0,     0,   228,
     235,   236,   234,     0,     0,   180,     0,     0,     0,     0,
       0,    34,     0,   158,     0,   157,    34,    90,     0,   247,
     357,   355,   526,     0,   148,    41,     0,    34,   123,    47,
       0,   136,    34,   138,   506,    34,    23,   193,     0,   194,
     195,     0,     0,   215,     0,     0,   221,   237,   238,   230,
     231,   229,   259,   256,   184,   182,   260,     0,     0,   349,
       0,     0,   159,     0,   156,     0,    34,    93,     0,   142,
       0,     0,   137,     0,     0,   247,     0,   213,   204,   217,
     219,   205,     0,   542,   186,   350,     0,   345,   336,     0,
     161,    85,     0,    34,     0,   126,    76,   200,     0,   246,
     216,     0,   257,     0,   542,     0,   347,   160,    87,   144,
     124,    80,     0,     0,   199,   220,   647,     0,   348,     0,
      79,    69,     0,   198,     0,   647,     0,   197,   242,    34,
     185,     0,     0,     0,   187,     0,   243,     0,    34,     0,
      75
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    86,   641,   465,   268,   269,    87,    88,
      89,    90,   306,   474,   475,   701,   244,  1054,   711,   985,
    1191,   726,   727,  1201,   319,   124,   476,   649,   774,   477,
     491,   965,   661,  1043,   446,   658,   478,   470,   659,   321,
     284,   301,    96,   651,   768,   623,   741,   939,   811,   704,
    1109,  1057,   807,   710,   412,   816,   915,  1063,   803,   903,
     906,   979,   761,   762,   485,   486,   273,   274,   278,   756,
     861,   950,  1030,  1163,  1184,   941,  1071,  1119,  1120,  1121,
    1014,  1124,  1015,  1016,  1122,  1126,  1161,  1181,   943,   944,
     948,  1023,  1024,  1025,  1210,   862,   863,   864,   865,  1028,
     866,   408,   409,    97,    98,   648,    99,   100,   101,  1035,
    1140,   102,   385,   676,  1045,  1101,   386,   783,   782,   558,
     893,   890,   205,   553,   554,   559,   784,   785,   206,   103,
     104,   742,   105,   234,   239,   309,   397,   106,   605,   606,
     107,   825,   564,   826,   689,   789,   791,   792,   793,   691,
     692,   562,   389,   207,   208,   108,   764,   126,   110,   111,
     112,   113,   114,   115,   116,   520,   117,   236,   237,   449,
     452,   453,   293,   294,   635,   118,   441,   119,   261,   285,
     421,   587,   921,   959,   621,   262,   595,   872
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -931
static const yytype_int16 yypact[] =
{
    -931,   101,  2142,  -931,  7223,  7223,   -54,  7223,  7223,  7223,
    7750,  -931,  7223,  7223,  7223,  7223,  7223,  7223,  7223,  7223,
    7223,  7223,  7223,  7223,  1105,  1105,  7223,  1117,   -45,     3,
    -931,  -931,   188,  -931,  -931,  -931,  -931,  7223,  -931,   150,
     164,   166,   172,   190,  5192,   289,  5337,  -931,   109,  5482,
     197,  7223,   268,   221,   258,   264,   316,   259,   280,   293,
     300,  -931,   355,   314,   321,  -931,  -931,  -931,  -931,  -931,
     393,   211,  -931,  -931,   369,  5627,  -931,   355,  -931,  7223,
    -931,  -931,   342,    32,   361,   361,  -931,   375,   352,  -931,
      33,  -931,   502,  -931,  -931,  -931,    40,  1372,  -931,   464,
     478,   480,  -931,    67,   406,  -931,  -931,  -931,   615,  1685,
      81,   389,    85,    92,   411,    75,  -931,   224,  -931,   523,
    -931,  -931,   419,   452,   502,  9246,  5263,  9246,  7223,  9246,
    9246,  9162,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,   113,  -931,   550,   528,  -931,
    -931,   552,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  1105,   352,   474,   499,   499,  -931,   485,   498,   369,
     497,  1105,   375,   505,   512,  -931,   535,   108,  5772,  -931,
    7223,  -931,    34,  9246,  5047,  7223,  7223,  7223,   593,  7223,
    -931,  8307,   511,  -931,  8348,   201,   201,   367,  -931,  -931,
     256,   644,   613,  -931,  8389,  -931,  8445,   369,    35,  -931,
     -15,  -931,   424,    36,  -931,  -931,  -931,   675,    38,  1105,
    1105,  1105,   526,   650,  -931,  -931,   877,  5917,    52,   491,
    -931,  7368,  1105,   494,  -931,   369,  -931,   -13,   382,   527,
    8486,  -931,  -931,  9246,   537,   908,  2287,  7223,   438,   536,
     520,   438,   354,   257,   631,  6062,   109,  -931,  -931,   231,
    -931,  -931,  -931,  7223,  7223,  7223,  6208,  7223,  7223,  7223,
    7223,  7223,  7223,  7223,  7223,  7223,  7223,  7223,  7223,  7223,
    7223,  7223,  7223,  7223,  7223,  7223,  7223,  7223,  1117,  -931,
    7223,  7223,    86,   355,   355,   502,    40,  1443,  7223,  7223,
    7223,  7223,  7223,  7223,  7223,  7223,  7223,  7223,  7223,  -931,
    -931,   248,  7223,  7223,  6062,  7223,  7223,   342,   115,   877,
     546,  6353,  8624,  -931,  -931,   674,  7496,  -931,   554,   706,
       5,   203,   369,   382,    19,   249,  6062,  -931,   439,  7223,
    7223,  -931,  8665,  8721,  7223,  -931,   635,  8763,   711,   556,
    9139,   713,    30,  8804,  -931,  -931,  -931,   693,  -931,  -931,
     313,  -931,    66,   723,  8949,  -931,  -931,  2432,  -931,    14,
     268,  -931,   667,  7223,   499,   221,  -931,  8949,   671,  -931,
     499,    49,    50,   194,   583,   680,   646,   606,   499,    55,
    1105,  9002,   608,   762,   534,   703,  -931,  -931,   714,  1635,
     -18,  -931,  -931,  -931,   404,  -931,  -931,  -931,  -931,  -931,
     611,   716,    10,  -931,  -931,  -931,  -931,  -931,  -931,  1723,
    -931,  -931,  -931,  -931,  1105,   617,   773,  9246,   769,  -931,
    -931,   664,  9286,  9323,  9162,  7223,  9205,  9368,  9389,  5387,
    5532,  5820,  5965,  5965,  5965,  5965,  1830,  1830,  1830,  1830,
     927,   927,   696,   696,   696,   552,   552,   552,  -931,  9246,
     616,   618,   624,   628,   115,  -931,  -931,   355,  -931,   425,
    7223,  9162,  9162,  9162,  9162,  9162,  9162,  9162,  9162,  9162,
    9162,  9162,  9162,    72,  7223,   629,   625,  7887,   633,   630,
    7928,    58,  -931,  7750,  7750,  -931,  -931,  -931,   785,   458,
    -931,   736,  -931,   731,  -931,   624,   404,   122,  -931,   640,
     130,  7223,  -931,   643,  7969,  -931,  4318,  9246,   642,  -931,
    7223,   319,  8949,   742,  4464,  -931,   780,  -931,   110,  -931,
    -931,  -931,  -931,  -931,   201,   791,   201,  8949,  8949,   657,
      29,   683,  -931,   684,  -931,  -931,  -931,   730,   754,  -931,
    -931,  8027,  -931,  -931,   820,  1105,   672,  -931,  -931,  -931,
      65,   798,   362,   676,   877,  1063,   824,   499,  6498,  -931,
    6643,  -931,  -931,  -931,  -931,   670,  -931,  7223,  -931,  -931,
    1852,  -931,  -931,  -931,   499,  -931,  6788,  8949,    73,   678,
     362,   727,  9346,  7223,  -931,  -931,  6062,  6062,   646,   689,
    -931,   664,  1117,   499,  8527,  6062,  8068,  6062,    76,    82,
     140,  -931,  -931,   830,  -931,  -931,  -931,  -931,  -931,  -931,
     548,   548,   736,  -931,  -931,  -931,  -931,   686,    37,  -931,
    -931,   841,   695,   554,  -931,  8109,   155,   178,  -931,  -931,
    7223,  4610,   692,  1105,   737,   499,  -931,   851,  -931,  -931,
    -931,    22,   313,   313,  -931,   702,   858,   201,   704,  -931,
    -931,  8949,   746,   797,   803,   724,  -931,   732,  -931,  -931,
    8949,   499,  -931,   680,   814,  -931,   750,   369,  -931,   753,
     693,   890,  -931,  -931,    60,   739,   499,  6933,  1105,  9246,
    1105,  9098,  -931,    51,  -931,  1997,   446,  1105,  9246,  -931,
    -931,   741,   894,  -931,   201,    73,  -931,   362,   743,  9346,
     744,   747,   748,  -931,   752,   727,   512,  -931,   775,   127,
     776,  7223,    20,   138,  7750,  7750,  -931,  -931,  -931,  -931,
    -931,   819,   767,   934,   736,   806,  -931,   736,  -931,  -931,
    -931,   184,  4172,   493,  8858,  -931,  -931,  -931,  7223,   499,
     319,   781,  8949,  2577,   787,   788,  -931,   905,   329,  -931,
     923,   201,   790,  -931,   832,   792,   949,  -931,  -931,   362,
     793,  -931,  -931,   951,   369,   753,  -931,   362,   561,  -931,
     877,  7223,  9162,   499,   499,  7078,   794,  -931,   109,   362,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,   811,   246,
    -931,  -931,  -931,   864,   343,  -931,    39,   770,   499,   937,
      24,  -931,   301,   816,   890,  -931,  -931,  -931,  -931,   586,
      73,   812,  -931,  -931,  6062,  -931,  7623,  -931,  7223,  -931,
    -931,  -931,  7223,  -931,  -931,  -931,   736,  -931,   736,  -931,
     818,  -931,  -931,   544,   822,  5047,  -931,   826,  2722,   828,
    -931,  -931,  -931,   829,  -931,   542,  -931,   278,  -931,   313,
     201,  -931,  -931,  8949,  -931,  8949,  -931,   909,  -931,   680,
     753,  -931,  -931,    61,  8583,  1105,  9246,  -931,   979,    31,
    -931,   834,  -931,    41,  -931,  -931,  -931,   311,   833,  -931,
     931,   502,  -931,  -931,   109,  -931,   864,   770,   201,   915,
    -931,   201,   952,  1009,   937,  -931,   989,  -931,   861,  -931,
     865,  -931,  8167,  8208,  -931,   910,  -931,   871,  1001,   966,
    7223,  -931,  -931,   872,  -931,  4756,  -931,   569,   873,  7223,
      25,   306,  -931,   878,  -931,   913,   879,  2867,  -931,  -931,
    -931,   499,  8949,   362,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,    42,   881,    44,  -931,  7750,  -931,   811,  -931,
    -931,  -931,  -931,  -931,    91,   495,  -931,  1036,    45,   355,
     931,  1038,   502,  -931,   891,   889,   347,  1042,  8949,   892,
    1324,  -931,   937,  -931,  -931,   916,  -931,  -931,   736,  7223,
    -931,   895,  8900,  -931,  4902,  -931,  -931,  -931,   897,  -931,
    8251,  -931,  -931,  -931,  -931,  -931,  8949,   898,  -931,  -931,
    -931,   336,  1392,  -931,  -931,   119,  -931,   311,   311,   558,
    -931,  -931,  -931,  8949,   983,  -931,   903,    46,  8949,   355,
     356,  -931,   993,  1054,  8949,  -931,  -931,  -931,   907,  1677,
    -931,  -931,  -931,  8941,  4172,  -931,  5047,  -931,  -931,  -931,
    3012,  -931,  -931,  4172,  -931,  -931,   978,  -931,   930,  -931,
    -931,   984,    28,  -931,   834,  8949,  1024,  1047,  -931,  -931,
    -931,  -931,  -931,  1065,  -931,  -931,  -931,   918,  1008,  -931,
      62,  3157,  1069,  8949,  -931,  3302,  -931,  -931,  1057,  -931,
    3447,   920,  4172,  3592,  1014,   770,  1392,  -931,  -931,  -931,
    -931,  -931,  8949,    73,  -931,  -931,   370,  -931,  -931,  8949,
    -931,  -931,  3737,  -931,   922,  -931,  -931,   932,  1018,   572,
    -931,   938,  -931,   940,    73,  1017,  -931,  -931,  -931,  4172,
    -931,     0,   362,   941,  -931,  -931,   937,   943,  -931,   945,
    -931,  -931,    47,  -931,   326,   937,   362,  -931,  -931,  -931,
    -931,   326,  1021,  3882,  -931,   946,  -931,   948,  -931,  4027,
    -931
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -931,  -931,  -434,  -931,  -931,  -931,  -931,   685,    -7,   125,
     655,  -931,  -262,  -931,     4,  -931,  -931,  -931,  -931,  -931,
    -931,   -77,  -931,  -931,  -120,     8,    -1,  -931,  -931,     6,
    -931,  -931,  -931,  -931,  -931,  -931,     7,  -931,  -931,   760,
     766,   771,  1012,   462,   349,   468,   363,   -64,  -931,   322,
    -931,  -931,  -931,  -931,  -931,  -931,  -547,    69,  -931,  -931,
    -931,  -931,  -739,  -931,  -356,  -931,  -931,   698,  -931,  -714,
    -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,    11,  -931,  -931,  -931,  -931,  -931,  -931,   123,
    -931,   272,  -425,  -931,   -69,  -931,  -839,  -837,  -848,   114,
    -931,  1108,  -543,   994,  1067,  -931,  -931,  -931,  -931,  -931,
    -931,   365,  -931,  -931,  -931,  -931,  -931,  -931,  -931,  -931,
    -931,  -931,  -931,  -931,  -931,   136,  -931,  -931,  -375,  -931,
     237,  -645,  -931,  -329,  -931,  -931,   378,  -299,  -274,  -931,
    -931,  -931,  -428,  -931,  -931,   475,  -554,   368,  -931,  -931,
    -931,   466,  -931,  -931,  -931,  -621,   287,    -2,  -931,  -931,
    -931,  -931,   -12,   -85,  -931,  -245,  -931,  -931,  -931,  -355,
    -931,  -931,   673,   607,  -931,  -931,   884,  -931,  -314,   -63,
     426,  -692,  -522,  -930,  -698,  -243,  -931,   204
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -642
static const yytype_int16 yytable[] =
{
     109,    93,   488,   427,   380,   766,    91,   690,    94,    95,
      92,   560,   418,   419,   302,   235,   952,   423,   548,   518,
     232,   819,   224,   225,   551,   631,   873,   763,   956,   838,
     957,   640,   378,   322,  1039,   832,  1156,   702,   583,  1003,
     569,   316,   404,   430,   435,   270,   438,   954,   814,  1018,
    -206,  1061,  1073,  1084,  1084,  1003,   886,   615,   615,   879,
     323,   324,   325,   625,   297,    11,   625,   298,   625,   625,
    1166,  -563,   715,   733,   718,   432,   326,   305,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,  1077,
     348,     3,  1199,   371,   608,   288,   521,   722,   128,   952,
     308,   283,  1098,   454,    11,   795,   355,   238,   713,   589,
    1078,   590,   591,   592,   593,   604,   371,   546,  -555,   314,
     549,   314,  1125,  -560,   255,   867,   375,  -558,   604,   383,
     371,   968,   372,  -559,   763,   678,  -556,   714,   639,   222,
     222,   466,   467,  -557,   573,   734,   384,   522,   314,    34,
     256,   -77,   679,   613,   966,   240,   290,   887,   472,   399,
     291,   292,  -567,   455,   796,  -563,   375,   588,   674,   675,
     257,   960,   567,  -596,   927,   888,   320,   815,  -561,  1062,
     817,  -594,   931,   584,  1157,  1004,  1005,   317,   405,   431,
     436,  -562,   439,   955,   940,  1019,  -206,   755,  1074,  1085,
    1135,  1207,   616,   617,   241,  -455,  -592,   867,   626,   390,
     846,   673,   393,   839,   999,  1167,   255,   819,   594,   394,
     760,   998,  -555,   258,   665,   527,  -153,  -560,  -565,  -593,
     376,  -558,   109,   902,  -566,  -595,   373,  -559,   406,   763,
    -556,  1040,   256,    82,  1079,   259,   490,  -557,   867,   763,
     429,   223,   223,   798,   233,   909,  1204,   524,   917,   744,
     434,   260,   417,   400,   565,  1211,    34,   440,   440,   443,
     376,   255,   120,   604,   448,   891,   524,  -596,   464,   884,
     460,   302,  -561,   322,   271,  -594,  1017,    34,   604,   604,
     770,   771,   283,   892,   109,  -562,   524,   256,   706,   778,
     524,   780,   245,   524,    92,   371,  1178,   945,  1179,   543,
    -592,    34,    34,   719,   720,   258,   246,   417,   247,  1099,
     288,   952,   962,   776,   248,   461,   235,   919,   255,   120,
     523,   232,   974,  -593,   975,   867,   222,   259,   604,  -595,
     703,   716,   249,   716,   422,   295,   222,   618,  1070,   545,
     252,   946,   265,   260,   256,   899,   714,   987,   255,   991,
      82,   989,   990,   759,   963,   275,   296,   448,  1092,   523,
     258,   276,  1020,   568,   586,   566,   572,  1138,   272,   277,
     120,   377,    34,   489,   256,   291,   292,   222,   926,   989,
     990,  1185,   259,   420,   222,   222,   222,  1116,   947,   894,
     895,   222,   267,   544,   417,    82,    82,   222,   260,   867,
    1093,   279,   604,   270,  1183,   109,   283,   288,   482,  1139,
     121,   604,   461,   120,   288,    92,   802,   258,  -641,   311,
     120,  -246,   280,  1186,   992,  1197,   813,   824,   627,   850,
     851,   852,   853,   854,   855,   281,   831,  1021,   223,   259,
     123,  1022,   282,    74,   660,    76,   288,   258,   223,   818,
     588,   289,  1064,   947,   822,   260,   286,   736,   867,  1017,
     737,   221,   644,   287,   678,   933,    82,    11,   662,   259,
    1208,  1209,   291,   292,  1102,   120,   120,    34,    34,   291,
     292,   679,  1117,   736,   222,   260,   737,   307,   738,   223,
     570,   288,    34,   604,   315,   -28,   223,   223,   223,   314,
    1080,   871,   -29,   223,   481,  -342,   314,   663,   970,   223,
     290,   291,   292,   318,   938,   121,   121,  1081,   912,   350,
    1082,   351,   763,   908,   -30,   848,   352,   940,   314,   602,
     849,   374,   850,   851,   852,   853,   854,   855,   856,   904,
     905,  1212,   602,   763,   288,   123,   123,   288,    74,    74,
      76,    76,   461,  -564,   109,   222,   291,   292,   716,   705,
     699,   379,   109,  1129,  -343,   233,   221,   221,   709,   433,
     387,    82,    82,   288,   857,   858,   295,   859,   461,   388,
    1130,   348,    11,  1131,   571,   632,    82,   633,   634,   222,
     977,   978,   860,   731,   391,   739,   223,   786,   787,   788,
     371,   953,   448,   746,   604,  -454,   604,    11,   456,   291,
     292,   462,   291,   292,   988,   989,   990,  -459,   109,    93,
    1031,   560,   392,   739,    91,   395,    94,    95,    92,   994,
     235,   995,  1127,  1128,   222,   232,   398,   424,   291,   292,
     848,  1058,   989,   990,   411,   849,   997,   850,   851,   852,
     853,   854,   855,   856,   396,   415,   818,   993,   850,   851,
     852,   853,   854,   855,   425,   848,   445,   223,   437,   444,
     849,   468,   850,   851,   852,   853,   854,   855,   856,   109,
     471,   809,   483,   604,   480,   806,   222,   602,   489,   857,
     858,   555,   859,    47,   563,  1033,   561,   578,   871,   404,
     580,   223,   602,   602,    54,    55,   582,   932,  1069,   420,
     835,   596,    61,   353,   857,   858,  1194,   859,   610,   604,
     222,   345,   346,   347,   614,   348,   843,   619,   844,   222,
     222,   620,   967,   109,    93,   868,   310,   312,   313,    91,
     739,    94,    95,    92,  1095,   622,   223,   604,   624,   354,
     630,   629,   602,  1123,   636,   637,   642,  -344,   680,   681,
     645,   646,   647,   650,   604,   654,   656,   655,  1104,   604,
     657,   667,  1114,  1110,   668,   604,   670,   682,   677,   671,
     109,  1113,   693,   694,   700,   683,   684,   685,   705,  1132,
      92,   109,   696,   707,  1136,   686,   712,   717,   223,   721,
    1144,    92,   739,   723,   724,   728,   604,   930,   222,  1141,
     739,  1029,   725,   730,  1145,   735,   732,   747,   448,   752,
     765,   743,   739,   781,   604,  1150,   602,   767,   794,   797,
    1152,  1159,   223,  1153,   773,   602,   808,  1180,   799,   810,
     687,   223,   223,   604,   812,   820,   821,   823,   827,  1170,
     604,   951,   688,   222,   828,   222,   850,   851,   852,   853,
     854,   855,   222,   456,  1172,   833,   829,  1006,  1182,  1007,
    1008,  1009,  1010,   830,   834,  1187,   457,   314,   837,   233,
     463,   840,   870,   109,   869,   120,   109,   876,   875,   981,
     877,  1189,  1089,   878,   880,   457,    92,   463,   457,   463,
     463,   381,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,  1001,   739,   222,   897,   602,   883,   885,
     223,   896,   898,   901,   911,   918,  1011,  1213,   120,   920,
      34,   914,   916,   922,   923,   924,  1219,   925,   928,   929,
     937,   942,    47,   958,  1032,   222,  1086,   369,   370,   342,
     343,   344,   345,   346,   347,   736,   348,   969,   737,   964,
    1012,   976,   996,   109,   980,   223,  1013,   223,   121,  1056,
     982,   984,  1002,   986,   223,   109,   739,  1026,   125,   127,
     447,   129,   130,   131,  1027,    92,   209,   210,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   123,  1034,
     226,    74,  1038,    76,  1042,  1037,  1137,  1050,  1044,   371,
      11,   243,  1048,  1049,  1051,  1066,  1053,  1059,   251,   221,
     254,  1065,  1067,   264,    82,   266,  1072,   223,   602,  1083,
     602,  1088,   109,  1090,  1091,  1094,  1133,  1096,  1108,  1105,
     222,  1111,  1100,  1115,   739,  1134,  1142,  1143,  -201,   300,
    1154,   472,  1146,   303,  1155,  1160,  1078,   223,  1162,   603,
    1164,  1165,  1169,  1173,  1175,  1177,  1190,  1192,   848,  1193,
    1198,  1195,   603,   849,  1215,   850,   851,   852,   853,   854,
     855,   856,   109,  1196,   109,  1203,  1205,  1206,   109,  1217,
    1149,   109,    92,  1218,  1200,   609,   528,   739,    92,   525,
     356,    92,   382,   775,   881,   526,   772,   602,  1202,  1112,
     874,   949,   910,   612,   120,  1158,    34,   857,   858,   109,
     859,  1076,  1214,   109,  1087,   242,   304,   889,   109,    92,
     109,   109,  1075,    92,   882,  1041,   790,   961,    92,   800,
      92,    92,   900,   602,   442,  1036,   836,     0,     0,     0,
     109,     0,   223,     0,   121,     0,   120,     0,    34,     0,
      92,     0,     0,     0,     0,   739,   745,   109,   120,     0,
      34,   602,     0,     0,     0,     0,     0,    92,     0,   739,
       0,     0,     0,     0,   123,     0,     0,    74,   602,    76,
       0,   109,     0,   602,     0,     0,   121,   109,     0,   602,
       0,    92,     0,     0,     0,   221,     0,    92,   227,     0,
      82,     0,   402,     0,   403,     0,     0,   603,     0,   407,
     243,   410,     0,   413,     0,     0,   123,     0,     0,    74,
     602,    76,   603,   603,     0,     0,     0,     0,   228,     0,
       0,   229,     0,   230,     0,     0,     0,   221,   602,     0,
       0,     0,    82,     0,     0,     0,     0,   740,     0,   231,
       0,   451,     0,     0,    82,   459,     0,   602,     0,     0,
       0,     0,     0,     0,   602,     0,     0,     0,     0,     0,
       0,   479,   603,     0,     0,   740,     0,     0,     0,   487,
       0,     0,     0,     0,     0,     0,     0,   492,   493,   494,
     496,   497,   498,   499,   500,   501,   502,   503,   504,   505,
     506,   507,   508,   509,   510,   511,   512,   513,   514,   515,
     516,   517,     0,     0,   519,   519,     0,     0,     0,     0,
       0,   531,   532,   533,   534,   535,   536,   537,   538,   539,
     540,   541,   542,     0,     0,    11,   519,   547,   487,   519,
     550,     0,     0,     0,     0,   531,   603,     0,     0,     0,
       0,   323,   324,   325,     0,   603,     0,     0,     0,     0,
     487,     0,     0,   519,   574,     0,     0,   326,   577,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
       0,   348,   740,   848,     0,     0,     0,   611,   849,     0,
     850,   851,   852,   853,   854,   855,   856,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     4,     5,     6,     7,
       8,     0,     0,     0,     0,     9,     0,     0,     0,     0,
       0,    30,    31,     0,     0,     0,     0,   603,     0,     0,
       0,    36,   857,   858,   529,   859,     0,     0,     0,    10,
       0,     0,     0,     0,   740,    12,    13,     0,     0,   652,
    1097,    14,   740,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,   740,    26,    27,    28,     0,     0,
       0,     0,    30,    31,   120,    33,    34,    65,    66,    67,
      68,    69,    36,     0,   664,     0,     0,     0,   600,     0,
       0,     0,     0,     0,    72,    73,   349,     0,   666,     0,
       0,    47,     0,     0,     0,     0,     0,     0,    78,     0,
       0,     0,     0,     0,   121,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,   695,   122,    64,    65,    66,
      67,    68,    69,     0,   243,     0,     0,     0,   603,    70,
     603,     0,     0,     0,   123,    72,    73,    74,   530,    76,
       0,     0,     0,     0,     0,     0,   740,     0,     0,    78,
       0,     0,     0,     0,     0,    79,     0,     0,     0,     0,
      82,    83,     0,    84,    85,     0,     0,     0,     0,     0,
       0,     0,   749,     0,   751,     0,     0,     0,     0,     0,
       0,   753,     0,     0,     0,     0,     0,     0,     0,     0,
     758,     0,     0,     0,   323,   324,   325,   769,     0,     0,
     487,   487,     0,     0,     0,     0,     0,   603,   740,   487,
     326,   487,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,     0,   348,     0,     0,     0,     0,     0,
       0,     0,     0,   603,   804,     0,     0,     0,   357,   358,
     359,   360,   361,   362,   363,   364,   365,   366,   367,   368,
       0,     0,     0,     0,     0,     0,     0,     0,    11,     0,
       0,   603,     0,     0,     0,     0,  1118,     0,     0,     0,
       0,     0,   323,   324,   325,     0,     0,     0,   603,     0,
       0,   842,     0,   603,   369,   370,     0,     0,   326,   603,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,     0,   348,     0,     0,   842,   848,     0,     0,   740,
     603,   849,     0,   850,   851,   852,   853,   854,   855,   856,
       0,     0,     0,     0,     0,     0,     0,     0,   603,     0,
       0,   638,   243,     0,     0,     0,   371,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   603,     0,     0,
       0,     0,     0,     0,   603,   857,   858,     0,   859,     0,
       0,     0,     0,     0,     0,   934,     0,     0,     0,   936,
       0,     0,     0,  1147,     0,     0,     0,   740,     0,     0,
       0,     0,     0,     0,     0,     4,     5,     6,     7,     8,
       0,   740,     0,     0,     9,     0,  -642,  -642,  -642,  -642,
     340,   341,   342,   343,   344,   345,   346,   347,   487,   348,
       0,     0,   972,     0,     0,     0,   973,     0,    10,   643,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,    60,    61,
      62,     0,     0,     0,  1052,    63,    64,    65,    66,    67,
      68,    69,     0,  1060,     0,     0,     0,     0,    70,     0,
       0,     0,     0,    71,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,    77,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,   754,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,    12,
      13,     0,     0,  1103,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,    48,    49,    50,     0,
      51,    52,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,    60,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,    71,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,    77,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,   847,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,    48,    49,    50,     0,    51,    52,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,    60,    61,
      62,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,    71,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,    77,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,     0,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,    77,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,   473,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,     0,    49,    50,     0,    51,     0,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,     0,    61,
      62,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,    77,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,   607,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,   913,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,    77,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,     0,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,   983,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,     0,    49,    50,     0,    51,     0,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,     0,    61,
      62,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,    77,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,     0,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,    77,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,  1068,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,  1151,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,     0,    49,    50,     0,    51,     0,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,     0,    61,
      62,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,    77,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,     0,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,    77,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,  1168,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,     0,    49,    50,     0,    51,     0,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,     0,    61,
      62,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,    77,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,  1171,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,  1174,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,    77,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,     0,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,     0,    49,    50,     0,    51,     0,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,     0,    61,
      62,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,    77,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,  1176,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,    77,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,  1188,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,     0,    49,    50,     0,    51,     0,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,     0,    61,
      62,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,    77,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,  1216,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,    11,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,    54,    55,    56,     0,
      57,    58,    59,     0,    61,    62,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,    77,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,  1220,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,    11,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,     0,    49,    50,     0,    51,     0,    53,     0,     0,
       0,    54,    55,    56,     0,    57,    58,    59,     0,    61,
      62,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,    75,    76,     0,
       0,     4,     5,     6,     7,     8,    77,     0,    78,     0,
       9,     0,     0,     0,    79,     0,    80,    81,     0,    82,
      83,     0,    84,    85,   698,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,    29,     0,     0,     0,    30,    31,    32,
      33,    34,     0,    35,     0,     0,     0,    36,    37,    38,
      39,     0,    40,     0,    41,     0,    42,     0,     0,    43,
       0,     0,     0,    44,    45,    46,    47,     0,    49,    50,
       0,    51,     0,    53,     0,     0,     0,     0,     0,    56,
       0,    57,    58,    59,     0,     0,     0,     0,     0,     0,
       0,    63,    64,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,     0,    70,     0,     0,     0,     0,   123,
      72,    73,    74,    75,    76,     0,     0,     4,     5,     6,
       7,     8,     0,     0,    78,     0,     9,     0,     0,     0,
      79,     0,    80,    81,     0,    82,    83,     0,    84,    85,
     708,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      10,     0,     0,     0,     0,     0,    12,    13,     0,     0,
       0,     0,    14,     0,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    26,    27,    28,    29,
       0,     0,     0,    30,    31,    32,    33,    34,     0,    35,
       0,     0,     0,    36,    37,    38,    39,     0,    40,     0,
      41,     0,    42,     0,     0,    43,     0,     0,     0,    44,
      45,    46,    47,     0,    49,    50,     0,    51,     0,    53,
       0,     0,     0,     0,     0,    56,     0,    57,    58,    59,
       0,     0,     0,     0,     0,     0,     0,    63,    64,    65,
      66,    67,    68,    69,     0,     0,     0,     0,     0,     0,
      70,     0,     0,     0,     0,   123,    72,    73,    74,    75,
      76,     0,     0,     4,     5,     6,     7,     8,     0,     0,
      78,     0,     9,     0,     0,     0,    79,     0,    80,    81,
       0,    82,    83,     0,    84,    85,   805,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    10,     0,     0,     0,
       0,     0,    12,    13,     0,     0,     0,     0,    14,     0,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,    26,    27,    28,    29,     0,     0,     0,    30,
      31,    32,    33,    34,     0,    35,     0,     0,     0,    36,
      37,    38,    39,     0,    40,     0,    41,     0,    42,     0,
       0,    43,     0,     0,     0,    44,    45,    46,    47,     0,
      49,    50,     0,    51,     0,    53,     0,     0,     0,     0,
       0,    56,     0,    57,    58,    59,     0,     0,     0,     0,
       0,     0,     0,    63,    64,    65,    66,    67,    68,    69,
       0,     0,     0,     0,     0,     0,    70,     0,     0,     0,
       0,   123,    72,    73,    74,    75,    76,     0,     0,     4,
       5,     6,     7,     8,     0,     0,    78,     0,     9,     0,
       0,     0,    79,     0,    80,    81,     0,    82,    83,     0,
      84,    85,  1055,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    10,     0,     0,     0,     0,     0,    12,    13,
       0,     0,     0,     0,    14,     0,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,    26,    27,
      28,    29,     0,     0,     0,    30,    31,    32,    33,    34,
       0,    35,     0,     0,     0,    36,    37,    38,    39,     0,
      40,     0,    41,     0,    42,     0,     0,    43,     0,     0,
       0,    44,    45,    46,    47,     0,    49,    50,     0,    51,
       0,    53,     0,     0,     0,     0,     0,    56,     0,    57,
      58,    59,     0,     0,     0,     0,     0,     0,     0,    63,
      64,    65,    66,    67,    68,    69,     0,     0,     0,     0,
       0,     0,    70,     0,     0,     0,     0,   123,    72,    73,
      74,    75,    76,     0,     0,     4,     5,     6,     7,     8,
       0,     0,    78,     0,     9,     0,     0,     0,    79,     0,
      80,    81,     0,    82,    83,     0,    84,    85,  1107,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,    29,     0,     0,
       0,    30,    31,    32,    33,    34,     0,    35,     0,     0,
       0,    36,    37,    38,    39,     0,    40,     0,    41,     0,
      42,     0,     0,    43,     0,     0,     0,    44,    45,    46,
      47,     0,    49,    50,     0,    51,     0,    53,     0,     0,
       0,     0,     0,    56,     0,    57,    58,    59,     0,     0,
       0,     0,     0,     0,     0,    63,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,    75,    76,     0,
       4,     5,     6,     7,     8,     0,     0,     0,    78,     9,
       0,     0,     0,     0,    79,     0,    80,    81,     0,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,    29,     0,     0,     0,    30,    31,    32,    33,
      34,     0,    35,     0,     0,     0,    36,    37,    38,    39,
       0,    40,     0,    41,     0,    42,     0,     0,    43,     0,
       0,     0,    44,    45,    46,    47,     0,    49,    50,     0,
      51,     0,    53,     0,     0,     0,     0,     0,    56,     0,
      57,    58,    59,     0,     0,     0,     0,     0,     0,     0,
      63,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,    75,    76,     0,     4,     5,     6,     7,     8,
       0,     0,     0,    78,     9,     0,     0,     0,     0,    79,
       0,    80,    81,     0,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,    30,    31,   120,    33,    34,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,   381,   358,   359,   360,
     361,   362,   363,   364,   365,   366,   367,   368,     0,     0,
      47,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   121,     0,     0,    58,    59,     0,     0,
       0,     0,     0,     0,     0,   122,    64,    65,    66,    67,
      68,    69,   369,   370,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,     0,    76,     0,
       4,     5,     6,     7,     8,     0,     0,     0,    78,     9,
       0,     0,     0,     0,    79,     0,   250,     0,     0,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,   371,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,    30,    31,   120,    33,
      34,     0,     0,     0,     0,     0,    36,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,    47,   348,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   121,     0,
       0,    58,    59,     0,     0,     0,     0,     0,     0,     0,
     122,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,     0,    76,     0,     4,     5,     6,     7,     8,
       0,     0,     0,    78,     9,     0,     0,     0,     0,    79,
       0,   253,     0,     0,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,    30,    31,   120,    33,    34,     0,     0,     0,     0,
       0,    36,     0,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
      47,   348,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   121,     0,     0,    58,    59,     0,     0,
       0,     0,     0,     0,     0,   122,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,     0,    76,     0,
       4,     5,     6,     7,     8,     0,     0,     0,    78,     9,
       0,     0,     0,     0,    79,     0,   263,     0,     0,    82,
      83,     0,    84,    85,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,    30,    31,   120,    33,
      34,     0,     0,     0,     0,     0,    36,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   299,     0,     0,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   121,     0,
       0,    58,    59,     0,     0,     0,     0,     0,     0,     0,
     122,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,     0,    76,     0,     4,     5,     6,     7,     8,
       0,     0,     0,    78,     9,     0,     0,     0,     0,    79,
       0,     0,     0,     0,    82,    83,     0,    84,    85,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,    30,    31,   120,    33,    34,     0,     0,     0,     0,
       0,    36,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,     0,   348,
      47,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   121,     0,     0,    58,    59,     0,     0,
       0,     0,     0,     0,     0,   122,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,     0,    76,     0,
       4,     5,     6,     7,     8,     0,     0,     0,    78,     9,
       0,     0,     0,     0,    79,   401,     0,     0,     0,    82,
      83,     0,    84,    85,     0,     0,     0,     0,   450,     0,
       0,     0,     0,    10,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,    26,
      27,    28,     0,     0,     0,     0,    30,    31,   120,    33,
      34,     0,     0,     0,     0,     0,    36,  -642,  -642,  -642,
    -642,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,     0,   348,    47,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   121,     0,
       0,    58,    59,     0,     0,     0,     0,     0,     0,     0,
     122,    64,    65,    66,    67,    68,    69,     0,     0,     0,
       0,     0,     0,    70,     0,     0,     0,     0,   123,    72,
      73,    74,     0,    76,     0,     4,     5,     6,     7,     8,
       0,     0,     0,    78,     9,     0,     0,     0,     0,    79,
       0,     0,     0,     0,    82,    83,     0,    84,    85,     0,
       0,     0,     0,   484,     0,     0,     0,     0,    10,     0,
       0,     0,     0,     0,    12,    13,     0,     0,     0,     0,
      14,     0,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,     0,    26,    27,    28,     0,     0,     0,
       0,    30,    31,   120,    33,    34,     0,     0,     0,     0,
       0,    36,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      47,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   121,     0,     0,    58,    59,     0,     0,
       0,     0,     0,     0,     0,   122,    64,    65,    66,    67,
      68,    69,     0,     0,     0,     0,     0,     0,    70,     0,
       0,     0,     0,   123,    72,    73,    74,     0,    76,     0,
       0,     4,     5,     6,     7,     8,     0,     0,    78,     0,
       9,     0,     0,     0,    79,     0,     0,     0,     0,    82,
      83,     0,    84,    85,   495,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,    30,    31,   120,
      33,    34,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,     0,    58,    59,     0,     0,     0,     0,     0,     0,
       0,   122,    64,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,     0,    70,     0,     0,     0,     0,   123,
      72,    73,    74,     0,    76,     0,     4,     5,     6,     7,
       8,     0,     0,     0,    78,     9,     0,     0,     0,     0,
      79,     0,     0,     0,     0,    82,    83,     0,    84,    85,
       0,     0,     0,     0,   529,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,    30,    31,   120,    33,    34,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   121,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,   122,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   123,    72,    73,    74,     0,    76,
       0,     4,     5,     6,     7,     8,     0,     0,     0,    78,
       9,     0,     0,     0,     0,    79,     0,     0,     0,     0,
      82,    83,     0,    84,    85,     0,     0,     0,     0,   748,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,    30,    31,   120,
      33,    34,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,     0,    58,    59,     0,     0,     0,     0,     0,     0,
       0,   122,    64,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,     0,    70,     0,     0,     0,     0,   123,
      72,    73,    74,     0,    76,     0,     4,     5,     6,     7,
       8,     0,     0,     0,    78,     9,     0,     0,     0,     0,
      79,     0,     0,     0,     0,    82,    83,     0,    84,    85,
       0,     0,     0,     0,   750,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,    30,    31,   120,    33,    34,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   121,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,   122,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   123,    72,    73,    74,     0,    76,
       0,     4,     5,     6,     7,     8,     0,     0,     0,    78,
       9,     0,     0,     0,     0,    79,     0,     0,     0,     0,
      82,    83,     0,    84,    85,     0,     0,     0,     0,   757,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,    30,    31,   120,
      33,    34,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,     0,    58,    59,     0,     0,     0,     0,     0,     0,
       0,   122,    64,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,     0,    70,     0,     0,     0,     0,   123,
      72,    73,    74,     0,    76,     0,     4,     5,     6,     7,
       8,     0,     0,     0,    78,     9,     0,     0,     0,     0,
      79,     0,     0,     0,     0,    82,    83,     0,    84,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,    30,    31,   120,    33,    34,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   121,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,   122,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   123,    72,    73,    74,   841,    76,
       0,     4,     5,     6,     7,     8,     0,     0,     0,    78,
       9,     0,     0,     0,     0,    79,     0,     0,     0,     0,
      82,    83,     0,    84,    85,     0,     0,     0,     0,   935,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,    30,    31,   120,
      33,    34,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,     0,    58,    59,     0,     0,     0,     0,     0,     0,
       0,   122,    64,    65,    66,    67,    68,    69,     0,     0,
       0,     0,     0,     0,    70,     0,     0,     0,     0,   123,
      72,    73,    74,     0,    76,     0,     4,     5,     6,     7,
       8,     0,     0,     0,    78,     9,     0,     0,     0,     0,
      79,     0,     0,     0,     0,    82,    83,     0,    84,    85,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    10,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,    26,    27,    28,     0,     0,
       0,     0,    30,    31,   120,    33,    34,     0,     0,     0,
       0,     0,    36,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    47,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   121,     0,     0,    58,    59,     0,
       0,     0,     0,     0,     0,     0,   122,    64,    65,    66,
      67,    68,    69,     0,     0,     0,     0,     0,     0,    70,
       0,     0,     0,     0,   123,    72,    73,    74,     0,    76,
       0,     4,     5,     6,     7,     8,     0,     0,     0,    78,
       9,     0,     0,     0,     0,    79,     0,     0,     0,     0,
      82,    83,     0,    84,    85,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     0,     0,
      12,    13,     0,     0,     0,     0,    14,     0,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
      26,    27,    28,     0,     0,     0,     0,    30,    31,   120,
     458,    34,     0,     0,     0,     0,     0,    36,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    47,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   121,
       0,     0,    58,    59,     0,     0,     0,     0,     0,     0,
       0,   122,    64,    65,    66,    67,    68,    69,     0,   132,
     133,   134,   135,   136,    70,   137,   138,   139,   140,   123,
      72,    73,    74,     0,    76,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    78,     0,     0,     0,     0,     0,
      79,     0,     0,   556,     0,    82,    83,     0,    84,    85,
       0,     0,   557,     0,     0,   141,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   142,   143,
     144,   145,   146,   147,   148,     0,     0,   149,     0,     0,
       0,     0,     0,     0,     0,     0,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   162,   163,
     164,   165,   166,   167,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,     0,     0,   191,
     192,   193,   194,   195,   196,   197,   132,   133,   134,   135,
     136,     0,   137,   138,   139,   140,     0,   198,   199,   200,
       0,   201,     0,     0,     0,     0,     0,     0,     0,     0,
     202,     0,   203,     0,     0,     0,     0,   204,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   971,
       0,     0,   141,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   142,   143,   144,   145,   146,
     147,   148,     0,     0,   149,     0,     0,     0,     0,     0,
       0,     0,     0,   150,   151,   152,   153,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,     0,     0,   191,   192,   193,   194,
     195,   196,   197,   132,   133,   134,   135,   136,     0,   137,
     138,   139,   140,     0,   198,   199,   200,     0,   201,     0,
       0,     0,     0,     0,     0,     0,     0,   202,     0,   203,
       0,     0,     0,     0,   204,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   141,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,   143,   144,   145,   146,   147,   148,     0,
       0,   149,     0,     0,     0,     0,     0,     0,     0,     0,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,     0,     0,   191,   192,   193,   194,   195,   196,   197,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   198,   199,   200,     0,   201,   323,   324,   325,     0,
       0,     0,     0,     0,   202,     0,   203,     0,     0,     0,
       0,   204,   326,     0,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,     0,   348,   323,   324,   325,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,     0,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,     0,   348,   323,   324,
     325,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   326,     0,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,     0,   348,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   323,   324,   325,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   326,   669,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,     0,   348,   323,   324,   325,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,   672,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,     0,   348,   323,   324,
     325,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   326,   697,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,     0,   348,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   323,   324,   325,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   326,   729,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,     0,   348,   323,   324,   325,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,   779,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,     0,   348,     0,     0,
     323,   324,   325,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   801,   326,  1061,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,     0,
     348,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   323,   324,   325,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   326,  1046,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,     0,   348,   323,   324,   325,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,  1047,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,     0,   348,   323,   324,
     325,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   326,  1062,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,     0,   348,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   323,   324,   325,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     326,   414,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,     0,   348,   323,   324,   325,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   326,   416,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,     0,   348,   323,   324,   325,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   326,   426,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,     0,   348,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   323,   324,   325,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   326,   428,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,     0,   348,   323,   324,   325,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   326,
     469,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,     0,   348,   323,   324,   325,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     326,   777,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,     0,   348,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     323,   324,   325,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   326,  1000,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,     0,
     348,     0,   323,   324,   325,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   552,   326,     0,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,     0,   348,   323,   324,   325,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   575,   326,
       0,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,     0,   348,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   323,   324,   325,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,   576,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,     0,   348,     0,   323,
     324,   325,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   326,   579,   327,   328,   329,
     330,   331,   332,   333,   334,   335,   336,   337,   338,   339,
     340,   341,   342,   343,   344,   345,   346,   347,     0,   348,
     323,   324,   325,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   326,   585,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,   338,
     339,   340,   341,   342,   343,   344,   345,   346,   347,     0,
     348,   597,   598,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   323,   324,   325,     0,     0,     0,     0,    30,    31,
     120,   907,     0,     0,     0,     0,     0,   326,    36,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
       0,   348,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1106,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   599,    65,    66,    67,    68,    69,     0,
       0,     0,     0,     0,     0,   600,     0,     0,     0,     0,
     123,    72,    73,    74,     0,   601,     0,     0,     0,     0,
       0,     0,     0,     0,  1148,    78,     0,   323,   324,   325,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,   628,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,     0,   348,   323,   324,
     325,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   326,     0,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   326,   348,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
       0,   348,     0,     0,   323,   324,   325,     0,     0,     0,
     845,     0,     0,     0,     0,     0,     0,     0,     0,   581,
     326,   653,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,     0,   348,   323,   324,   325,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   326,     0,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,     0,   348,   324,   325,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   326,     0,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   325,   348,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   326,     0,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   340,   341,   342,   343,   344,   345,   346,
     347,     0,   348,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,     0,   348,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,   339,   340,   341,
     342,   343,   344,   345,   346,   347,     0,   348,   329,   330,
     331,   332,   333,   334,   335,   336,   337,   338,   339,   340,
     341,   342,   343,   344,   345,   346,   347,     0,   348
};

static const yytype_int16 yycheck[] =
{
       2,     2,   316,   265,   124,   650,     2,   561,     2,     2,
       2,   386,   255,   256,    77,    27,   864,   260,   374,   348,
      27,   713,    24,    25,   379,   453,   765,   648,   867,   743,
     867,   465,   117,    96,   964,   733,     8,   580,     8,     8,
     396,     8,     8,     8,     8,    52,     8,     8,    26,     8,
       8,    26,     8,     8,     8,     8,    36,     8,     8,   773,
       9,    10,    11,     8,    71,    41,     8,    74,     8,     8,
       8,    61,   594,     8,   596,    90,    25,    79,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,     8,
      49,     0,   102,   121,    90,    73,   351,    78,   162,   957,
      78,    71,  1042,    61,    41,    78,   108,   162,     8,    53,
      29,    55,    56,    57,    58,   424,   121,   372,    61,   144,
     375,   144,    13,    61,    25,   756,    61,    61,   437,    26,
     121,   880,    61,    61,   765,    26,    61,    37,   166,    24,
      25,   164,   165,    61,   399,    90,    43,    71,   144,    73,
      51,   161,    43,   437,   878,   162,   137,   147,   163,    61,
     138,   139,   162,   121,   137,   165,    61,   420,   553,   554,
      71,   157,   163,    61,   829,   165,   146,   165,    61,   164,
     712,    61,   837,   163,   166,   164,   165,   164,   164,   164,
     164,    61,   164,   164,   849,   164,   164,   641,   164,   164,
     164,   164,   163,   163,    26,   140,    61,   838,   163,   221,
     169,   163,   229,   163,   163,   163,    25,   919,   162,   231,
     157,   929,   165,   124,   162,   355,   163,   165,   162,    61,
     165,   165,   244,   797,   162,    61,   165,   165,   244,   870,
     165,   965,    51,   167,   163,   146,   319,   165,   879,   880,
     267,    24,    25,   691,    27,   808,  1196,   352,   815,   624,
     272,   162,    71,   165,    71,  1205,    73,   279,   280,   281,
     165,    25,    71,   582,   286,   147,   371,   165,   295,   162,
     292,   354,   165,   356,    73,   165,   941,    73,   597,   598,
     656,   657,    71,   165,   306,   165,   391,    51,   582,   665,
     395,   667,   162,   398,   306,   121,  1155,    71,  1155,    71,
     165,    73,    73,   597,   598,   124,   162,    71,   162,  1043,
      73,  1179,    31,   662,   162,    78,   348,     8,    25,    71,
     352,   348,   896,   165,   898,   966,   221,   146,   647,   165,
      31,   594,   162,   596,    98,   144,   231,   163,  1003,   371,
      71,   115,   165,   162,    51,   793,    37,   914,    25,   916,
     167,    93,    94,   647,    73,   117,   165,   379,    31,   391,
     124,   117,    71,   395,    71,   392,   398,    31,   167,    73,
      71,   167,    73,   162,    51,   138,   139,   272,   826,    93,
      94,    31,   146,    36,   279,   280,   281,    71,   162,   784,
     785,   286,   144,   165,    71,   167,   167,   292,   162,  1040,
      73,   162,   721,   430,  1163,   427,    71,    73,   171,    73,
     111,   730,    78,    71,    73,   427,   698,   124,    71,    78,
      71,    98,   162,    73,   166,  1184,   708,   721,   450,   106,
     107,   108,   109,   110,   111,   162,   730,   146,   221,   146,
     141,   150,   162,   144,   527,   146,    73,   124,   231,   712,
     713,    78,   166,   162,   717,   162,   162,   141,  1099,  1124,
     144,   162,   484,   162,    26,   840,   167,    41,    63,   146,
     164,   165,   138,   139,  1048,    71,    71,    73,    73,   138,
     139,    43,   166,   141,   379,   162,   144,   165,   146,   272,
      71,    73,    73,   812,   162,   140,   279,   280,   281,   144,
      25,   764,   140,   286,   170,    61,   144,   529,   884,   292,
     137,   138,   139,    31,   848,   111,   111,    42,   812,    61,
      45,    61,  1163,   805,   140,    99,   140,  1192,   144,   424,
     104,   162,   106,   107,   108,   109,   110,   111,   112,    66,
      67,  1206,   437,  1184,    73,   141,   141,    73,   144,   144,
     146,   146,    78,   162,   576,   450,   138,   139,   821,   581,
     576,   162,   584,    25,    61,   348,   162,   162,   584,   165,
      40,   167,   167,    73,   148,   149,   144,   151,    78,    71,
      42,    49,    41,    45,   165,    71,   167,    73,    74,   484,
      66,    67,   166,   615,   140,   622,   379,    69,    70,    71,
     121,   864,   624,   625,   923,   140,   925,    41,   137,   138,
     139,   137,   138,   139,    92,    93,    94,   140,   640,   640,
     954,  1016,   144,   650,   640,   140,   640,   640,   640,   923,
     662,   925,  1077,  1078,   529,   662,   121,    13,   138,   139,
      99,    92,    93,    94,    71,   104,   928,   106,   107,   108,
     109,   110,   111,   112,   162,   164,   919,   920,   106,   107,
     108,   109,   110,   111,    71,    99,    36,   450,    13,   163,
     104,   164,   106,   107,   108,   109,   110,   111,   112,   701,
     163,   703,    71,  1002,   168,   701,   581,   582,   162,   148,
     149,    37,   151,    98,     8,   958,   162,    82,   961,     8,
     164,   484,   597,   598,   109,   110,    13,   166,  1002,    36,
     737,     8,   117,   118,   148,   149,   164,   151,    71,  1038,
     615,    45,    46,    47,    73,    49,   748,   164,   750,   624,
     625,    71,   166,   755,   755,   757,    83,    84,    85,   755,
     767,   755,   755,   755,  1038,   119,   529,  1066,   162,   154,
       8,   163,   647,  1072,    71,    61,   165,    61,    42,    43,
     163,     8,    13,   119,  1083,   169,   162,   169,  1050,  1088,
     162,   162,  1066,  1055,   169,  1094,   163,    61,    13,   169,
     802,  1063,    71,   163,   162,    69,    70,    71,   810,  1083,
     802,   813,   169,    71,  1088,    79,    36,    26,   581,   162,
    1094,   813,   829,   140,   140,    71,  1125,   834,   703,  1091,
     837,   951,   102,    13,  1096,    37,   164,    13,   840,   169,
     162,   165,   849,    13,  1143,  1107,   721,   120,   162,     8,
    1112,  1125,   615,  1115,   165,   730,   164,  1156,   163,   122,
     124,   624,   625,  1162,    13,   163,     8,   163,    71,  1143,
    1169,   863,   136,   748,    71,   750,   106,   107,   108,   109,
     110,   111,   757,   137,  1146,    71,   162,    53,  1162,    55,
      56,    57,    58,   161,   144,  1169,   289,   144,     8,   662,
     293,   162,     8,   905,   163,    71,   908,   163,   165,   905,
     163,  1173,  1032,   165,   162,   308,   908,   310,   311,   312,
     313,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,   935,   941,   810,   169,   812,   163,   163,
     703,   122,     8,   137,   163,    40,   112,  1209,    71,    26,
      73,   164,   164,   163,   122,   163,  1218,     8,   165,     8,
     166,   150,    98,    26,   956,   840,  1029,    59,    60,    42,
      43,    44,    45,    46,    47,   141,    49,   165,   144,   163,
     146,   163,    73,   985,   162,   748,   152,   750,   111,   985,
     164,   163,    13,   164,   757,   997,  1003,   164,     4,     5,
     123,     7,     8,     9,    73,   997,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,   141,   104,
      26,   144,    13,   146,   163,    73,  1089,    26,   163,   121,
      41,    37,   122,   162,    68,   122,   164,   164,    44,   162,
      46,   163,   163,    49,   167,    51,   165,   810,   923,    13,
     925,    13,  1054,   162,   165,    13,    73,   165,  1054,   164,
     935,   164,   146,   165,  1071,   162,    73,    13,    90,    75,
     140,   163,   165,    79,    90,    51,    29,   840,    13,   424,
     162,    73,    13,    26,   164,    71,   164,   155,    99,    71,
      73,   153,   437,   104,    73,   106,   107,   108,   109,   110,
     111,   112,  1104,   163,  1106,   164,   163,   162,  1110,   163,
    1106,  1113,  1104,   165,  1191,   430,   356,  1124,  1110,   353,
     108,  1113,   128,   661,   775,   354,   658,  1002,  1192,  1060,
     767,   859,   810,   435,    71,  1124,    73,   148,   149,  1141,
     151,  1018,  1211,  1145,  1030,    37,    79,   782,  1150,  1141,
    1152,  1153,  1016,  1145,   776,   166,   681,   870,  1150,   693,
    1152,  1153,   794,  1038,   280,   961,   740,    -1,    -1,    -1,
    1172,    -1,   935,    -1,   111,    -1,    71,    -1,    73,    -1,
    1172,    -1,    -1,    -1,    -1,  1192,   123,  1189,    71,    -1,
      73,  1066,    -1,    -1,    -1,    -1,    -1,  1189,    -1,  1206,
      -1,    -1,    -1,    -1,   141,    -1,    -1,   144,  1083,   146,
      -1,  1213,    -1,  1088,    -1,    -1,   111,  1219,    -1,  1094,
      -1,  1213,    -1,    -1,    -1,   162,    -1,  1219,   111,    -1,
     167,    -1,   238,    -1,   240,    -1,    -1,   582,    -1,   245,
     246,   247,    -1,   249,    -1,    -1,   141,    -1,    -1,   144,
    1125,   146,   597,   598,    -1,    -1,    -1,    -1,   141,    -1,
      -1,   144,    -1,   146,    -1,    -1,    -1,   162,  1143,    -1,
      -1,    -1,   167,    -1,    -1,    -1,    -1,   622,    -1,   162,
      -1,   287,    -1,    -1,   167,   291,    -1,  1162,    -1,    -1,
      -1,    -1,    -1,    -1,  1169,    -1,    -1,    -1,    -1,    -1,
      -1,   307,   647,    -1,    -1,   650,    -1,    -1,    -1,   315,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,    -1,    -1,   350,   351,    -1,    -1,    -1,    -1,
      -1,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,    -1,    -1,    41,   372,   373,   374,   375,
     376,    -1,    -1,    -1,    -1,   381,   721,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,   730,    -1,    -1,    -1,    -1,
     396,    -1,    -1,   399,   400,    -1,    -1,    25,   404,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,   767,    99,    -1,    -1,    -1,   433,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    69,    70,    -1,    -1,    -1,    -1,   812,    -1,    -1,
      -1,    79,   148,   149,    31,   151,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,   829,    42,    43,    -1,    -1,   495,
     166,    48,   837,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,   849,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,   125,   126,   127,
     128,   129,    79,    -1,   530,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,    -1,   142,   143,   164,    -1,   544,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,   156,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,   571,   123,   124,   125,   126,
     127,   128,   129,    -1,   580,    -1,    -1,    -1,   923,   136,
     925,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,    -1,    -1,    -1,    -1,    -1,   941,    -1,    -1,   156,
      -1,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
     167,   168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   628,    -1,   630,    -1,    -1,    -1,    -1,    -1,
      -1,   637,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     646,    -1,    -1,    -1,     9,    10,    11,   653,    -1,    -1,
     656,   657,    -1,    -1,    -1,    -1,    -1,  1002,  1003,   665,
      25,   667,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,  1038,   700,    -1,    -1,    -1,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    -1,
      -1,  1066,    -1,    -1,    -1,    -1,  1071,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,  1083,    -1,
      -1,   747,    -1,  1088,    59,    60,    -1,    -1,    25,  1094,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,   781,    99,    -1,    -1,  1124,
    1125,   104,    -1,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1143,    -1,
      -1,   166,   808,    -1,    -1,    -1,   121,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1162,    -1,    -1,
      -1,    -1,    -1,    -1,  1169,   148,   149,    -1,   151,    -1,
      -1,    -1,    -1,    -1,    -1,   841,    -1,    -1,    -1,   845,
      -1,    -1,    -1,   166,    -1,    -1,    -1,  1192,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
      -1,  1206,    -1,    -1,    12,    -1,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   884,    49,
      -1,    -1,   888,    -1,    -1,    -1,   892,    -1,    36,   166,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,   980,   123,   124,   125,   126,   127,
     128,   129,    -1,   989,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,   154,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,   166,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,  1049,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,   154,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,   166,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    99,   100,   101,    -1,   103,   104,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,   154,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,   154,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,   166,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,   154,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,   166,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    89,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,   154,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    83,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,   154,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,   154,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,   166,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    87,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,   154,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,   154,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,   166,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,   154,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,   166,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    85,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,   154,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,   154,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,   166,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,   154,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,   166,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,   154,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,   166,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,   154,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,   166,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,   109,   110,   111,    -1,   113,   114,   115,    -1,   117,
     118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
      -1,     3,     4,     5,     6,     7,   154,    -1,   156,    -1,
      12,    -1,    -1,    -1,   162,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    63,    64,    65,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,
      82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,
      -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,   101,
      -1,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,   111,
      -1,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,   145,   146,    -1,    -1,     3,     4,     5,
       6,     7,    -1,    -1,   156,    -1,    12,    -1,    -1,    -1,
     162,    -1,   164,   165,    -1,   167,   168,    -1,   170,   171,
      26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    63,    64,    65,
      -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,    75,
      -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,
      86,    -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,
      96,    97,    98,    -1,   100,   101,    -1,   103,    -1,   105,
      -1,    -1,    -1,    -1,    -1,   111,    -1,   113,   114,   115,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,
     126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,
     146,    -1,    -1,     3,     4,     5,     6,     7,    -1,    -1,
     156,    -1,    12,    -1,    -1,    -1,   162,    -1,   164,   165,
      -1,   167,   168,    -1,   170,   171,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,
      -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,    -1,
      -1,   111,    -1,   113,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,    -1,     3,
       4,     5,     6,     7,    -1,    -1,   156,    -1,    12,    -1,
      -1,    -1,   162,    -1,   164,   165,    -1,   167,   168,    -1,
     170,   171,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    -1,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,    -1,    -1,    -1,    -1,    -1,   111,    -1,   113,
     114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,    -1,     3,     4,     5,     6,     7,
      -1,    -1,   156,    -1,    12,    -1,    -1,    -1,   162,    -1,
     164,   165,    -1,   167,   168,    -1,   170,   171,    26,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    65,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    75,    -1,    -1,
      -1,    79,    80,    81,    82,    -1,    84,    -1,    86,    -1,
      88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,
      98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,
      -1,    -1,    -1,   111,    -1,   113,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,   145,   146,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,   165,    -1,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,    -1,    -1,   111,    -1,
     113,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,   165,    -1,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    59,    60,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,    -1,    -1,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,   121,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    98,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,   164,    -1,    -1,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      98,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,    -1,   164,    -1,    -1,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    95,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,   156,    12,
      -1,    -1,    -1,    -1,   162,   163,    -1,    -1,    -1,   167,
     168,    -1,   170,   171,    -1,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,    -1,   146,    -1,     3,     4,     5,     6,     7,
      -1,    -1,    -1,   156,    12,    -1,    -1,    -1,    -1,   162,
      -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    36,    -1,
      -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,
      48,    -1,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    -1,    62,    63,    64,    -1,    -1,    -1,
      -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
      -1,     3,     4,     5,     6,     7,    -1,    -1,   156,    -1,
      12,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,   167,
     168,    -1,   170,   171,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,   156,    12,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,   156,
      12,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
     167,   168,    -1,   170,   171,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,   156,    12,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,   156,
      12,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
     167,   168,    -1,   170,   171,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,   156,    12,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,   156,
      12,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
     167,   168,    -1,   170,   171,    -1,    -1,    -1,    -1,    31,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,    -1,
      -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,
     142,   143,   144,    -1,   146,    -1,     3,     4,     5,     6,
       7,    -1,    -1,    -1,   156,    12,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    -1,    -1,   167,   168,    -1,   170,   171,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,     3,     4,     5,     6,     7,    -1,    -1,    -1,   156,
      12,    -1,    -1,    -1,    -1,   162,    -1,    -1,    -1,    -1,
     167,   168,    -1,   170,   171,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
      62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,
      -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   123,   124,   125,   126,   127,   128,   129,    -1,     3,
       4,     5,     6,     7,   136,     9,    10,    11,    12,   141,
     142,   143,   144,    -1,   146,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,    -1,
     162,    -1,    -1,    37,    -1,   167,   168,    -1,   170,   171,
      -1,    -1,    46,    -1,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    66,    67,    68,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    -1,   141,   142,   143,
      -1,   145,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     154,    -1,   156,    -1,    -1,    -1,    -1,   161,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      -1,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,    66,
      67,    68,    -1,    -1,    71,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    -1,   141,   142,   143,    -1,   145,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   154,    -1,   156,
      -1,    -1,    -1,    -1,   161,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    -1,
      -1,    71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   141,   142,   143,    -1,   145,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,   154,    -1,   156,    -1,    -1,    -1,
      -1,   161,    25,    -1,    27,    28,    29,    30,    31,    32,
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
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   166,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   166,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   166,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   166,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   166,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   166,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   166,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   166,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   164,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   164,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,   164,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   164,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   164,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     164,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   164,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   164,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   163,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   163,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   163,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   163,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    42,    43,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    69,    70,
      71,   163,    -1,    -1,    -1,    -1,    -1,    25,    79,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   163,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   163,   156,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   122,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    25,    49,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
     122,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    11,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   173,   174,     0,     3,     4,     5,     6,     7,    12,
      36,    41,    42,    43,    48,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    62,    63,    64,    65,
      69,    70,    71,    72,    73,    75,    79,    80,    81,    82,
      84,    86,    88,    91,    95,    96,    97,    98,    99,   100,
     101,   103,   104,   105,   109,   110,   111,   113,   114,   115,
     116,   117,   118,   123,   124,   125,   126,   127,   128,   129,
     136,   141,   142,   143,   144,   145,   146,   154,   156,   162,
     164,   165,   167,   168,   170,   171,   175,   180,   181,   182,
     183,   186,   197,   198,   201,   208,   214,   275,   276,   278,
     279,   280,   283,   301,   302,   304,   309,   312,   327,   329,
     330,   331,   332,   333,   334,   335,   336,   338,   347,   349,
      71,   111,   123,   141,   197,   275,   329,   275,   162,   275,
     275,   275,     3,     4,     5,     6,     7,     9,    10,    11,
      12,    49,    62,    63,    64,    65,    66,    67,    68,    71,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   123,   124,   125,   126,   127,   128,   129,   141,   142,
     143,   145,   154,   156,   161,   294,   300,   325,   326,   275,
     275,   275,   275,   275,   275,   275,   275,   275,   275,   275,
     275,   162,   181,   302,   329,   329,   275,   111,   141,   144,
     146,   162,   180,   302,   305,   334,   339,   340,   162,   306,
     162,    26,   273,   275,   188,   162,   162,   162,   162,   162,
     164,   275,    71,   164,   275,    25,    51,    71,   124,   146,
     162,   350,   357,   164,   275,   165,   275,   144,   178,   179,
     180,    73,   167,   238,   239,   117,   117,    73,   240,   162,
     162,   162,   162,    71,   212,   351,   162,   162,    73,    78,
     137,   138,   139,   344,   345,   144,   165,   180,   180,    95,
     275,   213,   351,   275,   276,   329,   184,   165,    78,   307,
     344,    78,   344,   344,   144,   162,     8,   164,    31,   196,
     146,   211,   351,     9,    10,    11,    25,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    49,   164,
      61,    61,   140,   118,   154,   197,   214,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    59,
      60,   121,    61,   165,   162,    61,   165,   167,   335,   162,
     196,    13,   275,    26,    43,   284,   288,    40,    71,   324,
     329,   140,   144,   180,   329,   140,   162,   308,   121,    61,
     165,   163,   275,   275,     8,   164,   186,   275,   273,   274,
     275,    71,   226,   275,   164,   164,   164,    71,   357,   357,
      36,   352,    98,   357,    13,    71,   164,   184,   164,   180,
       8,   164,    90,   165,   329,     8,   164,    13,     8,   164,
     329,   348,   348,   329,   163,    36,   206,   123,   329,   341,
      31,   275,   342,   343,    61,   121,   137,   345,    72,   275,
     329,    78,   137,   345,   180,   177,   164,   165,   164,   164,
     209,   163,   163,   166,   185,   186,   198,   201,   208,   275,
     168,   170,   171,    71,    31,   236,   237,   275,   350,   162,
     351,   202,   275,   275,   275,    26,   275,   275,   275,   275,
     275,   275,   275,   275,   275,   275,   275,   275,   275,   275,
     275,   275,   275,   275,   275,   275,   275,   275,   305,   275,
     337,   337,    71,   334,   335,   212,   213,   196,   211,    31,
     145,   275,   275,   275,   275,   275,   275,   275,   275,   275,
     275,   275,   275,    71,   165,   334,   337,   275,   236,   337,
     275,   341,   163,   295,   296,    37,    37,    46,   291,   297,
     300,   162,   323,     8,   314,    71,   180,   163,   334,   236,
      71,   165,   334,   337,   275,   163,   163,   275,    82,   163,
     164,    90,    13,     8,   163,   163,    71,   353,   357,    53,
      55,    56,    57,    58,   162,   358,     8,    42,    43,   124,
     136,   146,   181,   182,   309,   310,   311,   166,    90,   179,
      71,   275,   239,   310,    73,     8,   163,   163,   163,   164,
      71,   356,   119,   217,   162,     8,   163,   329,   122,   163,
       8,   314,    71,    73,    74,   346,    71,    61,   166,   166,
     174,   176,   165,   166,   329,   163,     8,    13,   277,   199,
     119,   215,   275,    26,   169,   169,   162,   162,   207,   210,
     351,   204,    63,   329,   275,   162,   275,   162,   169,   166,
     163,   169,   166,   163,   300,   300,   285,    13,    26,    43,
      42,    43,    61,    69,    70,    71,    79,   124,   136,   316,
     318,   321,   322,    71,   163,   275,   169,   166,    26,   186,
     162,   187,   274,    31,   221,   329,   310,    71,    26,   186,
     225,   190,    36,     8,    37,   354,   357,    26,   354,   310,
     310,   162,    78,   140,   140,   102,   193,   194,    71,   166,
      13,   329,   164,     8,    90,    37,   141,   144,   146,   180,
     182,   218,   303,   165,   341,   123,   329,    13,    31,   275,
      31,   275,   169,   275,   166,   174,   241,    31,   275,   310,
     157,   234,   235,   327,   328,   162,   303,   120,   216,   275,
     236,   236,   217,   165,   200,   215,   305,   164,   236,   166,
     236,    13,   290,   289,   298,   299,    69,    70,    71,   317,
     317,   318,   319,   320,   162,    78,   137,     8,   314,   163,
     323,   166,   184,   230,   275,    26,   186,   224,   164,   329,
     122,   220,    13,   184,    26,   165,   227,   354,   357,   353,
     163,     8,   357,   163,   310,   313,   315,    71,    71,   162,
     161,   310,   356,    71,   144,   180,   352,     8,   241,   163,
     162,   145,   275,   329,   329,   122,   169,   166,    99,   104,
     106,   107,   108,   109,   110,   111,   112,   148,   149,   151,
     166,   242,   267,   268,   269,   270,   272,   327,   329,   163,
       8,   357,   359,   234,   218,   165,   163,   163,   165,   241,
     162,   216,   308,   163,   162,   163,    36,   147,   165,   283,
     293,   147,   165,   292,   300,   300,   122,   169,     8,   314,
     319,   137,   318,   231,    66,    67,   232,   163,   184,   274,
     221,   163,   310,    89,   164,   228,   164,   228,    40,     8,
      26,   354,   163,   122,   163,     8,   314,   303,   165,     8,
     180,   303,   166,   341,   275,    31,   275,   166,   350,   219,
     303,   247,   150,   260,   261,    71,   115,   162,   262,   263,
     243,   197,   270,   357,     8,   164,   268,   269,    26,   355,
     157,   328,    31,    73,   163,   203,   241,   166,   234,   165,
     236,    46,   275,   275,   318,   318,   163,    66,    67,   233,
     162,   186,   164,    83,   163,   191,   164,   228,    92,    93,
      94,   228,   166,   357,   310,   310,    73,   184,   356,   163,
     164,   329,    13,     8,   164,   165,    53,    55,    56,    57,
      58,   112,   146,   152,   252,   254,   255,   303,     8,   164,
      71,   146,   150,   263,   264,   265,   164,    73,   271,   196,
     244,   350,   197,   357,   104,   281,   359,    73,    13,   355,
     241,   166,   163,   205,   163,   286,   166,   166,   122,   162,
      26,    68,   275,   164,   189,    26,   186,   223,    92,   164,
     275,    26,   164,   229,   166,   163,   122,   163,   166,   310,
     303,   248,   165,     8,   164,   297,   261,     8,    29,   163,
      25,    42,    45,    13,     8,   164,   351,   271,    13,   196,
     162,   165,    31,    73,    13,   310,   165,   166,   355,   241,
     146,   287,   318,   275,   184,   164,   163,    26,   186,   222,
     184,   164,   229,   184,   310,   165,    71,   166,   182,   249,
     250,   251,   256,   309,   253,    13,   257,   264,   264,    25,
      42,    45,   310,    73,   162,   164,   310,   351,    31,    73,
     282,   184,    73,    13,   310,   184,   165,   166,   163,   186,
     184,    87,   184,   184,   140,    90,     8,   166,   254,   310,
      51,   258,    13,   245,   162,    73,     8,   163,   166,    13,
     310,   166,   184,    26,    85,   164,   166,    71,   268,   269,
     309,   259,   310,   234,   246,    31,    73,   310,   166,   184,
     164,   192,   155,    71,   164,   153,   163,   234,    73,   102,
     193,   195,   219,   164,   355,   163,   162,   164,   164,   165,
     266,   355,   303,   184,   266,    73,   166,   163,   165,   184,
     166
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
#line 998 "../../../src/util/parser/hphp.y"
    { _p->popLabelInfo();
                                         _p->saveParseTree((yyval));;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 1004 "../../../src/util/parser/hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 1005 "../../../src/util/parser/hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 1008 "../../../src/util/parser/hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num() == T_DECLARE);
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 1010 "../../../src/util/parser/hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 1011 "../../../src/util/parser/hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 1012 "../../../src/util/parser/hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 1013 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 1014 "../../../src/util/parser/hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());
                                         (yyval).reset();;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 1016 "../../../src/util/parser/hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 1017 "../../../src/util/parser/hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 1018 "../../../src/util/parser/hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 1019 "../../../src/util/parser/hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 1020 "../../../src/util/parser/hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 1021 "../../../src/util/parser/hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 1027 "../../../src/util/parser/hphp.y"
    { ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 1028 "../../../src/util/parser/hphp.y"
    { ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 1031 "../../../src/util/parser/hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 1032 "../../../src/util/parser/hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 1033 "../../../src/util/parser/hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 1035 "../../../src/util/parser/hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 1038 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 1040 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 1043 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);
                                         (yyval).setText(_p->resolve((yyval).text(),0));;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 1045 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 1047 "../../../src/util/parser/hphp.y"
    { (yyval).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 1050 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);
                                         (yyval).setText(_p->resolve((yyval).text(),1));;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 1052 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 1054 "../../../src/util/parser/hphp.y"
    { (yyval).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 1058 "../../../src/util/parser/hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 1060 "../../../src/util/parser/hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),  0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 1066 "../../../src/util/parser/hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 1067 "../../../src/util/parser/hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 1070 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 1071 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 1072 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 1073 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 1076 "../../../src/util/parser/hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 1080 "../../../src/util/parser/hphp.y"
    { _p->onIf((yyval),(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(6) - (7)]),(yyvsp[(7) - (7)]));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 1085 "../../../src/util/parser/hphp.y"
    { _p->onIf((yyval),(yyvsp[(3) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(8) - (10)]));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 1086 "../../../src/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 1087 "../../../src/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 1090 "../../../src/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 1091 "../../../src/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 1094 "../../../src/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 1095 "../../../src/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 1097 "../../../src/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 1098 "../../../src/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 1100 "../../../src/util/parser/hphp.y"
    { _p->onBreak((yyval), NULL);;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 1101 "../../../src/util/parser/hphp.y"
    { _p->onBreak((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 1102 "../../../src/util/parser/hphp.y"
    { _p->onContinue((yyval), NULL);;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 1103 "../../../src/util/parser/hphp.y"
    { _p->onContinue((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 1104 "../../../src/util/parser/hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 1105 "../../../src/util/parser/hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 1106 "../../../src/util/parser/hphp.y"
    { _p->onYield((yyval), NULL, false);;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 1107 "../../../src/util/parser/hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (3)]), false);;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 1108 "../../../src/util/parser/hphp.y"
    { on_yield_assign(_p, (yyval), (yyvsp[(1) - (5)]), &(yyvsp[(4) - (5)]));;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 1110 "../../../src/util/parser/hphp.y"
    { on_yield_list_assign(_p, (yyval), (yyvsp[(3) - (8)]), &(yyvsp[(7) - (8)]));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 1111 "../../../src/util/parser/hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 1112 "../../../src/util/parser/hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 1113 "../../../src/util/parser/hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 1114 "../../../src/util/parser/hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 1115 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 1116 "../../../src/util/parser/hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 1119 "../../../src/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 1120 "../../../src/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 1124 "../../../src/util/parser/hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 1132 "../../../src/util/parser/hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 1135 "../../../src/util/parser/hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 1136 "../../../src/util/parser/hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 1137 "../../../src/util/parser/hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 1141 "../../../src/util/parser/hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 1142 "../../../src/util/parser/hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 1154 "../../../src/util/parser/hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 1155 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 1159 "../../../src/util/parser/hphp.y"
    { finally_statement(_p);;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 1161 "../../../src/util/parser/hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (5)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 1166 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 1170 "../../../src/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 1171 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 1175 "../../../src/util/parser/hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 1180 "../../../src/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 1185 "../../../src/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onFunction((yyval),t,(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 1190 "../../../src/util/parser/hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 1195 "../../../src/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onFunction((yyval),t,(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 1203 "../../../src/util/parser/hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 1205 "../../../src/util/parser/hphp.y"
    { if (_p->peekClass())
                                           _p->scanner().xhpStatement();;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 1207 "../../../src/util/parser/hphp.y"
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[(8) - (9)]));
                                         } else {
                                           stmts = (yyvsp[(8) - (9)]);
                                         }
                                         _p->onClass((yyval),(yyvsp[(1) - (9)]).num(),(yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]),(yyvsp[(5) - (9)]),
                                                     stmts,0);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                           _p->scanner().xhpReset();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1223 "../../../src/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1225 "../../../src/util/parser/hphp.y"
    { if (_p->peekClass())
                                           _p->scanner().xhpStatement();;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1227 "../../../src/util/parser/hphp.y"
    { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,(yyvsp[(9) - (10)]));
                                         } else {
                                           stmts = (yyvsp[(9) - (10)]);
                                         }
                                         _p->onClass((yyval),(yyvsp[(2) - (10)]).num(),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(6) - (10)]),
                                                     stmts,&(yyvsp[(1) - (10)]));
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                           _p->scanner().xhpReset();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1242 "../../../src/util/parser/hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1245 "../../../src/util/parser/hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1250 "../../../src/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1253 "../../../src/util/parser/hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 1260 "../../../src/util/parser/hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1262 "../../../src/util/parser/hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (6)]),t_ext,t_imp,
                                                     (yyvsp[(5) - (6)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1270 "../../../src/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1272 "../../../src/util/parser/hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (7)]),t_ext,t_imp,
                                                     (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1280 "../../../src/util/parser/hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1281 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1285 "../../../src/util/parser/hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1288 "../../../src/util/parser/hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1291 "../../../src/util/parser/hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1292 "../../../src/util/parser/hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1293 "../../../src/util/parser/hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1297 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1298 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1301 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1302 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1305 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1306 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1309 "../../../src/util/parser/hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1311 "../../../src/util/parser/hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1314 "../../../src/util/parser/hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1316 "../../../src/util/parser/hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1320 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1321 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1324 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1325 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1329 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1331 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1334 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1336 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1339 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1341 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1344 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1346 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1356 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1357 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1358 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1359 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1364 "../../../src/util/parser/hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1366 "../../../src/util/parser/hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1367 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1370 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1371 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1376 "../../../src/util/parser/hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1377 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1382 "../../../src/util/parser/hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(7) - (7)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1383 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1386 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1387 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1390 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1391 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1396 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1397 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1398 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1399 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1404 "../../../src/util/parser/hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,NULL,&(yyvsp[(1) - (3)]));;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1406 "../../../src/util/parser/hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,NULL,&(yyvsp[(1) - (4)]));;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1409 "../../../src/util/parser/hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,&(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1412 "../../../src/util/parser/hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,&(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]));;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1415 "../../../src/util/parser/hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,NULL,&(yyvsp[(3) - (5)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1418 "../../../src/util/parser/hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,NULL,&(yyvsp[(3) - (6)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1422 "../../../src/util/parser/hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,&(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1426 "../../../src/util/parser/hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,&(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1430 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1431 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1434 "../../../src/util/parser/hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1435 "../../../src/util/parser/hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1437 "../../../src/util/parser/hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1439 "../../../src/util/parser/hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1443 "../../../src/util/parser/hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1444 "../../../src/util/parser/hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1447 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1448 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1449 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1453 "../../../src/util/parser/hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1455 "../../../src/util/parser/hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1456 "../../../src/util/parser/hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1457 "../../../src/util/parser/hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1462 "../../../src/util/parser/hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
                                         if (_p->peekClass())
                                           _p->scanner().xhpStatement();;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1465 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1468 "../../../src/util/parser/hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1469 "../../../src/util/parser/hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1472 "../../../src/util/parser/hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1473 "../../../src/util/parser/hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1475 "../../../src/util/parser/hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1479 "../../../src/util/parser/hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1484 "../../../src/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onMethod((yyval),(yyvsp[(1) - (10)]),t,(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1491 "../../../src/util/parser/hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1496 "../../../src/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onMethod((yyval),(yyvsp[(2) - (11)]),t,(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1500 "../../../src/util/parser/hphp.y"
    { _p->scanner().xhpAttributeDecl();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1501 "../../../src/util/parser/hphp.y"
    { _p->xhpSetAttributes((yyvsp[(3) - (4)]));;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1503 "../../../src/util/parser/hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1505 "../../../src/util/parser/hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1506 "../../../src/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1509 "../../../src/util/parser/hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1512 "../../../src/util/parser/hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1513 "../../../src/util/parser/hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1514 "../../../src/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1520 "../../../src/util/parser/hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)])); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1524 "../../../src/util/parser/hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1527 "../../../src/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1534 "../../../src/util/parser/hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1535 "../../../src/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1539 "../../../src/util/parser/hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1541 "../../../src/util/parser/hphp.y"
    { _p->scanner().xhpAttributeDecl();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1542 "../../../src/util/parser/hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1549 "../../../src/util/parser/hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1551 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1555 "../../../src/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1556 "../../../src/util/parser/hphp.y"
    { (yyval) = 2;;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1557 "../../../src/util/parser/hphp.y"
    { (yyval) = 3;;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1558 "../../../src/util/parser/hphp.y"
    { (yyval) = 4;;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1559 "../../../src/util/parser/hphp.y"
    { (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1560 "../../../src/util/parser/hphp.y"
    { (yyval) = 6;;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1562 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1563 "../../../src/util/parser/hphp.y"
    { (yyval) = 8;;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1567 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1569 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1573 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1574 "../../../src/util/parser/hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1578 "../../../src/util/parser/hphp.y"
    { _p->scanner().xhpAttributeDecl();;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1579 "../../../src/util/parser/hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1580 "../../../src/util/parser/hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1584 "../../../src/util/parser/hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1587 "../../../src/util/parser/hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1592 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1597 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1598 "../../../src/util/parser/hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1600 "../../../src/util/parser/hphp.y"
    { (yyval) = 0;;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1604 "../../../src/util/parser/hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1605 "../../../src/util/parser/hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1606 "../../../src/util/parser/hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1607 "../../../src/util/parser/hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1611 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1612 "../../../src/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1613 "../../../src/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1614 "../../../src/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1615 "../../../src/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1617 "../../../src/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1619 "../../../src/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1623 "../../../src/util/parser/hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1626 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1627 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1631 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1632 "../../../src/util/parser/hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1635 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1636 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1639 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1640 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1643 "../../../src/util/parser/hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1645 "../../../src/util/parser/hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1648 "../../../src/util/parser/hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1649 "../../../src/util/parser/hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1650 "../../../src/util/parser/hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1651 "../../../src/util/parser/hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1652 "../../../src/util/parser/hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1653 "../../../src/util/parser/hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1657 "../../../src/util/parser/hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1659 "../../../src/util/parser/hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1660 "../../../src/util/parser/hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1661 "../../../src/util/parser/hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1665 "../../../src/util/parser/hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1666 "../../../src/util/parser/hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1670 "../../../src/util/parser/hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1671 "../../../src/util/parser/hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1675 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1676 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1680 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1681 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1685 "../../../src/util/parser/hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1686 "../../../src/util/parser/hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1687 "../../../src/util/parser/hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1690 "../../../src/util/parser/hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1692 "../../../src/util/parser/hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1693 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1694 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1695 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1696 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1697 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1698 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1699 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1700 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1701 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1702 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1703 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1704 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1705 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1706 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1707 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1708 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1709 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1710 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1711 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1712 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1713 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1714 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1715 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1716 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1717 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1718 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1719 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1720 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1721 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1722 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1723 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1724 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1725 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1726 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1727 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1728 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1729 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1730 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1731 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1732 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1733 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1734 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1736 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1737 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1740 "../../../src/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1741 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1742 "../../../src/util/parser/hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1743 "../../../src/util/parser/hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1744 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1745 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1746 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1747 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1748 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1749 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1750 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1751 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1752 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1753 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1754 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1755 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1756 "../../../src/util/parser/hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1757 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1758 "../../../src/util/parser/hphp.y"
    { Token t; _p->onFunctionStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1762 "../../../src/util/parser/hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]));
                                         _p->popLabelInfo();;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1765 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1766 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1770 "../../../src/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1775 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1777 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1781 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1782 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1783 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1787 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1788 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1792 "../../../src/util/parser/hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1793 "../../../src/util/parser/hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1794 "../../../src/util/parser/hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1795 "../../../src/util/parser/hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1799 "../../../src/util/parser/hphp.y"
    { no_gap(_p); xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1802 "../../../src/util/parser/hphp.y"
    { _p->scanner().xhpCloseTag();
                                         Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         (yyval).setText("");;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1808 "../../../src/util/parser/hphp.y"
    { _p->scanner().xhpChild();;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1809 "../../../src/util/parser/hphp.y"
    { _p->scanner().xhpCloseTag();;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1810 "../../../src/util/parser/hphp.y"
    { _p->onArray((yyvsp[(5) - (8)]),(yyvsp[(1) - (8)]));
                                         _p->onArray((yyvsp[(6) - (8)]),(yyvsp[(4) - (8)]));
                                         _p->onCallParam((yyvsp[(2) - (8)]),NULL,(yyvsp[(5) - (8)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (8)]),(yyvsp[(6) - (8)]),0);
                                         (yyval).setText((yyvsp[(8) - (8)]).text());;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1817 "../../../src/util/parser/hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1818 "../../../src/util/parser/hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1822 "../../../src/util/parser/hphp.y"
    { _p->scanner().xhpAttribute();;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1823 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1824 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1827 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1828 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1831 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1835 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1838 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1841 "../../../src/util/parser/hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1848 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); _p->scanner().xhpChild();;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1849 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); _p->scanner().xhpChild();;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1852 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); no_gap(_p);;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1853 "../../../src/util/parser/hphp.y"
    { no_gap(_p);;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1854 "../../../src/util/parser/hphp.y"
    { no_gap(_p); (yyval) = (yyvsp[(1) - (4)]) + ":" + (yyvsp[(4) - (4)]);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1855 "../../../src/util/parser/hphp.y"
    { no_gap(_p);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1856 "../../../src/util/parser/hphp.y"
    { no_gap(_p); (yyval) = (yyvsp[(1) - (4)]) + "-" + (yyvsp[(4) - (4)]);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1859 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1860 "../../../src/util/parser/hphp.y"
    { no_gap(_p);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1861 "../../../src/util/parser/hphp.y"
    { no_gap(_p); (yyval) = (yyvsp[(1) - (4)]) + ":" + (yyvsp[(4) - (4)]);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1862 "../../../src/util/parser/hphp.y"
    { no_gap(_p);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1863 "../../../src/util/parser/hphp.y"
    { no_gap(_p); (yyval) = (yyvsp[(1) - (4)]) + "-" + (yyvsp[(4) - (4)]);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1866 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1867 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1868 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1869 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1870 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1871 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1872 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1873 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1874 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1875 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1876 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1877 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1878 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1879 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1880 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1881 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1882 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1883 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1884 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1885 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1886 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1887 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1888 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1889 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1890 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1891 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1892 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1893 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1894 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1895 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1896 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1897 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1898 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1899 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1900 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1901 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1902 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1903 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1904 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1905 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1906 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1907 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1908 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1909 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1910 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1911 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1912 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1913 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1914 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1915 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1916 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1917 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1918 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1919 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1920 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1921 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1922 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1923 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1924 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1925 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1926 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1927 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1928 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1929 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1930 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1931 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1932 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1933 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1934 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1935 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1936 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1937 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1938 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1943 "../../../src/util/parser/hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1948 "../../../src/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1949 "../../../src/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1950 "../../../src/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1954 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1955 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1958 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1959 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1962 "../../../src/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1963 "../../../src/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1964 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1966 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),1));
                                         _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1968 "../../../src/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(2) - (2)]),Parser::StringName);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1970 "../../../src/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onName((yyval),(yyvsp[(3) - (3)]),Parser::StringName);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1975 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1976 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1977 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1981 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1982 "../../../src/util/parser/hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1983 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1987 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1988 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1992 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1993 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1994 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1996 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1997 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1998 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1999 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2000 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2001 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2002 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2003 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2006 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2008 "../../../src/util/parser/hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2012 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2013 "../../../src/util/parser/hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2014 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2015 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2017 "../../../src/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2018 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2023 "../../../src/util/parser/hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2025 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2029 "../../../src/util/parser/hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2030 "../../../src/util/parser/hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2031 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2032 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2033 "../../../src/util/parser/hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2034 "../../../src/util/parser/hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2036 "../../../src/util/parser/hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2041 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2042 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2045 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2046 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2051 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2053 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2055 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2056 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2060 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2061 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2062 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2066 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2068 "../../../src/util/parser/hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2071 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2072 "../../../src/util/parser/hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2073 "../../../src/util/parser/hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2076 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2077 "../../../src/util/parser/hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2078 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2079 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2081 "../../../src/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2082 "../../../src/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2086 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2087 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2092 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2094 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2096 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2097 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2101 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2102 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2106 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2107 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2110 "../../../src/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2111 "../../../src/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2117 "../../../src/util/parser/hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2119 "../../../src/util/parser/hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2122 "../../../src/util/parser/hphp.y"
    { user_attribute_check(_p);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2124 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2127 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2130 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2131 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2134 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2135 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2136 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2137 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2139 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2140 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2142 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2144 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2146 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2149 "../../../src/util/parser/hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2151 "../../../src/util/parser/hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2152 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2156 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2157 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2158 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2160 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2161 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2163 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2165 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]));;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2167 "../../../src/util/parser/hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2168 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2172 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2174 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2175 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2176 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2182 "../../../src/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2185 "../../../src/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2188 "../../../src/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2195 "../../../src/util/parser/hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2199 "../../../src/util/parser/hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2203 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2205 "../../../src/util/parser/hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2210 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2211 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2212 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2215 "../../../src/util/parser/hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2216 "../../../src/util/parser/hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2219 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2220 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2224 "../../../src/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2225 "../../../src/util/parser/hphp.y"
    { (yyval)++;;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2229 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2231 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2233 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2235 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2237 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2239 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]));;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2242 "../../../src/util/parser/hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2243 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2248 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2250 "../../../src/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2252 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2254 "../../../src/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]));;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2255 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2259 "../../../src/util/parser/hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2260 "../../../src/util/parser/hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2262 "../../../src/util/parser/hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2263 "../../../src/util/parser/hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2264 "../../../src/util/parser/hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2265 "../../../src/util/parser/hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2270 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2271 "../../../src/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2275 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2276 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2277 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2278 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2281 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2283 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2284 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2285 "../../../src/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2289 "../../../src/util/parser/hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2291 "../../../src/util/parser/hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2292 "../../../src/util/parser/hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2294 "../../../src/util/parser/hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2299 "../../../src/util/parser/hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2301 "../../../src/util/parser/hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2303 "../../../src/util/parser/hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2305 "../../../src/util/parser/hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2307 "../../../src/util/parser/hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2308 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2311 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2312 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2313 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2317 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2318 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2319 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2320 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2321 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2322 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2323 "../../../src/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2327 "../../../src/util/parser/hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2328 "../../../src/util/parser/hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2333 "../../../src/util/parser/hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2341 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2342 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2348 "../../../src/util/parser/hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2349 "../../../src/util/parser/hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]);
                                         only_in_strict_mode(_p); ;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2354 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2355 "../../../src/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2371 "../../../src/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2372 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2376 "../../../src/util/parser/hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2377 "../../../src/util/parser/hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2379 "../../../src/util/parser/hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (5)]).text()); ;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2380 "../../../src/util/parser/hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2388 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2389 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2390 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);
                                         /* if the type annotation is a bound
                                            typevar we have to strip it */
                                         if (_p->scanner().isStrictMode() &&
                                             (_p->isTypeVar((yyval).text()) ||
                                              !(yyval).text().compare("mixed") ||
                                              !(yyval).text().compare("this")
                                             )) {
                                           (yyval).reset();
                                         }
                                       ;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2401 "../../../src/util/parser/hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2402 "../../../src/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2404 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2406 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2407 "../../../src/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).setText("array"); ;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2412 "../../../src/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2413 "../../../src/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2414 "../../../src/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2415 "../../../src/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2416 "../../../src/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2419 "../../../src/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2420 "../../../src/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10010 "hphp.tab.cpp"
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
#line 2423 "../../../src/util/parser/hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

