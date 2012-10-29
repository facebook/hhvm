%{
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
    _p->onExprListElem(init, NULL, assign);
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
%}

%expect 2
%define api.pure
%parse-param {HPHP::HPHP_PARSER_NS::Parser *_p}

%left T_INCLUDE T_INCLUDE_ONCE T_EVAL T_REQUIRE T_REQUIRE_ONCE
%left ','
%left T_LOGICAL_OR
%left T_LOGICAL_XOR
%left T_LOGICAL_AND
%right T_PRINT
%left '=' T_PLUS_EQUAL T_MINUS_EQUAL T_MUL_EQUAL T_DIV_EQUAL T_CONCAT_EQUAL T_MOD_EQUAL T_AND_EQUAL T_OR_EQUAL T_XOR_EQUAL T_SL_EQUAL T_SR_EQUAL
%left '?' ':'
%left T_BOOLEAN_OR
%left T_BOOLEAN_AND
%left '|'
%left '^'
%left '&'
%nonassoc T_IS_EQUAL T_IS_NOT_EQUAL T_IS_IDENTICAL T_IS_NOT_IDENTICAL
%nonassoc '<' T_IS_SMALLER_OR_EQUAL '>' T_IS_GREATER_OR_EQUAL
%left T_SL T_SR
%left '+' '-' '.'
%left '*' '/' '%'
%right '!'
%nonassoc T_INSTANCEOF
%right '~' T_INC T_DEC T_INT_CAST T_DOUBLE_CAST T_STRING_CAST T_ARRAY_CAST T_OBJECT_CAST T_BOOL_CAST T_UNSET_CAST '@'
%right '['

%nonassoc T_NEW T_CLONE
%token T_EXIT
%token T_IF
%left T_ELSEIF
%left T_ELSE
%left T_ENDIF
%token T_LNUMBER
%token T_DNUMBER
%token T_STRING
%token T_STRING_VARNAME
%token T_VARIABLE
%token T_NUM_STRING
%token T_INLINE_HTML
%token T_CHARACTER
%token T_BAD_CHARACTER
%token T_ENCAPSED_AND_WHITESPACE
%token T_CONSTANT_ENCAPSED_STRING
%token T_ECHO
%token T_DO
%token T_WHILE
%token T_ENDWHILE
%token T_FOR
%token T_ENDFOR
%token T_FOREACH
%token T_ENDFOREACH
%token T_DECLARE
%token T_ENDDECLARE
%token T_AS
%token T_SWITCH
%token T_ENDSWITCH
%token T_CASE
%token T_DEFAULT
%token T_BREAK
%token T_GOTO
%token T_CONTINUE
%token T_FUNCTION
%token T_CONST
%token T_RETURN
%token T_TRY
%token T_CATCH
%token T_THROW
%token T_USE
%token T_GLOBAL
%right T_STATIC T_ABSTRACT T_FINAL T_PRIVATE T_PROTECTED T_PUBLIC
%token T_VAR
%token T_UNSET
%token T_ISSET
%token T_EMPTY
%token T_HALT_COMPILER
%token T_CLASS
%token T_INTERFACE
%token T_EXTENDS
%token T_IMPLEMENTS
%token T_OBJECT_OPERATOR
%token T_DOUBLE_ARROW
%token T_LIST
%token T_ARRAY
%token T_CLASS_C
%token T_METHOD_C
%token T_FUNC_C
%token T_LINE
%token T_FILE
%token T_COMMENT
%token T_DOC_COMMENT
%token T_OPEN_TAG
%token T_OPEN_TAG_WITH_ECHO
%token T_CLOSE_TAG
%token T_WHITESPACE
%token T_START_HEREDOC
%token T_END_HEREDOC
%token T_DOLLAR_OPEN_CURLY_BRACES
%token T_CURLY_OPEN
%token T_PAAMAYIM_NEKUDOTAYIM
%token T_NAMESPACE
%token T_NS_C
%token T_DIR
%token T_NS_SEPARATOR

%token T_YIELD

%token T_XHP_LABEL
%token T_XHP_TEXT
%token T_XHP_ATTRIBUTE
%token T_XHP_CATEGORY
%token T_XHP_CATEGORY_LABEL
%token T_XHP_CHILDREN
%token T_XHP_ENUM
%token T_XHP_REQUIRED

%token T_TRAIT
%token T_INSTEADOF
%token T_TRAIT_C

%token T_VARARG
%token T_STRICT_INT_MAP
%token T_STRICT_STR_MAP
%token T_STRICT_ERROR
%token T_FINALLY

%%

start:
    top_statement_list                 { _p->popLabelInfo();
                                         _p->saveParseTree($$);}
;

top_statement_list:
    top_statement_list
    top_statement                      { _p->addStatement($$,$1,$2);}
  |                                    { _p->onStatementListStart($$);}
;
top_statement:
    statement                          { _p->nns($1.num() == T_DECLARE);
                                         $$ = $1;}
  | function_declaration_statement     { _p->nns(); $$ = $1;}
  | class_declaration_statement        { _p->nns(); $$ = $1;}
  | trait_declaration_statement        { _p->nns(); $$ = $1;}
  | T_HALT_COMPILER '(' ')' ';'        { $$.reset();}
  | T_NAMESPACE namespace_name ';'     { _p->onNamespaceStart($2.text());
                                         $$.reset();}
  | T_NAMESPACE namespace_name '{'     { _p->onNamespaceStart($2.text());}
    top_statement_list '}'             { _p->onNamespaceEnd(); $$ = $5;}
  | T_NAMESPACE '{'                    { _p->onNamespaceStart("");}
    top_statement_list '}'             { _p->onNamespaceEnd(); $$ = $4;}
  | T_USE use_declarations ';'         { _p->nns(); $$.reset();}
  | constant_declaration ';'           { _p->nns();
                                         _p->finishStatement($$, $1); $$ = 1;}
;

use_declarations:
    use_declarations ','
    use_declaration                    { }
  | use_declaration                    { }
;
use_declaration:
    namespace_name                     { _p->onUse($1.text(),"");}
  | T_NS_SEPARATOR namespace_name      { _p->onUse($2.text(),"");}
  | namespace_name T_AS T_STRING       { _p->onUse($1.text(),$3.text());}
  | T_NS_SEPARATOR namespace_name
    T_AS T_STRING                      { _p->onUse($2.text(),$4.text());}
;
namespace_name:
    T_STRING                           { $$ = $1;}
  | namespace_name T_NS_SEPARATOR
    T_STRING                           { $$ = $1 + $2 + $3;}
;
namespace_string:
    namespace_name                     { $$ = $1;
                                         $$.setText(_p->resolve($$.text(),0));}
  | T_NS_SEPARATOR namespace_name      { $$ = $2;}
  | T_NAMESPACE T_NS_SEPARATOR
    namespace_name                     { $$.setText(_p->nsDecl($3.text()));}
;
class_namespace_string:
    namespace_name                     { $$ = $1;
                                         $$.setText(_p->resolve($$.text(),1));}
  | T_NS_SEPARATOR namespace_name      { $$ = $2;}
  | T_NAMESPACE T_NS_SEPARATOR
    namespace_name                     { $$.setText(_p->nsDecl($3.text()));}
;
constant_declaration:
    constant_declaration ','
    sm_name_with_type '=' static_scalar { $3.setText(_p->nsDecl($3.text()));
                                         on_constant(_p,$$,&$1,$3,$5);}
  | T_CONST sm_name_with_type '=' static_scalar { $2.setText(_p->nsDecl($2.text()));
                                         on_constant(_p,$$,  0,$2,$4);}
;

inner_statement_list:
    inner_statement_list
    inner_statement                    { _p->addStatement($$,$1,$2);}
  |                                    { _p->onStatementListStart($$);}
;
inner_statement:
    statement                          { $$ = $1;}
  | function_declaration_statement     { $$ = $1;}
  | class_declaration_statement        { $$ = $1;}
  | trait_declaration_statement        { $$ = $1;}
;
statement:
    '{' inner_statement_list '}'       { _p->onBlock($$, $2);}
  | T_IF '(' expr ')'
    statement
    elseif_list
    else_single                        { _p->onIf($$,$3,$5,$6,$7);}
  | T_IF '(' expr ')' ':'
    inner_statement_list
    new_elseif_list
    new_else_single
    T_ENDIF ';'                        { _p->onIf($$,$3,$6,$7,$8);}
  | T_WHILE '(' expr ')'               { _p->pushLabelScope();}
    while_statement                    { _p->popLabelScope();
                                         _p->onWhile($$,$3,$6);}

  | T_DO                               { _p->pushLabelScope();}
    statement T_WHILE '(' expr ')' ';' { _p->popLabelScope();
                                         _p->onDo($$,$3,$6);}
  | T_FOR '(' for_expr ';'
    for_expr ';' for_expr ')'          { _p->pushLabelScope();}
    for_statement                      { _p->popLabelScope();
                                         _p->onFor($$,$3,$5,$7,$10);}
  | T_SWITCH '(' expr ')'              { _p->pushLabelScope();}
    switch_case_list                   { _p->popLabelScope();
                                         _p->onSwitch($$,$3,$6);}
  | T_BREAK ';'                        { _p->onBreak($$, NULL);}
  | T_BREAK expr ';'                   { _p->onBreak($$, &$2);}
  | T_CONTINUE ';'                     { _p->onContinue($$, NULL);}
  | T_CONTINUE expr ';'                { _p->onContinue($$, &$2);}
  | T_RETURN ';'                       { _p->onReturn($$, NULL);}
  | T_RETURN expr ';'                  { _p->onReturn($$, &$2);}
  | T_YIELD T_BREAK ';'                { _p->onYield($$, NULL, false);}
  | T_YIELD expr ';'                   { _p->onYield($$, &$2, false);}
  | variable '=' T_YIELD expr ';'      { on_yield_assign(_p, $$, $1, &$4);}
  | T_LIST '(' assignment_list ')'
    '=' T_YIELD expr ';'               { on_yield_list_assign(_p, $$, $3, &$7);}
  | T_GLOBAL global_var_list ';'       { _p->onGlobal($$, $2);}
  | T_STATIC static_var_list ';'       { _p->onStatic($$, $2);}
  | T_ECHO expr_list ';'               { _p->onEcho($$, $2, 0);}
  | T_UNSET '(' variable_list ')' ';'  { _p->onUnset($$, $3);}
  | ';'                                { $$.reset();}
  | T_INLINE_HTML                      { _p->onEcho($$, $1, 1);}
  | T_FOREACH '(' expr
    T_AS foreach_variable
    foreach_optional_arg ')'           { _p->pushLabelScope();}
    foreach_statement                  { _p->popLabelScope();
                                         _p->onForEach($$,$3,$5,$6,$9);}

  | T_DECLARE '(' declare_list ')'
    declare_statement                  { _p->onBlock($$, $5); $$ = T_DECLARE;}
  | T_TRY '{'
    inner_statement_list '}'
    T_CATCH '('
    fully_qualified_class_name
    T_VARIABLE ')' '{'
    inner_statement_list '}'
    additional_catches
    optional_finally                   { _p->onTry($$,$3,$7,$8,$11,$13,$14);}
  | T_TRY '{'
    inner_statement_list '}'
    finally                            { _p->onTry($$, $3, $5);}
  | T_THROW expr ';'                   { _p->onThrow($$, $2);}
  | T_GOTO T_STRING ';'                { _p->onGoto($$, $2, true);
                                         _p->addGoto($2.text(),
                                                     _p->getLocation(),
                                                     &$$); }
  | expr ';'                           { _p->onExpStatement($$, $1);}
  | T_STRING ':'                       { _p->onLabel($$, $1);
                                         _p->addLabel($1.text(),
                                                      _p->getLocation(),
                                                      &$$); }
;

additional_catches:
    additional_catches
    T_CATCH '('
    fully_qualified_class_name
    T_VARIABLE ')'
    '{'
    inner_statement_list '}'           { _p->onCatch($$, $1, $4, $5, $8);}
  |                                    { $$.reset();}
;

finally:
                                       { finally_statement(_p);}
    T_FINALLY '{'
    inner_statement_list '}'           { _p->onFinally($$, $4);}
;

optional_finally:
    finally
  |                                    { $$.reset();}
;

is_reference:
    '&'                                { $$ = 1;}
  |                                    { $$.reset();}
;

function_loc:
    T_FUNCTION                         { _p->pushFuncLocation();}
;

function_declaration_statement:
    function_loc
    is_reference sm_name_with_typevar  { $3.setText(_p->nsDecl($3.text()));
                                         _p->onFunctionStart($3);
                                         _p->pushLabelInfo();}
    '(' parameter_list ')'
    sm_opt_return_type
    '{' inner_statement_list '}'       { Token t; t.reset();
                                         _p->onFunction($$,t,$2,$3,$6,$10,0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();}
  | non_empty_user_attributes function_loc
    is_reference sm_name_with_typevar  { $4.setText(_p->nsDecl($4.text()));
                                         _p->onFunctionStart($4);
                                         _p->pushLabelInfo();}
    '(' parameter_list ')'
    sm_opt_return_type
    '{' inner_statement_list '}'       { Token t; t.reset();
                                         _p->onFunction($$,t,$3,$4,$7,$11,&$1);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();}
;

class_declaration_statement:
    class_entry_type
    class_decl_name                    { $2.setText(_p->nsDecl($2.text()));
                                         _p->onClassStart($1.num(),$2);}
    extends_from implements_list '{'   { if (_p->peekClass())
                                           _p->scanner().xhpStatement();}
    class_statement_list '}'           { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,$8);
                                         } else {
                                           stmts = $8;
                                         }
                                         _p->onClass($$,$1.num(),$2,$4,$5,
                                                     stmts,0);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                           _p->scanner().xhpReset();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();}
  | non_empty_user_attributes
    class_entry_type
    class_decl_name                    { $3.setText(_p->nsDecl($3.text()));
                                         _p->onClassStart($2.num(),$3);}
    extends_from implements_list '{'   { if (_p->peekClass())
                                           _p->scanner().xhpStatement();}
    class_statement_list '}'           { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,$9);
                                         } else {
                                           stmts = $9;
                                         }
                                         _p->onClass($$,$2.num(),$3,$5,$6,
                                                     stmts,&$1);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                           _p->scanner().xhpReset();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();}
  | T_INTERFACE
    interface_decl_name                { $2.setText(_p->nsDecl($2.text()));
                                         _p->onClassStart(T_INTERFACE,$2);}
    interface_extends_list '{'
    class_statement_list '}'           { _p->onInterface($$,$2,$4,$6,0);
                                         _p->popClass();
                                         _p->popTypeScope();}
  | non_empty_user_attributes
    T_INTERFACE
    interface_decl_name                { $3.setText(_p->nsDecl($3.text()));
                                         _p->onClassStart(T_INTERFACE,$3);}
    interface_extends_list '{'
    class_statement_list '}'           { _p->onInterface($$,$3,$5,$7,&$1);
                                         _p->popClass();
                                         _p->popTypeScope();}
;

trait_declaration_statement:
    T_TRAIT
    trait_decl_name                    { $2.setText(_p->nsDecl($2.text()));
                                         _p->onClassStart(T_TRAIT, $2);}
    '{' class_statement_list '}'       { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass($$,T_TRAIT,$2,t_ext,t_imp,
                                                     $5, 0);
                                         _p->popClass();
                                         _p->popTypeScope();}
  | non_empty_user_attributes
    T_TRAIT
    trait_decl_name                    { $3.setText(_p->nsDecl($3.text()));
                                         _p->onClassStart(T_TRAIT, $3);}
    '{' class_statement_list '}'       { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass($$,T_TRAIT,$3,t_ext,t_imp,
                                                     $6, &$1);
                                         _p->popClass();
                                         _p->popTypeScope();}
;
class_decl_name:
    sm_name_with_typevar               { _p->pushClass(false); $$ = $1;}
  | T_XHP_LABEL                        { $1.xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); $$ = $1;}
;
interface_decl_name:
    sm_name_with_typevar               { _p->pushClass(false); $$ = $1;}
;
trait_decl_name:
    sm_name_with_typevar               { _p->pushClass(false); $$ = $1;}
;
class_entry_type:
    T_CLASS                            { $$ = T_CLASS;}
  | T_ABSTRACT T_CLASS                 { $$ = T_ABSTRACT;}
  | T_FINAL T_CLASS                    { $$ = T_FINAL;}
;
extends_from:
    T_EXTENDS
    fully_qualified_class_name         { $$ = $2;}
  |                                    { $$.reset();}
;
implements_list:
    T_IMPLEMENTS interface_list        { $$ = $2;}
  |                                    { $$.reset();}
;
interface_extends_list:
    T_EXTENDS interface_list           { $$ = $2;}
  |                                    { $$.reset();}
;
interface_list:
    fully_qualified_class_name         { _p->onInterfaceName($$, NULL, $1);}
  | interface_list ','
    fully_qualified_class_name         { _p->onInterfaceName($$, &$1, $3);}
;
trait_list:
    fully_qualified_class_name         { _p->onTraitName($$, NULL, $1);}
  | trait_list ','
    fully_qualified_class_name         { _p->onTraitName($$, &$1, $3);}
;

foreach_optional_arg:
    T_DOUBLE_ARROW foreach_variable    { $$ = $2;}
  |                                    { $$.reset();}
;
foreach_variable:
    variable                           { $$ = $1;}
  | '&' variable                       { $$ = $2; $$ = 1;}
;

for_statement:
    statement                          { $$ = $1;}
  | ':' inner_statement_list
    T_ENDFOR ';'                       { $$ = $2;}
;
foreach_statement:
    statement                          { $$ = $1;}
  | ':' inner_statement_list
    T_ENDFOREACH ';'                   { $$ = $2;}
;
while_statement:
    statement                          { $$ = $1;}
  | ':' inner_statement_list
    T_ENDWHILE ';'                     { $$ = $2;}
;
declare_statement:
    statement                          { $$ = $1;}
  | ':' inner_statement_list
    T_ENDDECLARE ';'                   { $$ = $2;}
;

declare_list:
    T_STRING '=' static_scalar
  | declare_list ','
    T_STRING '=' static_scalar
;

switch_case_list:
    '{' case_list '}'                  { $$ = $2;}
  | '{' ';' case_list '}'              { $$ = $3;}
  | ':' case_list T_ENDSWITCH ';'      { $$ = $2;}
  | ':' ';' case_list T_ENDSWITCH ';'  { $$ = $3;}
;
case_list:
    case_list T_CASE expr
    case_separator
    inner_statement_list               { _p->onCase($$,$1,&$3,$5);}
  | case_list T_DEFAULT case_separator
    inner_statement_list               { _p->onCase($$,$1,NULL,$4);}
  |                                    { $$.reset();}
;
case_separator:
    ':'                                { $$.reset();}
  | ';'                                { $$.reset();}
;

elseif_list:
    elseif_list T_ELSEIF '(' expr ')'
    statement                          { _p->onElseIf($$,$1,$4,$6);}
  |                                    { $$.reset();}
;
new_elseif_list:
    new_elseif_list T_ELSEIF
    '(' expr ')' ':'
    inner_statement_list               { _p->onElseIf($$,$1,$4,$7);}
  |                                    { $$.reset();}
;
else_single:
    T_ELSE statement                   { $$ = $2;}
  |                                    { $$.reset();}
;
new_else_single:
    T_ELSE ':' inner_statement_list    { $$ = $3;}
  |                                    { $$.reset();}
;

parameter_list:
    non_empty_parameter_list ',' T_VARARG
                                       { only_in_strict_mode(_p); $$ = $1; }
  | non_empty_parameter_list           { $$ = $1;}
  | T_VARARG                           { only_in_strict_mode(_p); $$.reset(); }
  |                                    { $$.reset();}
;

non_empty_parameter_list:
    optional_user_attributes
    sm_type_opt T_VARIABLE             { _p->onParam($$,NULL,$2,$3,0,NULL,&$1);}
  | optional_user_attributes
    sm_type_opt '&' T_VARIABLE         { _p->onParam($$,NULL,$2,$4,1,NULL,&$1);}
  | optional_user_attributes
    sm_type_opt '&' T_VARIABLE
    '=' static_scalar                  { _p->onParam($$,NULL,$2,$4,1,&$6,&$1);}
  | optional_user_attributes
    sm_type_opt T_VARIABLE
    '=' static_scalar                  { _p->onParam($$,NULL,$2,$3,0,&$5,&$1);}
  | non_empty_parameter_list ','
    optional_user_attributes
    sm_type_opt T_VARIABLE             { _p->onParam($$,&$1,$4,$5,0,NULL,&$3);}
  | non_empty_parameter_list ','
    optional_user_attributes
    sm_type_opt '&' T_VARIABLE         { _p->onParam($$,&$1,$4,$6,1,NULL,&$3);}
  | non_empty_parameter_list ','
    optional_user_attributes
    sm_type_opt '&' T_VARIABLE
    '=' static_scalar                  { _p->onParam($$,&$1,$4,$6,1,&$8,&$3);}
  | non_empty_parameter_list ','
    optional_user_attributes
    sm_type_opt T_VARIABLE
    '=' static_scalar                  { _p->onParam($$,&$1,$4,$5,0,&$7,&$3);}
;

function_call_parameter_list:
    non_empty_fcall_parameter_list     { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_fcall_parameter_list:
    expr                               { _p->onCallParam($$,NULL,$1,0);}
  | '&' variable                       { _p->onCallParam($$,NULL,$2,1);}
  | non_empty_fcall_parameter_list ','
    expr                               { _p->onCallParam($$,&$1,$3,0);}
  | non_empty_fcall_parameter_list ','
    '&' variable                       { _p->onCallParam($$,&$1,$4,1);}
;

global_var_list:
    global_var_list ',' global_var     { _p->onGlobalVar($$, &$1, $3);}
  | global_var                         { _p->onGlobalVar($$, NULL, $1);}
;
global_var:
    T_VARIABLE                         { $$ = $1;}
  | '$' variable                       { $$ = $2; $$ = 1;}
  | '$' '{' expr '}'                   { $$ = $3; $$ = 1;}
;

static_var_list:
    static_var_list ',' T_VARIABLE     { _p->onStaticVariable($$,&$1,$3,0);}
  | static_var_list ',' T_VARIABLE
    '=' static_scalar                  { _p->onStaticVariable($$,&$1,$3,&$5);}
  | T_VARIABLE                         { _p->onStaticVariable($$,0,$1,0);}
  | T_VARIABLE '=' static_scalar       { _p->onStaticVariable($$,0,$1,&$3);}
;

class_statement_list:
    class_statement_list
    class_statement                    { _p->onClassStatement($$, $1, $2);
                                         if (_p->peekClass())
                                           _p->scanner().xhpStatement();}
  |                                    { $$.reset();}
;
class_statement:
    variable_modifiers                 { _p->onClassVariableModifer($1);}
    class_variable_declaration ';'     { _p->onClassVariableStart
                                         ($$,&$1,$3,NULL);}
  | non_empty_member_modifiers
    sm_type                            { _p->onClassVariableModifer($1);}
    class_variable_declaration ';'     { _p->onClassVariableStart
                                         ($$,&$1,$4,&$2);}
  | class_constant_declaration ';'     { _p->onClassVariableStart
                                         ($$,NULL,$1,NULL);}
  | method_modifiers function_loc
    is_reference sm_name_with_typevar '('
                                       { _p->onMethodStart($4, $1);
                                         _p->pushLabelInfo();}
    parameter_list ')'
    sm_opt_return_type
    method_body
                                       { Token t; t.reset();
                                         _p->onMethod($$,$1,t,$3,$4,$7,$10,0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();}
  | non_empty_user_attributes
    method_modifiers function_loc
    is_reference sm_name_with_typevar '('
                                       { _p->onMethodStart($5, $2);
                                         _p->pushLabelInfo();}
    parameter_list ')'
    sm_opt_return_type
    method_body
                                       { Token t; t.reset();
                                         _p->onMethod($$,$2,t,$4,$5,$8,$11,&$1);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();}
  | T_XHP_ATTRIBUTE                    { _p->scanner().xhpAttributeDecl();}
    xhp_attribute_stmt ';'             { _p->xhpSetAttributes($3);}
  | T_XHP_CATEGORY
    xhp_category_stmt ';'              { xhp_category_stmt(_p,$$,$2);}
  | T_XHP_CHILDREN
    xhp_children_stmt ';'              { xhp_children_stmt(_p,$$,$2);}
  | T_USE trait_list ';'               { Token t; t.reset();
                                         _p->onTraitUse($$,$2,t); }
  | T_USE trait_list '{'
    trait_rules  '}'                   { _p->onTraitUse($$,$2,$4); }
;
trait_rules:
    trait_rules trait_precedence_rule  { _p->onTraitRule($$,$1,$2); }
  | trait_rules trait_alias_rule       { _p->onTraitRule($$,$1,$2); }
  | /* empty */                        { $$.reset(); }
;
trait_precedence_rule:
    class_namespace_string
    T_PAAMAYIM_NEKUDOTAYIM
    T_STRING
    T_INSTEADOF trait_list ';'         { _p->onTraitPrecRule($$,$1,$3,$5); }
;
trait_alias_rule:
    trait_alias_rule_method T_AS
    method_modifiers T_STRING ';'      { _p->onTraitAliasRuleModify($$,$1,$3,
                                                                    $4);}
  | trait_alias_rule_method T_AS
    non_empty_member_modifiers ';'     { Token t; t.reset();
                                         _p->onTraitAliasRuleModify($$,$1,$3,
                                                                    t);}
;
trait_alias_rule_method:
    class_namespace_string
    T_PAAMAYIM_NEKUDOTAYIM
    T_STRING                           { _p->onTraitAliasRuleStart($$,$1,$3);}
  | T_STRING                           { Token t; t.reset();
                                         _p->onTraitAliasRuleStart($$,t,$1);}
;
xhp_attribute_stmt:
    xhp_attribute_decl                 { xhp_attribute_list(_p,$$,
                                         _p->xhpGetAttributes(),$1);}
  | xhp_attribute_stmt ','             { _p->scanner().xhpAttributeDecl();}
    xhp_attribute_decl                 { xhp_attribute_list(_p,$$, &$1,$4);}
;

xhp_attribute_decl:
    xhp_attribute_decl_type
    xhp_label_ws
    xhp_attribute_default
    xhp_attribute_is_required          { xhp_attribute(_p,$$,$1,$2,$3,$4);
                                         $$ = 1;}
  | T_XHP_LABEL                        { $$ = $1; $$ = 0;}
;

xhp_attribute_decl_type:
    T_STRING_CAST                      { $$ = 1;}
  | T_BOOL_CAST                        { $$ = 2;}
  | T_INT_CAST                         { $$ = 3;}
  | T_ARRAY_CAST                       { $$ = 4;}
  | fully_qualified_class_name         { $$ = 5; $$.setText($1);}
  | T_VAR                              { $$ = 6;}
  | T_XHP_ENUM '{'
    xhp_attribute_enum '}'             { $$ = $3; $$ = 7;}
  | T_DOUBLE_CAST                      { $$ = 8;}
;

xhp_attribute_enum:
    common_scalar                      { _p->onArrayPair($$,  0,0,$1,0);}
  | xhp_attribute_enum ','
    common_scalar                      { _p->onArrayPair($$,&$1,0,$3,0);}
;

xhp_attribute_default:
    '=' static_scalar                  { $$ = $2;}
  |                                    { scalar_null(_p, $$);}
;

xhp_attribute_is_required:
    '@'                                { _p->scanner().xhpAttributeDecl();}
    T_XHP_REQUIRED                     { scalar_num(_p, $$, "1");}
  |                                    { scalar_num(_p, $$, "0");}
;

xhp_category_stmt:
    xhp_category_decl                  { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair($$,0,&$1,t,0);}
  | xhp_category_stmt ','
    xhp_category_decl                  { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair($$,&$1,&$3,t,0);}
;

xhp_category_decl:
    T_XHP_CATEGORY_LABEL               { _p->onScalar($$,
                                         T_CONSTANT_ENCAPSED_STRING, $1);}
;

xhp_children_stmt:
    xhp_children_paren_expr            { $$ = $1; $$ = 2;}
  | T_STRING                           { $$ = -1;
                                         if ($1.same("any")) $$ = 1;}
  | T_EMPTY                            { $$ = 0;}
;

xhp_children_paren_expr:
    '(' xhp_children_decl_expr ')'     { xhp_children_paren(_p, $$, $2, 0);}
  | '(' xhp_children_decl_expr ')' '*' { xhp_children_paren(_p, $$, $2, 1);}
  | '(' xhp_children_decl_expr ')' '?' { xhp_children_paren(_p, $$, $2, 2);}
  | '(' xhp_children_decl_expr ')' '+' { xhp_children_paren(_p, $$, $2, 3);}
;

xhp_children_decl_expr:
    xhp_children_paren_expr            { $$ = $1;}
  | xhp_children_decl_tag              { xhp_children_decl(_p,$$,$1,0,  0);}
  | xhp_children_decl_tag '*'          { xhp_children_decl(_p,$$,$1,1,  0);}
  | xhp_children_decl_tag '?'          { xhp_children_decl(_p,$$,$1,2,  0);}
  | xhp_children_decl_tag '+'          { xhp_children_decl(_p,$$,$1,3,  0);}
  | xhp_children_decl_expr ','
    xhp_children_decl_expr             { xhp_children_decl(_p,$$,$1,4,&$3);}
  | xhp_children_decl_expr '|'
    xhp_children_decl_expr             { xhp_children_decl(_p,$$,$1,5,&$3);}
;

xhp_children_decl_tag:
    T_STRING                           { $$ = -1;
                                         if ($1.same("any")) $$ = 1; else
                                         if ($1.same("pcdata")) $$ = 2;}
  | T_XHP_LABEL                        { $1.xhpLabel();  $$ = $1; $$ = 3;}
  | T_XHP_CATEGORY_LABEL               { $1.xhpLabel(0); $$ = $1; $$ = 4;}
;

method_body:
    ';'                                { $$.reset();}
  | '{' inner_statement_list '}'       { _p->finishStatement($$, $2); $$ = 1;}
;
variable_modifiers:
    non_empty_member_modifiers         { $$ = $1;}
  | T_VAR                              { $$.reset();}
;
method_modifiers:
    non_empty_member_modifiers         { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_member_modifiers:
    member_modifier                    { _p->onMemberModifier($$,NULL,$1);}
  | non_empty_member_modifiers
    member_modifier                    { _p->onMemberModifier($$,&$1,$2);}
;
member_modifier:
    T_PUBLIC                           { $$ = T_PUBLIC;}
  | T_PROTECTED                        { $$ = T_PROTECTED;}
  | T_PRIVATE                          { $$ = T_PRIVATE;}
  | T_STATIC                           { $$ = T_STATIC;}
  | T_ABSTRACT                         { $$ = T_ABSTRACT;}
  | T_FINAL                            { $$ = T_FINAL;}
;
class_variable_declaration:
    class_variable_declaration ','
    T_VARIABLE                         { _p->onClassVariable($$,&$1,$3,0);}
  | class_variable_declaration ','
    T_VARIABLE '=' static_scalar       { _p->onClassVariable($$,&$1,$3,&$5);}
  | T_VARIABLE                         { _p->onClassVariable($$,0,$1,0);}
  | T_VARIABLE '=' static_scalar       { _p->onClassVariable($$,0,$1,&$3);}
;
class_constant_declaration:
    class_constant_declaration ','
    sm_name_with_type '=' static_scalar { _p->onClassConstant($$,&$1,$3,$5);}
  | T_CONST sm_name_with_type '=' static_scalar { _p->onClassConstant($$,0,$2,$4);}
;

expr_list:
    expr_list ',' expr                 { _p->onExprListElem($$, &$1, $3);}
  | expr                               { _p->onExprListElem($$, NULL, $1);}
;

for_expr:
    expr_list                          { $$ = $1;}
  |                                    { $$.reset();}
;

expr:
    variable                           { $$ = $1;}
  | T_LIST '(' assignment_list ')'
    '=' expr                           { _p->onListAssignment($$, $3, &$6);}
  | variable '=' expr                  { _p->onAssign($$, $1, $3, 0);}
  | variable '=' '&' variable          { _p->onAssign($$, $1, $4, 1);}
  | variable '=' '&' T_NEW
    class_name_reference
    ctor_arguments                     { _p->onAssignNew($$,$1,$5,$6);}
  | T_NEW class_name_reference
    ctor_arguments                     { _p->onNewObject($$, $2, $3);}
  | T_CLONE expr                       { UEXP($$,$2,T_CLONE,1);}
  | variable T_PLUS_EQUAL expr         { BEXP($$,$1,$3,T_PLUS_EQUAL);}
  | variable T_MINUS_EQUAL expr        { BEXP($$,$1,$3,T_MINUS_EQUAL);}
  | variable T_MUL_EQUAL expr          { BEXP($$,$1,$3,T_MUL_EQUAL);}
  | variable T_DIV_EQUAL expr          { BEXP($$,$1,$3,T_DIV_EQUAL);}
  | variable T_CONCAT_EQUAL expr       { BEXP($$,$1,$3,T_CONCAT_EQUAL);}
  | variable T_MOD_EQUAL expr          { BEXP($$,$1,$3,T_MOD_EQUAL);}
  | variable T_AND_EQUAL expr          { BEXP($$,$1,$3,T_AND_EQUAL);}
  | variable T_OR_EQUAL expr           { BEXP($$,$1,$3,T_OR_EQUAL);}
  | variable T_XOR_EQUAL expr          { BEXP($$,$1,$3,T_XOR_EQUAL);}
  | variable T_SL_EQUAL expr           { BEXP($$,$1,$3,T_SL_EQUAL);}
  | variable T_SR_EQUAL expr           { BEXP($$,$1,$3,T_SR_EQUAL);}
  | variable T_INC                     { UEXP($$,$1,T_INC,0);}
  | T_INC variable                     { UEXP($$,$2,T_INC,1);}
  | variable T_DEC                     { UEXP($$,$1,T_DEC,0);}
  | T_DEC variable                     { UEXP($$,$2,T_DEC,1);}
  | expr T_BOOLEAN_OR expr             { BEXP($$,$1,$3,T_BOOLEAN_OR);}
  | expr T_BOOLEAN_AND expr            { BEXP($$,$1,$3,T_BOOLEAN_AND);}
  | expr T_LOGICAL_OR expr             { BEXP($$,$1,$3,T_LOGICAL_OR);}
  | expr T_LOGICAL_AND expr            { BEXP($$,$1,$3,T_LOGICAL_AND);}
  | expr T_LOGICAL_XOR expr            { BEXP($$,$1,$3,T_LOGICAL_XOR);}
  | expr '|' expr                      { BEXP($$,$1,$3,'|');}
  | expr '&' expr                      { BEXP($$,$1,$3,'&');}
  | expr '^' expr                      { BEXP($$,$1,$3,'^');}
  | expr '.' expr                      { BEXP($$,$1,$3,'.');}
  | expr '+' expr                      { BEXP($$,$1,$3,'+');}
  | expr '-' expr                      { BEXP($$,$1,$3,'-');}
  | expr '*' expr                      { BEXP($$,$1,$3,'*');}
  | expr '/' expr                      { BEXP($$,$1,$3,'/');}
  | expr '%' expr                      { BEXP($$,$1,$3,'%');}
  | expr T_SL expr                     { BEXP($$,$1,$3,T_SL);}
  | expr T_SR expr                     { BEXP($$,$1,$3,T_SR);}
  | '+' expr %prec T_INC               { UEXP($$,$2,'+',1);}
  | '-' expr %prec T_INC               { UEXP($$,$2,'-',1);}
  | '!' expr                           { UEXP($$,$2,'!',1);}
  | '~' expr                           { UEXP($$,$2,'~',1);}
  | expr T_IS_IDENTICAL expr           { BEXP($$,$1,$3,T_IS_IDENTICAL);}
  | expr T_IS_NOT_IDENTICAL expr       { BEXP($$,$1,$3,T_IS_NOT_IDENTICAL);}
  | expr T_IS_EQUAL expr               { BEXP($$,$1,$3,T_IS_EQUAL);}
  | expr T_IS_NOT_EQUAL expr           { BEXP($$,$1,$3,T_IS_NOT_EQUAL);}
  | expr '<' expr                      { BEXP($$,$1,$3,'<');}
  | expr T_IS_SMALLER_OR_EQUAL expr    { BEXP($$,$1,$3,
                                              T_IS_SMALLER_OR_EQUAL);}
  | expr '>' expr                      { BEXP($$,$1,$3,'>');}
  | expr T_IS_GREATER_OR_EQUAL expr    { BEXP($$,$1,$3,
                                              T_IS_GREATER_OR_EQUAL);}
  | expr T_INSTANCEOF
    class_name_reference               { BEXP($$,$1,$3,T_INSTANCEOF);}
  | '(' expr ')'                       { $$ = $2;}
  | expr '?' expr ':' expr             { _p->onQOp($$, $1, &$3, $5);}
  | expr '?' ':' expr                  { _p->onQOp($$, $1,   0, $4);}
  | internal_functions                 { $$ = $1;}
  | T_INT_CAST expr                    { UEXP($$,$2,T_INT_CAST,1);}
  | T_DOUBLE_CAST expr                 { UEXP($$,$2,T_DOUBLE_CAST,1);}
  | T_STRING_CAST expr                 { UEXP($$,$2,T_STRING_CAST,1);}
  | T_ARRAY_CAST expr                  { UEXP($$,$2,T_ARRAY_CAST,1);}
  | T_OBJECT_CAST expr                 { UEXP($$,$2,T_OBJECT_CAST,1);}
  | T_BOOL_CAST expr                   { UEXP($$,$2,T_BOOL_CAST,1);}
  | T_UNSET_CAST expr                  { UEXP($$,$2,T_UNSET_CAST,1);}
  | T_EXIT exit_expr                   { UEXP($$,$2,T_EXIT,1);}
  | '@' expr                           { UEXP($$,$2,'@',1);}
  | scalar                             { $$ = $1;}
  | array_literal                      { $$ = $1;}
  | '`' backticks_expr '`'             { _p->onEncapsList($$,'`',$2);}
  | T_PRINT expr                       { UEXP($$,$2,T_PRINT,1);}
  | function_loc is_reference '('      { Token t; _p->onFunctionStart(t);
                                         _p->pushLabelInfo();}
    parameter_list ')'
    sm_opt_return_type lexical_vars
    '{' inner_statement_list '}'       { Token u; u.reset();
                                         _p->onClosure($$,u,$2,$5,$8,$10);
                                         _p->popLabelInfo();}
  | xhp_tag                            { $$ = $1;}
  | dim_expr                           { $$ = $1;}
;

array_literal:
    T_ARRAY '(' array_pair_list ')'    { _p->onArray($$,$3,T_ARRAY);}
;

dim_expr:
    dim_expr
    '[' dim_offset ']'                 { _p->onRefDim($$, $1, $3);}
  | dim_expr_base
    '[' dim_offset ']'                 { _p->onRefDim($$, $1, $3);}
;

dim_expr_base:
    array_literal                      { $$ = $1;}
  | class_constant                     { $$ = $1;}
  | '(' expr ')'                       { $$ = $2;}
;

lexical_vars:
    T_USE '(' lexical_var_list ')'     { $$ = $3;}
  |                                    { $$.reset();}
;

lexical_var_list:
    lexical_var_list ',' T_VARIABLE    { _p->onClosureParam($$,&$1,$3,0);}
  | lexical_var_list ',' '&'T_VARIABLE { _p->onClosureParam($$,&$1,$4,1);}
  | T_VARIABLE                         { _p->onClosureParam($$,  0,$1,0);}
  | '&' T_VARIABLE                     { _p->onClosureParam($$,  0,$2,1);}
;

xhp_tag:
    '<' xhp_label xhp_tag_body '>'     { no_gap(_p); xhp_tag(_p,$$,$2,$3);}
;
xhp_tag_body:
    xhp_attributes '/'                 { _p->scanner().xhpCloseTag();
                                         Token t1; _p->onArray(t1,$1);
                                         Token t2; _p->onArray(t2,$2);
                                         _p->onCallParam($1,NULL,t1,0);
                                         _p->onCallParam($$, &$1,t2,0);
                                         $$.setText("");}
  | xhp_attributes '>'                 { _p->scanner().xhpChild();}
    xhp_children '<' '/'               { _p->scanner().xhpCloseTag();}
    xhp_opt_end_label                  { _p->onArray($5,$1);
                                         _p->onArray($6,$4);
                                         _p->onCallParam($2,NULL,$5,0);
                                         _p->onCallParam($$, &$2,$6,0);
                                         $$.setText($8.text());}
;
xhp_opt_end_label:
                                       { $$.reset(); $$.setText("");}
  | T_XHP_LABEL                        { $$.reset(); $$.setText($1);}
;
xhp_attributes:
    xhp_attributes
    xhp_attribute_name '='             { _p->scanner().xhpAttribute();}
    xhp_attribute_value                { _p->onArrayPair($$,&$1,&$2,$5,0);}
  |                                    { $$.reset();}
;
xhp_children:
    xhp_children xhp_child             { _p->onArrayPair($$,&$1,0,$2,0);}
  |                                    { $$.reset();}
;
xhp_attribute_name:
    xhp_label_ws                       { _p->onScalar($$,
                                         T_CONSTANT_ENCAPSED_STRING, $1);}
;
xhp_attribute_value:
    T_XHP_TEXT                         { $1.xhpDecode();
                                         _p->onScalar($$,
                                         T_CONSTANT_ENCAPSED_STRING, $1);}
  | '{' expr '}'                       { $$ = $2;}
;
xhp_child:
    T_XHP_TEXT                         { $$.reset();
                                         if ($1.htmlTrim()) {
                                           $1.xhpDecode();
                                           _p->onScalar($$,
                                           T_CONSTANT_ENCAPSED_STRING, $1);
                                         }
                                       }
  | '{' expr '}'                       { $$ = $2; _p->scanner().xhpChild();}
  | xhp_tag                            { $$ = $1; _p->scanner().xhpChild();}
;
xhp_label:
    xhp_bareword                       { $$ = $1; no_gap(_p);}
  | xhp_label ':'                      { no_gap(_p);}
    xhp_bareword                       { no_gap(_p); $$ = $1 + ":" + $4;}
  | xhp_label '-'                      { no_gap(_p);}
    xhp_bareword                       { no_gap(_p); $$ = $1 + "-" + $4;}
;
xhp_label_ws:
    xhp_bareword                       { $$ = $1;}
  | xhp_label_ws ':'                   { no_gap(_p);}
    xhp_bareword                       { no_gap(_p); $$ = $1 + ":" + $4;}
  | xhp_label_ws '-'                   { no_gap(_p);}
    xhp_bareword                       { no_gap(_p); $$ = $1 + "-" + $4;}
;
xhp_bareword:
    T_STRING                           { $$ = $1;}
  | T_EXIT                             { $$ = $1;}
  | T_FUNCTION                         { $$ = $1;}
  | T_CONST                            { $$ = $1;}
  | T_RETURN                           { $$ = $1;}
  | T_YIELD                            { $$ = $1;}
  | T_TRY                              { $$ = $1;}
  | T_CATCH                            { $$ = $1;}
  | T_FINALLY                          { $$ = $1;}
  | T_THROW                            { $$ = $1;}
  | T_IF                               { $$ = $1;}
  | T_ELSEIF                           { $$ = $1;}
  | T_ENDIF                            { $$ = $1;}
  | T_ELSE                             { $$ = $1;}
  | T_WHILE                            { $$ = $1;}
  | T_ENDWHILE                         { $$ = $1;}
  | T_DO                               { $$ = $1;}
  | T_FOR                              { $$ = $1;}
  | T_ENDFOR                           { $$ = $1;}
  | T_FOREACH                          { $$ = $1;}
  | T_ENDFOREACH                       { $$ = $1;}
  | T_DECLARE                          { $$ = $1;}
  | T_ENDDECLARE                       { $$ = $1;}
  | T_INSTANCEOF                       { $$ = $1;}
  | T_AS                               { $$ = $1;}
  | T_SWITCH                           { $$ = $1;}
  | T_ENDSWITCH                        { $$ = $1;}
  | T_CASE                             { $$ = $1;}
  | T_DEFAULT                          { $$ = $1;}
  | T_BREAK                            { $$ = $1;}
  | T_CONTINUE                         { $$ = $1;}
  | T_GOTO                             { $$ = $1;}
  | T_ECHO                             { $$ = $1;}
  | T_PRINT                            { $$ = $1;}
  | T_CLASS                            { $$ = $1;}
  | T_INTERFACE                        { $$ = $1;}
  | T_EXTENDS                          { $$ = $1;}
  | T_IMPLEMENTS                       { $$ = $1;}
  | T_NEW                              { $$ = $1;}
  | T_CLONE                            { $$ = $1;}
  | T_VAR                              { $$ = $1;}
  | T_EVAL                             { $$ = $1;}
  | T_INCLUDE                          { $$ = $1;}
  | T_INCLUDE_ONCE                     { $$ = $1;}
  | T_REQUIRE                          { $$ = $1;}
  | T_REQUIRE_ONCE                     { $$ = $1;}
  | T_NAMESPACE                        { $$ = $1;}
  | T_USE                              { $$ = $1;}
  | T_GLOBAL                           { $$ = $1;}
  | T_ISSET                            { $$ = $1;}
  | T_EMPTY                            { $$ = $1;}
  | T_HALT_COMPILER                    { $$ = $1;}
  | T_STATIC                           { $$ = $1;}
  | T_ABSTRACT                         { $$ = $1;}
  | T_FINAL                            { $$ = $1;}
  | T_PRIVATE                          { $$ = $1;}
  | T_PROTECTED                        { $$ = $1;}
  | T_PUBLIC                           { $$ = $1;}
  | T_UNSET                            { $$ = $1;}
  | T_LIST                             { $$ = $1;}
  | T_ARRAY                            { $$ = $1;}
  | T_LOGICAL_OR                       { $$ = $1;}
  | T_LOGICAL_AND                      { $$ = $1;}
  | T_LOGICAL_XOR                      { $$ = $1;}
  | T_CLASS_C                          { $$ = $1;}
  | T_FUNC_C                           { $$ = $1;}
  | T_METHOD_C                         { $$ = $1;}
  | T_LINE                             { $$ = $1;}
  | T_FILE                             { $$ = $1;}
  | T_DIR                              { $$ = $1;}
  | T_NS_C                             { $$ = $1;}
  | T_TRAIT                            { $$ = $1;}
  | T_TRAIT_C                          { $$ = $1;}
;

simple_function_call:
    namespace_string '('
    function_call_parameter_list ')'   { _p->onCall($$,0,$1,$3,NULL);}
;

static_class_name:
    fully_qualified_class_name_no_typeargs
                                       { _p->onName($$,$1,Parser::StringName);}
  | T_STATIC                           { _p->onName($$,$1,Parser::StaticName);}
  | reference_variable                 { _p->onName($$,$1,
                                         Parser::StaticClassExprName);}
;
fully_qualified_class_name:
    class_namespace_string sm_typeargs_opt { $$ = $1;}
  | T_XHP_LABEL                        { $1.xhpLabel(); $$ = $1;}
;
fully_qualified_class_name_no_typeargs:
    class_namespace_string             { $$ = $1;}
  | T_XHP_LABEL                        { $1.xhpLabel(); $$ = $1;}
;
class_name_reference:
    variable_no_calls                  { _p->onName($$,$1,Parser::ExprName);}
  | T_STATIC                           { _p->onName($$,$1,Parser::StaticName);}
  | T_XHP_LABEL                        { $1.xhpLabel();
                                         _p->onName($$,$1,Parser::StringName);}
  | namespace_name                     { $1.setText(_p->resolve($1.text(),1));
                                         _p->onName($$,$1,Parser::StringName);}
  | T_NS_SEPARATOR namespace_name      { _p->onName($$,$2,Parser::StringName);}
  | T_NAMESPACE T_NS_SEPARATOR
    namespace_name                     { $3.setText(_p->nsDecl($3.text()));
                                         _p->onName($$,$3,Parser::StringName);}
;

exit_expr:
    '(' ')'                            { $$.reset();}
  | '(' expr ')'                       { $$ = $2;}
  |                                    { $$.reset();}
;

backticks_expr:
     /* empty */                       { $$.reset();}
  |  T_ENCAPSED_AND_WHITESPACE         { _p->addEncap($$, NULL, $1, 0);}
  |  encaps_list                       { $$ = $1;}

ctor_arguments:
    '('
    function_call_parameter_list ')'   { $$ = $2;}
  |                                    { $$.reset();}
;

common_scalar:
    T_LNUMBER                          { _p->onScalar($$, T_LNUMBER,  $1);}
  | T_DNUMBER                          { _p->onScalar($$, T_DNUMBER,  $1);}
  | T_CONSTANT_ENCAPSED_STRING         { _p->onScalar($$,
                                         T_CONSTANT_ENCAPSED_STRING,  $1);}
  | T_LINE                             { _p->onScalar($$, T_LINE,     $1);}
  | T_FILE                             { _p->onScalar($$, T_FILE,     $1);}
  | T_DIR                              { _p->onScalar($$, T_DIR,      $1);}
  | T_CLASS_C                          { _p->onScalar($$, T_CLASS_C,  $1);}
  | T_TRAIT_C                          { _p->onScalar($$, T_TRAIT_C,  $1);}
  | T_METHOD_C                         { _p->onScalar($$, T_METHOD_C, $1);}
  | T_FUNC_C                           { _p->onScalar($$, T_FUNC_C,   $1);}
  | T_NS_C                             { _p->onScalar($$, T_NS_C,  $1);}
  | T_START_HEREDOC
    T_ENCAPSED_AND_WHITESPACE
    T_END_HEREDOC                      { _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $2);}
  | T_START_HEREDOC
    T_END_HEREDOC                      { $$.setText(""); _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $$);}
;

static_scalar:
    common_scalar                      { $$ = $1;}
  | namespace_string                   { _p->onConstantValue($$, $1);}
  | '+' static_scalar                  { UEXP($$,$2,'+',1);}
  | '-' static_scalar                  { UEXP($$,$2,'-',1);}
  | T_ARRAY '('
    static_array_pair_list ')'         { _p->onArray($$,$3,T_ARRAY);}
  | static_class_constant              { $$ = $1;}
;
static_class_constant:
    class_namespace_string
    T_PAAMAYIM_NEKUDOTAYIM
    T_STRING                           { _p->onClassConst($$, $1, $3, 1);}
  | T_XHP_LABEL T_PAAMAYIM_NEKUDOTAYIM
    T_STRING                           { $1.xhpLabel();
                                         _p->onClassConst($$, $1, $3, 1);}
;
scalar:
    namespace_string                   { _p->onConstantValue($$, $1);}
  | T_STRING_VARNAME                   { _p->onConstantValue($$, $1);}
  | class_constant                     { $$ = $1;}
  | common_scalar                      { $$ = $1;}
  | '"' encaps_list '"'                { _p->onEncapsList($$,'"',$2);}
  | '\'' encaps_list '\''              { _p->onEncapsList($$,'\'',$2);}
  | T_START_HEREDOC encaps_list
    T_END_HEREDOC                      { _p->onEncapsList($$,T_START_HEREDOC,
                                                          $2);}
;
static_array_pair_list:
    non_empty_static_array_pair_list
    possible_comma                     { $$ = $1;}
  |                                    { $$.reset();}
;
possible_comma:
    ','                                { $$.reset();}
  |                                    { $$.reset();}
;
non_empty_static_array_pair_list:
    non_empty_static_array_pair_list
    ',' static_scalar T_DOUBLE_ARROW
    static_scalar                      { _p->onArrayPair($$,&$1,&$3,$5,0);}
  | non_empty_static_array_pair_list
    ',' static_scalar                  { _p->onArrayPair($$,&$1,  0,$3,0);}
  | static_scalar T_DOUBLE_ARROW
    static_scalar                      { _p->onArrayPair($$,  0,&$1,$3,0);}
  | static_scalar                      { _p->onArrayPair($$,  0,  0,$1,0);}
;

common_scalar_ae:
    T_LNUMBER                          { _p->onScalar($$, T_LNUMBER,  $1);}
  | T_DNUMBER                          { _p->onScalar($$, T_DNUMBER,  $1);}
  | T_CONSTANT_ENCAPSED_STRING         { _p->onScalar($$,
                                         T_CONSTANT_ENCAPSED_STRING,  $1);}
  | T_START_HEREDOC
    T_ENCAPSED_AND_WHITESPACE
    T_END_HEREDOC                      { _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $2);}
  | T_START_HEREDOC
    T_END_HEREDOC                      { $$.setText(""); _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $$);}
;
static_numeric_scalar_ae:
    T_LNUMBER                          { _p->onScalar($$,T_LNUMBER,$1);}
  | T_DNUMBER                          { _p->onScalar($$,T_DNUMBER,$1);}
  | T_STRING                           { constant_ae(_p,$$,$1);}
;
static_scalar_ae:
    common_scalar_ae                   { $$ = $1;}
  | T_STRING                           { constant_ae(_p,$$,$1);}
  | '+' static_numeric_scalar_ae       { UEXP($$,$2,'+',1);}
  | '-' static_numeric_scalar_ae       { UEXP($$,$2,'-',1);}
  | T_ARRAY '('
    static_array_pair_list_ae ')'      { _p->onArray($$,$3,T_ARRAY);}
  | '[' static_array_pair_list_ae ']'  { _p->onArray($$,$2,T_ARRAY);}
;
static_array_pair_list_ae:
    non_empty_static_array_pair_list_ae
    possible_comma                     { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_static_array_pair_list_ae:
    non_empty_static_array_pair_list_ae
    ',' static_scalar_ae T_DOUBLE_ARROW
    static_scalar_ae                   { _p->onArrayPair($$,&$1,&$3,$5,0);}
  | non_empty_static_array_pair_list_ae
    ',' static_scalar_ae               { _p->onArrayPair($$,&$1,  0,$3,0);}
  | static_scalar_ae T_DOUBLE_ARROW
    static_scalar_ae                   { _p->onArrayPair($$,  0,&$1,$3,0);}
  | static_scalar_ae                   { _p->onArrayPair($$,  0,  0,$1,0);}
;
non_empty_static_scalar_list_ae:
    non_empty_static_scalar_list_ae
    ',' static_scalar_ae               { _p->onArrayPair($$,&$1,  0,$3,0);}
  | static_scalar_ae                   { _p->onArrayPair($$,  0,  0,$1,0);}
;
static_scalar_list_ae:
    non_empty_static_scalar_list_ae
    possible_comma                     { $$ = $1;}
  |                                    { $$.reset();}
;
attribute_static_scalar_list:
    '(' static_scalar_list_ae ')'      { _p->onArray($$,$2,T_ARRAY);}
  |                                    { Token t; t.reset();
                                         _p->onArray($$,t,T_ARRAY);}
;
non_empty_user_attribute_list:
    non_empty_user_attribute_list
    ',' T_STRING
    attribute_static_scalar_list       { _p->onUserAttribute($$,&$1,$3,$4);}
  | T_STRING
    attribute_static_scalar_list       { _p->onUserAttribute($$,  0,$1,$2);}
;
user_attribute_list:
                                       { user_attribute_check(_p);}
    non_empty_user_attribute_list
    possible_comma                     { $$ = $2;}
;
non_empty_user_attributes:
    T_SL user_attribute_list T_SR      { $$ = $2;}
;
optional_user_attributes:
    non_empty_user_attributes          { $$ = $1;}
  |                                    { $$.reset();}
;
variable:
    variable_without_objects           { $$ = $1;}
  | simple_function_call               { $$ = $1;}
  | object_method_call                 { $$ = $1;}
  | class_method_call                  { $$ = $1;}
  | dimmable_variable
    '[' dim_offset ']'                 { _p->onRefDim($$, $1, $3);}
  | dimmable_variable '{' expr '}'     { _p->onRefDim($$, $1, $3);}
  | variable T_OBJECT_OPERATOR
    T_STRING                           { _p->onObjectProperty($$,$1,$3);}
  | variable T_OBJECT_OPERATOR
    variable_without_objects           { _p->onObjectProperty($$,$1,$3);}
  | variable T_OBJECT_OPERATOR
    '{' expr '}'                       { _p->onObjectProperty($$,$1,$4);}
  | static_class_name
    T_PAAMAYIM_NEKUDOTAYIM
    variable_without_objects           { _p->onStaticMember($$,$1,$3);}
  | callable_variable '('
    function_call_parameter_list ')'   { _p->onCall($$,1,$1,$3,NULL);}
;

dimmable_variable:
    simple_function_call               { $$ = $1;}
  | object_method_call                 { $$ = $1;}
  | class_method_call                  { $$ = $1;}
  | dimmable_variable
    '[' dim_offset ']'                 { _p->onRefDim($$,$1,$3);}
  | dimmable_variable '{' expr '}'     { _p->onRefDim($$,$1,$3);}
  | variable T_OBJECT_OPERATOR
    T_STRING                           { _p->onObjectProperty($$,$1,$3);}
  | variable T_OBJECT_OPERATOR
    '{' expr '}'                       { _p->onObjectProperty($$,$1,$4);}
  | callable_variable '('
    function_call_parameter_list ')'   { _p->onCall($$,1,$1,$3,NULL);}
;

callable_variable:
    variable_without_objects           { $$ = $1;}
  | dimmable_variable
    '[' dim_offset ']'                 { _p->onRefDim($$, $1, $3);}
  | dimmable_variable '{' expr '}'     { _p->onRefDim($$, $1, $3);}
;

object_method_call:
    variable T_OBJECT_OPERATOR
    T_STRING '('
    function_call_parameter_list ')'   { _p->onObjectMethodCall($$,$1,$3,$5);}
  | variable T_OBJECT_OPERATOR
    variable_without_objects '('
    function_call_parameter_list ')'   { _p->onObjectMethodCall($$,$1,$3,$5);}
  | variable T_OBJECT_OPERATOR
    '{' expr '}' '('
    function_call_parameter_list ')'   { _p->onObjectMethodCall($$,$1,$4,$7);}
;

class_method_call:
    static_class_name
    T_PAAMAYIM_NEKUDOTAYIM
    T_STRING '('
    function_call_parameter_list ')'   { _p->onCall($$,0,$3,$5,&$1);}
  | static_class_name
    T_PAAMAYIM_NEKUDOTAYIM
    variable_without_objects '('
    function_call_parameter_list ')'   { _p->onCall($$,1,$3,$5,&$1);}
;

variable_without_objects:
    reference_variable                 { $$ = $1;}
  | simple_indirect_reference
    reference_variable                 { _p->onIndirectRef($$,$1,$2);}
;

reference_variable:
    reference_variable
    '[' dim_offset ']'                 { _p->onRefDim($$, $1, $3);}
  | reference_variable '{' expr '}'    { _p->onRefDim($$, $1, $3);}
  | compound_variable                  { $$ = $1;}
;
compound_variable:
    T_VARIABLE                         { _p->onSimpleVariable($$, $1);}
  | '$' '{' expr '}'                   { _p->onDynamicVariable($$, $3, 0);}
;
dim_offset:
    expr                               { $$ = $1;}
  |                                    { $$.reset();}
;

simple_indirect_reference:
    '$'                                { $$ = 1;}
  | simple_indirect_reference '$'      { $$++;}
;

variable_no_calls:
    variable_without_objects           { $$ = $1;}
  | dimmable_variable_no_calls
    '[' dim_offset ']'                 { _p->onRefDim($$, $1, $3);}
  | dimmable_variable_no_calls
    '{' expr '}'                       { _p->onRefDim($$, $1, $3);}
  | variable_no_calls T_OBJECT_OPERATOR
    T_STRING                           { _p->onObjectProperty($$,$1,$3);}
  | variable_no_calls T_OBJECT_OPERATOR
    variable_without_objects           { _p->onObjectProperty($$,$1,$3);}
  | variable_no_calls T_OBJECT_OPERATOR
    '{' expr '}'                       { _p->onObjectProperty($$,$1,$4);}
  | static_class_name
    T_PAAMAYIM_NEKUDOTAYIM
    variable_without_objects           { _p->onStaticMember($$,$1,$3);}
;

dimmable_variable_no_calls:
    dimmable_variable_no_calls
    '[' dim_offset ']'                 { _p->onRefDim($$, $1, $3);}
  | dimmable_variable_no_calls
    '{' expr '}'                       { _p->onRefDim($$, $1, $3);}
  | variable_no_calls T_OBJECT_OPERATOR
    T_STRING                           { _p->onObjectProperty($$,$1,$3);}
  | variable_no_calls T_OBJECT_OPERATOR
    '{' expr '}'                       { _p->onObjectProperty($$,$1,$4);}
;

assignment_list:
    assignment_list ','                { _p->onAListVar($$,&$1,NULL);}
  | assignment_list ',' variable       { _p->onAListVar($$,&$1,&$3);}
  | assignment_list ','
    T_LIST '(' assignment_list ')'     { _p->onAListSub($$,&$1,$5);}
  |                                    { _p->onAListVar($$,NULL,NULL);}
  | variable                           { _p->onAListVar($$,NULL,&$1);}
  | T_LIST '(' assignment_list ')'     { _p->onAListSub($$,NULL,$3);}
;

array_pair_list:
    non_empty_array_pair_list
    possible_comma                     { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_array_pair_list:
    non_empty_array_pair_list
    ',' expr T_DOUBLE_ARROW expr       { _p->onArrayPair($$,&$1,&$3,$5,0);}
  | non_empty_array_pair_list ',' expr { _p->onArrayPair($$,&$1,  0,$3,0);}
  | expr T_DOUBLE_ARROW expr           { _p->onArrayPair($$,  0,&$1,$3,0);}
  | expr                               { _p->onArrayPair($$,  0,  0,$1,0);}
  | non_empty_array_pair_list
    ',' expr T_DOUBLE_ARROW
    '&' variable                       { _p->onArrayPair($$,&$1,&$3,$6,1);}
  | non_empty_array_pair_list ','
    '&' variable                       { _p->onArrayPair($$,&$1,  0,$4,1);}
  | expr T_DOUBLE_ARROW '&' variable   { _p->onArrayPair($$,  0,&$1,$4,1);}
  | '&' variable                       { _p->onArrayPair($$,  0,  0,$2,1);}
;

encaps_list:
    encaps_list encaps_var             { _p->addEncap($$, &$1, $2, -1);}
  | encaps_list
    T_ENCAPSED_AND_WHITESPACE          { _p->addEncap($$, &$1, $2, 0);}
  | encaps_var                         { _p->addEncap($$, NULL, $1, -1);}
  | T_ENCAPSED_AND_WHITESPACE
    encaps_var                         { _p->addEncap($$, NULL, $1, 0);
                                         _p->addEncap($$, &$$, $2, -1); }
;

encaps_var:
    T_VARIABLE                         { _p->onSimpleVariable($$, $1);}
  | T_VARIABLE '['
    encaps_var_offset ']'              { _p->encapRefDim($$, $1, $3);}
  | T_VARIABLE T_OBJECT_OPERATOR
    T_STRING                           { _p->encapObjProp($$, $1, $3);}
  | T_DOLLAR_OPEN_CURLY_BRACES
    expr '}'                           { _p->onDynamicVariable($$, $2, 1);}
  | T_DOLLAR_OPEN_CURLY_BRACES
    T_STRING_VARNAME '[' expr ']' '}'  { _p->encapArray($$, $2, $4);}
  | T_CURLY_OPEN variable '}'          { $$ = $2;}
;
encaps_var_offset:
    T_STRING                           { $$ = $1; $$ = T_STRING;}
  | T_NUM_STRING                       { $$ = $1; $$ = T_NUM_STRING;}
  | T_VARIABLE                         { $$ = $1; $$ = T_VARIABLE;}
;

internal_functions:
    T_ISSET '(' variable_list ')'      { UEXP($$,$3,T_ISSET,1);}
  | T_EMPTY '(' variable ')'           { UEXP($$,$3,T_EMPTY,1);}
  | T_INCLUDE expr                     { UEXP($$,$2,T_INCLUDE,1);}
  | T_INCLUDE_ONCE expr                { UEXP($$,$2,T_INCLUDE_ONCE,1);}
  | T_EVAL '(' expr ')'                { UEXP($$,$3,T_EVAL,1);}
  | T_REQUIRE expr                     { UEXP($$,$2,T_REQUIRE,1);}
  | T_REQUIRE_ONCE expr                { UEXP($$,$2,T_REQUIRE_ONCE,1);}
;

variable_list:
    variable                           { _p->onExprListElem($$, NULL, $1);}
  | variable_list ',' variable         { _p->onExprListElem($$, &$1, $3);}
;

class_constant:
  static_class_name
  T_PAAMAYIM_NEKUDOTAYIM T_STRING      { _p->onClassConst($$, $1, $3, 0);}
;

/* strict-mode productions -- these allow some extra stuff in strict
 * mode, but simplify down to the original thing
 */

sm_name_with_type:  /* foo -> int foo */
    T_STRING                           { $$ = $1; }
  | sm_type T_STRING                   { only_in_strict_mode(_p); $$ = $2; }
;

sm_name_with_typevar:  /* foo -> foo<X,Y>; this adds a typevar scope
                        * and must be followed by a call to
                        * popTypeScope() */
    T_STRING                           { _p->pushTypeScope(); $$ = $1; }
  | T_STRING '<' sm_typevar_list '>'   { _p->pushTypeScope(); $$ = $1;
                                         only_in_strict_mode(_p); }
;

sm_typeargs_opt: /* -> <bar<baz>> */
    '<' sm_type_list_gt                { only_in_strict_mode(_p); $$ = $2; }
  |                                    { $$.reset(); }
;

/* this is just  sm_type_list '>'  with a little hack to avoid adding more lexer state */
sm_type_list_gt:
    T_STRING '<' sm_type_list T_SR
  | sm_type '>'
  | sm_type ',' sm_type_list_gt
;

sm_type_list:
    sm_type
  | sm_type ',' sm_type_list
;

sm_opt_return_type:
                                       { $$.reset(); }
  | ':' sm_type                        { only_in_strict_mode(_p); $$ = $2; }
;

sm_typevar_list:
    T_STRING ',' sm_typevar_list       { _p->addTypeVar($1.text()); }
 |  T_STRING                           { _p->addTypeVar($1.text()); }
;

/* extends non_empty_type_decl with some more types */
sm_type:
    /* double-optional types will be rejected by the typechecker; we
     * already allow plenty of nonsense types anyway */
    '?' sm_type                        { only_in_strict_mode(_p); $$.reset(); }
  | T_STRING sm_typeargs_opt           { $$ = $1;
                                         /* if the type annotation is a bound
                                            typevar we have to strip it */
                                         if (_p->scanner().isStrictMode() &&
                                             (_p->isTypeVar($$.text()) ||
                                              !$$.text().compare("mixed") ||
                                              !$$.text().compare("this")
                                             )) {
                                           $$.reset();
                                         }
                                       }
  | T_ARRAY                            { $$.setText("array"); }
  | T_XHP_LABEL                        { $1.xhpLabel(); $$ = $1; }
  | '(' T_FUNCTION '(' sm_type_list ')' ':' sm_type ')'
                                       { only_in_strict_mode(_p); $$.reset(); }
  | '(' T_FUNCTION sm_cast_fix ':' sm_type ')'
                                       { only_in_strict_mode(_p); $$.reset(); }
  | '(' sm_type ',' sm_type_list ')'   { only_in_strict_mode(_p); $$.setText("array"); }
;

/* required, because (int) gets lexed as T_INT_CAST */
sm_cast_fix:
  T_BOOL_CAST                          { $$ = 1;}
| T_INT_CAST                           { $$ = 1;}
| T_DOUBLE_CAST                        { $$ = 1;}
| T_ARRAY_CAST                         { $$ = 1;}
| T_STRING_CAST                        { $$ = 1;}

sm_type_opt:
    sm_type                            { $$ = $1; }
  |                                    { $$.reset(); }
;

%%
bool Parser::parseImpl() {
  return yyparse(this) == 0;
}
