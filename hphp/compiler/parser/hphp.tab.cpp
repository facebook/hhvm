
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
#line 1 "../../../hphp/util/parser/hphp.y"

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
  Token fname;  fname.setText(std::string("hphp_continuation_") + cname);
  Token empty;
  _p->onCall(rhs, false, fname, empty, NULL, true);
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

      switchExp = call;
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

void transform_yield(Parser *_p, Token &stmts, int index,
                     Token *expr, bool assign) {
  // hphp_pack_continuation(v___cont__, label, value)
  Token update;
  {
    Token name;    name.setText(CONTINUATION_OBJECT_NAME);
    Token var;     _p->onSynthesizedVariable(var, name);
    Token param0;  _p->onCallParam(param0, NULL, var, false);

    Token snum;    snum.setText(boost::lexical_cast<std::string>(index));
    Token num;     _p->onScalar(num, T_LNUMBER, snum);
    Token param1;  _p->onCallParam(param1, &param0, num, false);

    Token param2;  _p->onCallParam(param2, &param1, *expr, false);

    Token cname;   cname.setText("hphp_pack_continuation");
    Token call;    _p->onCall(call, false, cname, param2, NULL, true);
    _p->onExpStatement(update, call);
  }

  // return
  Token ret;     _p->onReturn(ret, NULL, false);

  // __yield__N:
  Token lname;   lname.setText(YIELD_LABEL_PREFIX +
                               boost::lexical_cast<std::string>(index));
  Token label;   _p->onLabel(label, lname);
                 _p->addLabel(lname.text(), _p->getLocation(), &label);

  Token stmts0;  _p->onStatementListStart(stmts0);
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

void transform_yield_break(Parser *_p, Token &out) {
  // hphp_continuation_done()
  Token mcall;   prepare_continuation_call(_p, mcall, "done");
  Token done;    _p->onExpStatement(done, mcall);

  // return
  Token ret;     _p->onReturn(ret, NULL, false);

  Token stmts0;  _p->onStatementListStart(stmts0);
  Token stmts1;  _p->addStatement(stmts1, stmts0, done);
  Token stmts2;  _p->addStatement(stmts2, stmts1, ret);
  _p->finishStatement(out, stmts2); out = 1;
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


///////////////////////////////////////////////////////////////////////////////

static int yylex(YYSTYPE *token, HPHP::Location *loc, Parser *_p) {
  return _p->scan(token, loc);
}


/* Line 189 of yacc.c  */
#line 880 "hphp.tab.cpp"

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
     T_COLLECTION = 401
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
#line 1081 "hphp.tab.cpp"

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
#define YYLAST   10434

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  176
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  191
/* YYNRULES -- Number of rules.  */
#define YYNRULES  675
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1254

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   401

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,   174,     2,   171,    47,    31,   175,
     166,   167,    45,    42,     8,    43,    44,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    26,   168,
      36,    13,    37,    25,    51,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    61,     2,   173,    30,     2,   172,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   169,    29,   170,    50,     2,     2,     2,
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
     164,   165
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,     9,    11,    13,    15,    17,
      22,    26,    27,    34,    35,    41,    45,    48,    50,    52,
      54,    56,    58,    60,    64,    66,    68,    71,    75,    80,
      82,    86,    88,    91,    95,    97,   100,   103,   109,   114,
     117,   118,   120,   122,   124,   126,   130,   136,   145,   146,
     151,   152,   159,   160,   171,   172,   177,   180,   184,   187,
     191,   194,   198,   202,   206,   212,   221,   225,   229,   233,
     239,   241,   243,   244,   254,   260,   275,   281,   285,   289,
     292,   295,   305,   306,   307,   313,   315,   316,   318,   319,
     321,   322,   334,   335,   348,   349,   358,   359,   369,   370,
     378,   379,   388,   389,   396,   397,   405,   407,   409,   411,
     413,   415,   418,   421,   424,   425,   428,   429,   432,   433,
     435,   439,   441,   445,   448,   449,   451,   454,   456,   461,
     463,   468,   470,   475,   477,   482,   486,   492,   496,   501,
     506,   512,   518,   523,   524,   526,   528,   533,   534,   540,
     541,   544,   545,   549,   550,   554,   557,   559,   560,   564,
     569,   576,   582,   588,   595,   604,   612,   615,   616,   618,
     621,   625,   630,   634,   636,   638,   641,   646,   650,   656,
     658,   662,   665,   666,   667,   672,   673,   679,   682,   683,
     694,   695,   707,   711,   715,   719,   723,   729,   732,   735,
     736,   743,   749,   754,   758,   760,   762,   766,   771,   773,
     775,   777,   779,   784,   786,   790,   793,   794,   797,   798,
     800,   804,   806,   808,   810,   812,   816,   821,   826,   831,
     833,   835,   838,   841,   844,   848,   852,   854,   856,   858,
     860,   864,   866,   868,   870,   871,   873,   876,   878,   880,
     882,   884,   886,   888,   892,   898,   900,   904,   910,   915,
     919,   923,   927,   931,   933,   935,   936,   938,   940,   942,
     949,   953,   958,   965,   968,   972,   976,   980,   984,   988,
     992,   996,  1000,  1004,  1008,  1012,  1015,  1018,  1021,  1024,
    1028,  1032,  1036,  1040,  1044,  1048,  1052,  1056,  1060,  1064,
    1068,  1072,  1076,  1080,  1084,  1088,  1091,  1094,  1097,  1100,
    1104,  1108,  1112,  1116,  1120,  1124,  1128,  1132,  1136,  1140,
    1146,  1151,  1153,  1156,  1159,  1162,  1165,  1168,  1171,  1174,
    1177,  1180,  1182,  1184,  1188,  1191,  1192,  1204,  1205,  1218,
    1220,  1222,  1224,  1229,  1234,  1239,  1244,  1249,  1251,  1253,
    1257,  1263,  1264,  1268,  1273,  1275,  1278,  1283,  1286,  1293,
    1294,  1296,  1301,  1302,  1305,  1306,  1308,  1310,  1314,  1316,
    1320,  1322,  1324,  1328,  1332,  1334,  1336,  1338,  1340,  1342,
    1344,  1346,  1348,  1350,  1352,  1354,  1356,  1358,  1360,  1362,
    1364,  1366,  1368,  1370,  1372,  1374,  1376,  1378,  1380,  1382,
    1384,  1386,  1388,  1390,  1392,  1394,  1396,  1398,  1400,  1402,
    1404,  1406,  1408,  1410,  1412,  1414,  1416,  1418,  1420,  1422,
    1424,  1426,  1428,  1430,  1432,  1434,  1436,  1438,  1440,  1442,
    1444,  1446,  1448,  1450,  1452,  1454,  1456,  1458,  1460,  1462,
    1464,  1466,  1468,  1470,  1472,  1474,  1476,  1478,  1483,  1485,
    1487,  1489,  1491,  1493,  1495,  1497,  1499,  1502,  1504,  1505,
    1506,  1508,  1510,  1514,  1515,  1517,  1519,  1521,  1523,  1525,
    1527,  1529,  1531,  1533,  1535,  1537,  1541,  1544,  1546,  1548,
    1551,  1554,  1559,  1561,  1563,  1567,  1571,  1573,  1575,  1577,
    1579,  1583,  1587,  1591,  1594,  1595,  1597,  1598,  1600,  1601,
    1607,  1611,  1615,  1617,  1619,  1621,  1623,  1627,  1630,  1632,
    1634,  1636,  1638,  1640,  1643,  1646,  1651,  1654,  1655,  1661,
    1665,  1669,  1671,  1675,  1677,  1680,  1681,  1685,  1686,  1691,
    1694,  1695,  1699,  1703,  1705,  1706,  1708,  1711,  1714,  1719,
    1723,  1727,  1730,  1735,  1738,  1743,  1745,  1747,  1749,  1751,
    1753,  1756,  1761,  1765,  1770,  1774,  1776,  1778,  1780,  1782,
    1785,  1790,  1795,  1799,  1801,  1803,  1807,  1815,  1822,  1831,
    1841,  1850,  1861,  1869,  1876,  1878,  1881,  1886,  1891,  1893,
    1895,  1900,  1902,  1903,  1905,  1908,  1910,  1912,  1915,  1920,
    1924,  1928,  1929,  1931,  1934,  1939,  1943,  1946,  1950,  1957,
    1958,  1960,  1965,  1968,  1969,  1975,  1979,  1983,  1985,  1992,
    1997,  2002,  2005,  2008,  2009,  2015,  2019,  2023,  2025,  2028,
    2029,  2035,  2039,  2043,  2045,  2048,  2051,  2053,  2056,  2058,
    2063,  2067,  2071,  2078,  2082,  2084,  2086,  2088,  2093,  2098,
    2101,  2104,  2109,  2112,  2115,  2117,  2121,  2125,  2127,  2130,
    2132,  2137,  2141,  2142,  2144,  2148,  2152,  2154,  2156,  2157,
    2158,  2161,  2165,  2167,  2173,  2177,  2180,  2183,  2186,  2188,
    2193,  2200,  2202,  2211,  2217,  2219
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     177,     0,    -1,   178,    -1,   178,   179,    -1,    -1,   193,
      -1,   205,    -1,   208,    -1,   213,    -1,   116,   166,   167,
     168,    -1,   141,   185,   168,    -1,    -1,   141,   185,   169,
     180,   178,   170,    -1,    -1,   141,   169,   181,   178,   170,
      -1,   104,   183,   168,    -1,   190,   168,    -1,    71,    -1,
     148,    -1,   149,    -1,   151,    -1,   153,    -1,   152,    -1,
     183,     8,   184,    -1,   184,    -1,   185,    -1,   144,   185,
      -1,   185,    90,   182,    -1,   144,   185,    90,   182,    -1,
     182,    -1,   185,   144,   182,    -1,   185,    -1,   144,   185,
      -1,   141,   144,   185,    -1,   186,    -1,   186,   360,    -1,
     186,   360,    -1,   190,     8,   358,    13,   308,    -1,    99,
     358,    13,   308,    -1,   191,   192,    -1,    -1,   193,    -1,
     205,    -1,   208,    -1,   213,    -1,   169,   191,   170,    -1,
      65,   276,   193,   235,   237,    -1,    65,   276,    26,   191,
     236,   238,    68,   168,    -1,    -1,    82,   276,   194,   229,
      -1,    -1,    81,   195,   193,    82,   276,   168,    -1,    -1,
      84,   166,   278,   168,   278,   168,   278,   167,   196,   227,
      -1,    -1,    91,   276,   197,   232,    -1,    95,   168,    -1,
      95,   279,   168,    -1,    97,   168,    -1,    97,   279,   168,
      -1,   100,   168,    -1,   100,   279,   168,    -1,   145,    95,
     168,    -1,   145,   279,   168,    -1,   333,    13,   145,   279,
     168,    -1,   123,   166,   345,   167,    13,   145,   279,   168,
      -1,   105,   243,   168,    -1,   111,   245,   168,    -1,    80,
     277,   168,    -1,   113,   166,   356,   167,   168,    -1,   168,
      -1,    75,    -1,    -1,    86,   166,   279,    90,   226,   225,
     167,   198,   228,    -1,    88,   166,   231,   167,   230,    -1,
     101,   169,   191,   170,   102,   166,   301,    73,   167,   169,
     191,   170,   199,   202,    -1,   101,   169,   191,   170,   200,
      -1,   103,   279,   168,    -1,    96,   182,   168,    -1,   279,
     168,    -1,   182,    26,    -1,   199,   102,   166,   301,    73,
     167,   169,   191,   170,    -1,    -1,    -1,   201,   159,   169,
     191,   170,    -1,   200,    -1,    -1,    31,    -1,    -1,    98,
      -1,    -1,   204,   203,   359,   206,   166,   239,   167,   363,
     169,   191,   170,    -1,    -1,   326,   204,   203,   359,   207,
     166,   239,   167,   363,   169,   191,   170,    -1,    -1,   219,
     216,   209,   220,   221,   169,   246,   170,    -1,    -1,   326,
     219,   216,   210,   220,   221,   169,   246,   170,    -1,    -1,
     118,   217,   211,   222,   169,   246,   170,    -1,    -1,   326,
     118,   217,   212,   222,   169,   246,   170,    -1,    -1,   154,
     218,   214,   169,   246,   170,    -1,    -1,   326,   154,   218,
     215,   169,   246,   170,    -1,   359,    -1,   146,    -1,   359,
      -1,   359,    -1,   117,    -1,   110,   117,    -1,   109,   117,
      -1,   119,   301,    -1,    -1,   120,   223,    -1,    -1,   119,
     223,    -1,    -1,   301,    -1,   223,     8,   301,    -1,   301,
      -1,   224,     8,   301,    -1,   122,   226,    -1,    -1,   333,
      -1,    31,   333,    -1,   193,    -1,    26,   191,    85,   168,
      -1,   193,    -1,    26,   191,    87,   168,    -1,   193,    -1,
      26,   191,    83,   168,    -1,   193,    -1,    26,   191,    89,
     168,    -1,   182,    13,   308,    -1,   231,     8,   182,    13,
     308,    -1,   169,   233,   170,    -1,   169,   168,   233,   170,
      -1,    26,   233,    92,   168,    -1,    26,   168,   233,    92,
     168,    -1,   233,    93,   279,   234,   191,    -1,   233,    94,
     234,   191,    -1,    -1,    26,    -1,   168,    -1,   235,    66,
     276,   193,    -1,    -1,   236,    66,   276,    26,   191,    -1,
      -1,    67,   193,    -1,    -1,    67,    26,   191,    -1,    -1,
     240,     8,   157,    -1,   240,   313,    -1,   157,    -1,    -1,
     327,   366,    73,    -1,   327,   366,    31,    73,    -1,   327,
     366,    31,    73,    13,   308,    -1,   327,   366,    73,    13,
     308,    -1,   240,     8,   327,   366,    73,    -1,   240,     8,
     327,   366,    31,    73,    -1,   240,     8,   327,   366,    31,
      73,    13,   308,    -1,   240,     8,   327,   366,    73,    13,
     308,    -1,   242,   313,    -1,    -1,   279,    -1,    31,   333,
      -1,   242,     8,   279,    -1,   242,     8,    31,   333,    -1,
     243,     8,   244,    -1,   244,    -1,    73,    -1,   171,   333,
      -1,   171,   169,   279,   170,    -1,   245,     8,    73,    -1,
     245,     8,    73,    13,   308,    -1,    73,    -1,    73,    13,
     308,    -1,   246,   247,    -1,    -1,    -1,   269,   248,   273,
     168,    -1,    -1,   271,   365,   249,   273,   168,    -1,   274,
     168,    -1,    -1,   270,   204,   203,   359,   166,   250,   239,
     167,   363,   268,    -1,    -1,   326,   270,   204,   203,   359,
     166,   251,   239,   167,   363,   268,    -1,   148,   256,   168,
      -1,   149,   262,   168,    -1,   151,   264,   168,    -1,   104,
     224,   168,    -1,   104,   224,   169,   252,   170,    -1,   252,
     253,    -1,   252,   254,    -1,    -1,   189,   140,   182,   155,
     224,   168,    -1,   255,    90,   270,   182,   168,    -1,   255,
      90,   271,   168,    -1,   189,   140,   182,    -1,   182,    -1,
     257,    -1,   256,     8,   257,    -1,   258,   298,   260,   261,
      -1,   146,    -1,   124,    -1,   301,    -1,   112,    -1,   152,
     169,   259,   170,    -1,   307,    -1,   259,     8,   307,    -1,
      13,   308,    -1,    -1,    51,   153,    -1,    -1,   263,    -1,
     262,     8,   263,    -1,   150,    -1,   265,    -1,   182,    -1,
     115,    -1,   166,   266,   167,    -1,   166,   266,   167,    45,
      -1,   166,   266,   167,    25,    -1,   166,   266,   167,    42,
      -1,   265,    -1,   267,    -1,   267,    45,    -1,   267,    25,
      -1,   267,    42,    -1,   266,     8,   266,    -1,   266,    29,
     266,    -1,   182,    -1,   146,    -1,   150,    -1,   168,    -1,
     169,   191,   170,    -1,   271,    -1,   112,    -1,   271,    -1,
      -1,   272,    -1,   271,   272,    -1,   106,    -1,   107,    -1,
     108,    -1,   111,    -1,   110,    -1,   109,    -1,   273,     8,
      73,    -1,   273,     8,    73,    13,   308,    -1,    73,    -1,
      73,    13,   308,    -1,   274,     8,   358,    13,   308,    -1,
      99,   358,    13,   308,    -1,    63,   303,   306,    -1,   166,
     275,   167,    -1,   166,   279,   167,    -1,   277,     8,   279,
      -1,   279,    -1,   277,    -1,    -1,   280,    -1,   333,    -1,
     275,    -1,   123,   166,   345,   167,    13,   279,    -1,   333,
      13,   279,    -1,   333,    13,    31,   333,    -1,   333,    13,
      31,    63,   303,   306,    -1,    62,   279,    -1,   333,    24,
     279,    -1,   333,    23,   279,    -1,   333,    22,   279,    -1,
     333,    21,   279,    -1,   333,    20,   279,    -1,   333,    19,
     279,    -1,   333,    18,   279,    -1,   333,    17,   279,    -1,
     333,    16,   279,    -1,   333,    15,   279,    -1,   333,    14,
     279,    -1,   333,    60,    -1,    60,   333,    -1,   333,    59,
      -1,    59,   333,    -1,   279,    27,   279,    -1,   279,    28,
     279,    -1,   279,     9,   279,    -1,   279,    11,   279,    -1,
     279,    10,   279,    -1,   279,    29,   279,    -1,   279,    31,
     279,    -1,   279,    30,   279,    -1,   279,    44,   279,    -1,
     279,    42,   279,    -1,   279,    43,   279,    -1,   279,    45,
     279,    -1,   279,    46,   279,    -1,   279,    47,   279,    -1,
     279,    41,   279,    -1,   279,    40,   279,    -1,    42,   279,
      -1,    43,   279,    -1,    48,   279,    -1,    50,   279,    -1,
     279,    33,   279,    -1,   279,    32,   279,    -1,   279,    35,
     279,    -1,   279,    34,   279,    -1,   279,    36,   279,    -1,
     279,    39,   279,    -1,   279,    37,   279,    -1,   279,    38,
     279,    -1,   279,    49,   303,    -1,   166,   280,   167,    -1,
     279,    25,   279,    26,   279,    -1,   279,    25,    26,   279,
      -1,   355,    -1,    58,   279,    -1,    57,   279,    -1,    56,
     279,    -1,    55,   279,    -1,    54,   279,    -1,    53,   279,
      -1,    52,   279,    -1,    64,   304,    -1,    51,   279,    -1,
     310,    -1,   283,    -1,   172,   305,   172,    -1,    12,   279,
      -1,    -1,   204,   203,   166,   281,   239,   167,   363,   288,
     169,   191,   170,    -1,    -1,   111,   204,   203,   166,   282,
     239,   167,   363,   288,   169,   191,   170,    -1,   290,    -1,
     286,    -1,   284,    -1,   124,   166,   346,   167,    -1,   301,
     169,   348,   170,    -1,   301,   169,   350,   170,    -1,   286,
      61,   341,   173,    -1,   287,    61,   341,   173,    -1,   283,
      -1,   357,    -1,   166,   280,   167,    -1,   104,   166,   289,
     313,   167,    -1,    -1,   289,     8,    73,    -1,   289,     8,
      31,    73,    -1,    73,    -1,    31,    73,    -1,   160,   146,
     291,   161,    -1,   293,    46,    -1,   293,   161,   294,   160,
      46,   292,    -1,    -1,   146,    -1,   293,   295,    13,   296,
      -1,    -1,   294,   297,    -1,    -1,   146,    -1,   147,    -1,
     169,   279,   170,    -1,   147,    -1,   169,   279,   170,    -1,
     290,    -1,   299,    -1,   298,    26,   299,    -1,   298,    43,
     299,    -1,   182,    -1,    64,    -1,    98,    -1,    99,    -1,
     100,    -1,   145,    -1,   101,    -1,   102,    -1,   159,    -1,
     103,    -1,    65,    -1,    66,    -1,    68,    -1,    67,    -1,
      82,    -1,    83,    -1,    81,    -1,    84,    -1,    85,    -1,
      86,    -1,    87,    -1,    88,    -1,    89,    -1,    49,    -1,
      90,    -1,    91,    -1,    92,    -1,    93,    -1,    94,    -1,
      95,    -1,    97,    -1,    96,    -1,    80,    -1,    12,    -1,
     117,    -1,   118,    -1,   119,    -1,   120,    -1,    63,    -1,
      62,    -1,   112,    -1,     5,    -1,     7,    -1,     6,    -1,
       4,    -1,     3,    -1,   141,    -1,   104,    -1,   105,    -1,
     114,    -1,   115,    -1,   116,    -1,   111,    -1,   110,    -1,
     109,    -1,   108,    -1,   107,    -1,   106,    -1,   113,    -1,
     123,    -1,   124,    -1,     9,    -1,    11,    -1,    10,    -1,
     125,    -1,   127,    -1,   126,    -1,   128,    -1,   129,    -1,
     143,    -1,   142,    -1,   154,    -1,   156,    -1,   188,   166,
     241,   167,    -1,   189,    -1,   146,    -1,   301,    -1,   111,
      -1,   339,    -1,   301,    -1,   111,    -1,   343,    -1,   166,
     167,    -1,   276,    -1,    -1,    -1,    78,    -1,   352,    -1,
     166,   241,   167,    -1,    -1,    69,    -1,    70,    -1,    79,
      -1,   128,    -1,   129,    -1,   143,    -1,   125,    -1,   156,
      -1,   126,    -1,   127,    -1,   142,    -1,   136,    78,   137,
      -1,   136,   137,    -1,   307,    -1,   187,    -1,    42,   308,
      -1,    43,   308,    -1,   124,   166,   311,   167,    -1,   309,
      -1,   285,    -1,   189,   140,   182,    -1,   146,   140,   182,
      -1,   187,    -1,    72,    -1,   357,    -1,   307,    -1,   174,
     352,   174,    -1,   175,   352,   175,    -1,   136,   352,   137,
      -1,   314,   312,    -1,    -1,     8,    -1,    -1,     8,    -1,
      -1,   314,     8,   308,   122,   308,    -1,   314,     8,   308,
      -1,   308,   122,   308,    -1,   308,    -1,    69,    -1,    70,
      -1,    79,    -1,   136,    78,   137,    -1,   136,   137,    -1,
      69,    -1,    70,    -1,   182,    -1,   315,    -1,   182,    -1,
      42,   316,    -1,    43,   316,    -1,   124,   166,   318,   167,
      -1,   319,   312,    -1,    -1,   319,     8,   317,   122,   317,
      -1,   319,     8,   317,    -1,   317,   122,   317,    -1,   317,
      -1,   320,     8,   317,    -1,   317,    -1,   320,   312,    -1,
      -1,   166,   321,   167,    -1,    -1,   323,     8,   182,   322,
      -1,   182,   322,    -1,    -1,   325,   323,   312,    -1,    41,
     324,    40,    -1,   326,    -1,    -1,   329,    -1,   121,   338,
      -1,   121,   182,    -1,   121,   169,   279,   170,    -1,    61,
     341,   173,    -1,   169,   279,   170,    -1,   334,   330,    -1,
     166,   275,   167,   330,    -1,   344,   330,    -1,   166,   275,
     167,   330,    -1,   338,    -1,   300,    -1,   336,    -1,   337,
      -1,   331,    -1,   333,   328,    -1,   166,   275,   167,   328,
      -1,   302,   140,   338,    -1,   335,   166,   241,   167,    -1,
     166,   333,   167,    -1,   300,    -1,   336,    -1,   337,    -1,
     331,    -1,   333,   329,    -1,   166,   275,   167,   329,    -1,
     335,   166,   241,   167,    -1,   166,   333,   167,    -1,   338,
      -1,   331,    -1,   166,   333,   167,    -1,   333,   121,   182,
     360,   166,   241,   167,    -1,   333,   121,   338,   166,   241,
     167,    -1,   333,   121,   169,   279,   170,   166,   241,   167,
      -1,   166,   275,   167,   121,   182,   360,   166,   241,   167,
      -1,   166,   275,   167,   121,   338,   166,   241,   167,    -1,
     166,   275,   167,   121,   169,   279,   170,   166,   241,   167,
      -1,   302,   140,   182,   360,   166,   241,   167,    -1,   302,
     140,   338,   166,   241,   167,    -1,   339,    -1,   342,   339,
      -1,   339,    61,   341,   173,    -1,   339,   169,   279,   170,
      -1,   340,    -1,    73,    -1,   171,   169,   279,   170,    -1,
     279,    -1,    -1,   171,    -1,   342,   171,    -1,   338,    -1,
     332,    -1,   343,   328,    -1,   166,   275,   167,   328,    -1,
     302,   140,   338,    -1,   166,   333,   167,    -1,    -1,   332,
      -1,   343,   329,    -1,   166,   275,   167,   329,    -1,   166,
     333,   167,    -1,   345,     8,    -1,   345,     8,   333,    -1,
     345,     8,   123,   166,   345,   167,    -1,    -1,   333,    -1,
     123,   166,   345,   167,    -1,   347,   312,    -1,    -1,   347,
       8,   279,   122,   279,    -1,   347,     8,   279,    -1,   279,
     122,   279,    -1,   279,    -1,   347,     8,   279,   122,    31,
     333,    -1,   347,     8,    31,   333,    -1,   279,   122,    31,
     333,    -1,    31,   333,    -1,   349,   312,    -1,    -1,   349,
       8,   279,   122,   279,    -1,   349,     8,   279,    -1,   279,
     122,   279,    -1,   279,    -1,   351,   312,    -1,    -1,   351,
       8,   308,   122,   308,    -1,   351,     8,   308,    -1,   308,
     122,   308,    -1,   308,    -1,   352,   353,    -1,   352,    78,
      -1,   353,    -1,    78,   353,    -1,    73,    -1,    73,    61,
     354,   173,    -1,    73,   121,   182,    -1,   138,   279,   170,
      -1,   138,    72,    61,   279,   173,   170,    -1,   139,   333,
     170,    -1,   182,    -1,    74,    -1,    73,    -1,   114,   166,
     356,   167,    -1,   115,   166,   333,   167,    -1,     7,   279,
      -1,     6,   279,    -1,     5,   166,   279,   167,    -1,     4,
     279,    -1,     3,   279,    -1,   333,    -1,   356,     8,   333,
      -1,   302,   140,   182,    -1,   182,    -1,   365,   182,    -1,
     182,    -1,   182,   162,   364,   163,    -1,   162,   361,   163,
      -1,    -1,   365,    -1,   361,     8,   365,    -1,   361,     8,
     157,    -1,   361,    -1,   157,    -1,    -1,    -1,    26,   365,
      -1,   182,     8,   364,    -1,   182,    -1,   182,    90,   182,
       8,   364,    -1,   182,    90,   182,    -1,    25,   365,    -1,
      51,   365,    -1,   182,   360,    -1,   124,    -1,   124,   162,
     365,   163,    -1,   124,   162,   365,     8,   365,   163,    -1,
     146,    -1,   166,    98,   166,   362,   167,    26,   365,   167,
      -1,   166,   361,     8,   365,   167,    -1,   365,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   934,   934,   939,   941,   944,   946,   947,   948,   949,
     950,   952,   952,   954,   954,   956,   957,   962,   963,   964,
     965,   966,   967,   971,   973,   976,   977,   978,   979,   984,
     985,   989,   990,   991,   996,  1001,  1007,  1013,  1016,  1021,
    1023,  1026,  1027,  1028,  1029,  1032,  1033,  1037,  1042,  1042,
    1046,  1046,  1051,  1050,  1054,  1054,  1057,  1058,  1059,  1060,
    1061,  1062,  1063,  1064,  1065,  1066,  1068,  1069,  1070,  1071,
    1072,  1073,  1076,  1074,  1079,  1081,  1089,  1092,  1093,  1097,
    1098,  1105,  1111,  1115,  1115,  1121,  1122,  1126,  1127,  1131,
    1136,  1135,  1146,  1145,  1159,  1158,  1177,  1175,  1194,  1193,
    1202,  1200,  1212,  1211,  1222,  1220,  1232,  1233,  1237,  1240,
    1243,  1244,  1245,  1248,  1250,  1253,  1254,  1257,  1258,  1261,
    1262,  1266,  1267,  1272,  1273,  1276,  1277,  1281,  1282,  1286,
    1287,  1291,  1292,  1296,  1297,  1302,  1303,  1308,  1309,  1310,
    1311,  1314,  1317,  1319,  1322,  1323,  1327,  1329,  1332,  1335,
    1338,  1339,  1342,  1343,  1347,  1349,  1351,  1352,  1356,  1358,
    1360,  1363,  1366,  1369,  1372,  1376,  1383,  1385,  1388,  1389,
    1390,  1392,  1397,  1398,  1401,  1402,  1403,  1407,  1408,  1410,
    1411,  1415,  1417,  1420,  1420,  1424,  1423,  1427,  1431,  1429,
    1443,  1440,  1452,  1454,  1456,  1458,  1460,  1464,  1465,  1466,
    1469,  1475,  1478,  1484,  1487,  1492,  1494,  1499,  1504,  1508,
    1509,  1515,  1516,  1521,  1522,  1527,  1528,  1532,  1533,  1537,
    1539,  1545,  1550,  1551,  1553,  1557,  1558,  1559,  1560,  1564,
    1565,  1566,  1567,  1568,  1569,  1571,  1576,  1579,  1580,  1584,
    1585,  1588,  1589,  1592,  1593,  1596,  1597,  1601,  1602,  1603,
    1604,  1605,  1606,  1609,  1611,  1613,  1614,  1617,  1619,  1623,
    1625,  1629,  1633,  1634,  1638,  1639,  1643,  1644,  1645,  1648,
    1650,  1651,  1652,  1655,  1656,  1657,  1658,  1659,  1660,  1661,
    1662,  1663,  1664,  1665,  1666,  1667,  1668,  1669,  1670,  1671,
    1672,  1673,  1674,  1675,  1676,  1677,  1678,  1679,  1680,  1681,
    1682,  1683,  1684,  1685,  1686,  1687,  1688,  1689,  1690,  1691,
    1692,  1693,  1694,  1695,  1696,  1698,  1699,  1701,  1703,  1704,
    1705,  1706,  1707,  1708,  1709,  1710,  1711,  1712,  1713,  1714,
    1715,  1716,  1717,  1718,  1719,  1721,  1720,  1729,  1728,  1736,
    1737,  1738,  1742,  1746,  1753,  1760,  1762,  1767,  1768,  1769,
    1773,  1777,  1781,  1782,  1783,  1784,  1788,  1794,  1799,  1808,
    1809,  1812,  1815,  1818,  1819,  1822,  1826,  1829,  1832,  1839,
    1840,  1844,  1845,  1847,  1851,  1852,  1853,  1854,  1855,  1856,
    1857,  1858,  1859,  1860,  1861,  1862,  1863,  1864,  1865,  1866,
    1867,  1868,  1869,  1870,  1871,  1872,  1873,  1874,  1875,  1876,
    1877,  1878,  1879,  1880,  1881,  1882,  1883,  1884,  1885,  1886,
    1887,  1888,  1889,  1890,  1891,  1892,  1893,  1894,  1895,  1896,
    1897,  1898,  1899,  1900,  1901,  1902,  1903,  1904,  1905,  1906,
    1907,  1908,  1909,  1910,  1911,  1912,  1913,  1914,  1915,  1916,
    1917,  1918,  1919,  1920,  1921,  1922,  1923,  1927,  1932,  1933,
    1936,  1937,  1938,  1942,  1943,  1944,  1948,  1949,  1950,  1954,
    1955,  1956,  1959,  1961,  1965,  1966,  1967,  1969,  1970,  1971,
    1972,  1973,  1974,  1975,  1976,  1977,  1980,  1985,  1986,  1987,
    1988,  1989,  1991,  1992,  1995,  1998,  2003,  2004,  2005,  2006,
    2007,  2008,  2009,  2014,  2016,  2019,  2020,  2023,  2024,  2027,
    2030,  2032,  2034,  2038,  2039,  2040,  2042,  2045,  2049,  2050,
    2051,  2054,  2055,  2056,  2057,  2058,  2062,  2064,  2067,  2070,
    2072,  2074,  2077,  2079,  2082,  2084,  2087,  2088,  2092,  2095,
    2099,  2099,  2104,  2107,  2108,  2112,  2113,  2118,  2119,  2123,
    2124,  2128,  2129,  2133,  2135,  2139,  2140,  2141,  2142,  2143,
    2144,  2145,  2146,  2149,  2151,  2155,  2156,  2157,  2158,  2159,
    2161,  2163,  2165,  2169,  2170,  2171,  2175,  2178,  2181,  2184,
    2187,  2190,  2196,  2200,  2207,  2208,  2213,  2215,  2216,  2219,
    2220,  2223,  2224,  2228,  2229,  2233,  2234,  2235,  2236,  2237,
    2240,  2243,  2244,  2245,  2247,  2249,  2253,  2254,  2255,  2257,
    2258,  2259,  2263,  2265,  2268,  2270,  2271,  2272,  2273,  2276,
    2278,  2279,  2283,  2285,  2288,  2290,  2291,  2292,  2296,  2298,
    2301,  2304,  2306,  2308,  2312,  2313,  2315,  2316,  2322,  2323,
    2325,  2327,  2329,  2331,  2334,  2335,  2336,  2340,  2341,  2342,
    2343,  2344,  2345,  2346,  2350,  2351,  2355,  2364,  2365,  2371,
    2372,  2380,  2383,  2387,  2388,  2392,  2393,  2394,  2395,  2399,
    2400,  2404,  2405,  2406,  2408,  2416,  2417,  2418,  2429,  2430,
    2433,  2436,  2437,  2440,  2444,  2445
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
  "T_TYPELIST_GT", "T_UNRESOLVED_LT", "T_COLLECTION", "'('", "')'", "';'",
  "'{'", "'}'", "'$'", "'`'", "']'", "'\"'", "'\\''", "$accept", "start",
  "top_statement_list", "top_statement", "$@1", "$@2", "ident",
  "use_declarations", "use_declaration", "namespace_name",
  "namespace_string_base", "namespace_string", "namespace_string_typeargs",
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
  "parenthesis_expr", "expr_list", "for_expr", "expr", "expr_no_variable",
  "$@21", "$@22", "array_literal", "collection_literal",
  "static_collection_literal", "dim_expr", "dim_expr_base", "lexical_vars",
  "lexical_var_list", "xhp_tag", "xhp_tag_body", "xhp_opt_end_label",
  "xhp_attributes", "xhp_children", "xhp_attribute_name",
  "xhp_attribute_value", "xhp_child", "xhp_label_ws", "xhp_bareword",
  "simple_function_call", "fully_qualified_class_name",
  "static_class_name", "class_name_reference", "exit_expr",
  "backticks_expr", "ctor_arguments", "common_scalar", "static_scalar",
  "static_class_constant", "scalar", "static_array_pair_list",
  "possible_comma", "possible_comma_in_hphp_syntax",
  "non_empty_static_array_pair_list", "common_scalar_ae",
  "static_numeric_scalar_ae", "static_scalar_ae",
  "static_array_pair_list_ae", "non_empty_static_array_pair_list_ae",
  "non_empty_static_scalar_list_ae", "static_scalar_list_ae",
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
  "class_constant", "sm_name_with_type", "sm_name_with_typevar",
  "sm_typeargs_opt", "sm_type_list", "sm_func_type_list",
  "sm_opt_return_type", "sm_typevar_list", "sm_type", "sm_type_opt", 0
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
     396,   397,   398,   399,   400,   401,    40,    41,    59,   123,
     125,    36,    96,    93,    34,    39
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   176,   177,   178,   178,   179,   179,   179,   179,   179,
     179,   180,   179,   181,   179,   179,   179,   182,   182,   182,
     182,   182,   182,   183,   183,   184,   184,   184,   184,   185,
     185,   186,   186,   186,   187,   188,   189,   190,   190,   191,
     191,   192,   192,   192,   192,   193,   193,   193,   194,   193,
     195,   193,   196,   193,   197,   193,   193,   193,   193,   193,
     193,   193,   193,   193,   193,   193,   193,   193,   193,   193,
     193,   193,   198,   193,   193,   193,   193,   193,   193,   193,
     193,   199,   199,   201,   200,   202,   202,   203,   203,   204,
     206,   205,   207,   205,   209,   208,   210,   208,   211,   208,
     212,   208,   214,   213,   215,   213,   216,   216,   217,   218,
     219,   219,   219,   220,   220,   221,   221,   222,   222,   223,
     223,   224,   224,   225,   225,   226,   226,   227,   227,   228,
     228,   229,   229,   230,   230,   231,   231,   232,   232,   232,
     232,   233,   233,   233,   234,   234,   235,   235,   236,   236,
     237,   237,   238,   238,   239,   239,   239,   239,   240,   240,
     240,   240,   240,   240,   240,   240,   241,   241,   242,   242,
     242,   242,   243,   243,   244,   244,   244,   245,   245,   245,
     245,   246,   246,   248,   247,   249,   247,   247,   250,   247,
     251,   247,   247,   247,   247,   247,   247,   252,   252,   252,
     253,   254,   254,   255,   255,   256,   256,   257,   257,   258,
     258,   258,   258,   259,   259,   260,   260,   261,   261,   262,
     262,   263,   264,   264,   264,   265,   265,   265,   265,   266,
     266,   266,   266,   266,   266,   266,   267,   267,   267,   268,
     268,   269,   269,   270,   270,   271,   271,   272,   272,   272,
     272,   272,   272,   273,   273,   273,   273,   274,   274,   275,
     275,   276,   277,   277,   278,   278,   279,   279,   279,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   281,   280,   282,   280,   280,
     280,   280,   283,   284,   285,   286,   286,   287,   287,   287,
     288,   288,   289,   289,   289,   289,   290,   291,   291,   292,
     292,   293,   293,   294,   294,   295,   296,   296,   297,   297,
     297,   298,   298,   298,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   300,   301,   301,
     302,   302,   302,   303,   303,   303,   304,   304,   304,   305,
     305,   305,   306,   306,   307,   307,   307,   307,   307,   307,
     307,   307,   307,   307,   307,   307,   307,   308,   308,   308,
     308,   308,   308,   308,   309,   309,   310,   310,   310,   310,
     310,   310,   310,   311,   311,   312,   312,   313,   313,   314,
     314,   314,   314,   315,   315,   315,   315,   315,   316,   316,
     316,   317,   317,   317,   317,   317,   318,   318,   319,   319,
     319,   319,   320,   320,   321,   321,   322,   322,   323,   323,
     325,   324,   326,   327,   327,   328,   328,   329,   329,   330,
     330,   331,   331,   332,   332,   333,   333,   333,   333,   333,
     333,   333,   333,   333,   333,   334,   334,   334,   334,   334,
     334,   334,   334,   335,   335,   335,   336,   336,   336,   336,
     336,   336,   337,   337,   338,   338,   339,   339,   339,   340,
     340,   341,   341,   342,   342,   343,   343,   343,   343,   343,
     343,   344,   344,   344,   344,   344,   345,   345,   345,   345,
     345,   345,   346,   346,   347,   347,   347,   347,   347,   347,
     347,   347,   348,   348,   349,   349,   349,   349,   350,   350,
     351,   351,   351,   351,   352,   352,   352,   352,   353,   353,
     353,   353,   353,   353,   354,   354,   354,   355,   355,   355,
     355,   355,   355,   355,   356,   356,   357,   358,   358,   359,
     359,   360,   360,   361,   361,   362,   362,   362,   362,   363,
     363,   364,   364,   364,   364,   365,   365,   365,   365,   365,
     365,   365,   365,   365,   366,   366
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     1,     1,     1,     1,     4,
       3,     0,     6,     0,     5,     3,     2,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     2,     3,     4,     1,
       3,     1,     2,     3,     1,     2,     2,     5,     4,     2,
       0,     1,     1,     1,     1,     3,     5,     8,     0,     4,
       0,     6,     0,    10,     0,     4,     2,     3,     2,     3,
       2,     3,     3,     3,     5,     8,     3,     3,     3,     5,
       1,     1,     0,     9,     5,    14,     5,     3,     3,     2,
       2,     9,     0,     0,     5,     1,     0,     1,     0,     1,
       0,    11,     0,    12,     0,     8,     0,     9,     0,     7,
       0,     8,     0,     6,     0,     7,     1,     1,     1,     1,
       1,     2,     2,     2,     0,     2,     0,     2,     0,     1,
       3,     1,     3,     2,     0,     1,     2,     1,     4,     1,
       4,     1,     4,     1,     4,     3,     5,     3,     4,     4,
       5,     5,     4,     0,     1,     1,     4,     0,     5,     0,
       2,     0,     3,     0,     3,     2,     1,     0,     3,     4,
       6,     5,     5,     6,     8,     7,     2,     0,     1,     2,
       3,     4,     3,     1,     1,     2,     4,     3,     5,     1,
       3,     2,     0,     0,     4,     0,     5,     2,     0,    10,
       0,    11,     3,     3,     3,     3,     5,     2,     2,     0,
       6,     5,     4,     3,     1,     1,     3,     4,     1,     1,
       1,     1,     4,     1,     3,     2,     0,     2,     0,     1,
       3,     1,     1,     1,     1,     3,     4,     4,     4,     1,
       1,     2,     2,     2,     3,     3,     1,     1,     1,     1,
       3,     1,     1,     1,     0,     1,     2,     1,     1,     1,
       1,     1,     1,     3,     5,     1,     3,     5,     4,     3,
       3,     3,     3,     1,     1,     0,     1,     1,     1,     6,
       3,     4,     6,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     5,
       4,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     1,     3,     2,     0,    11,     0,    12,     1,
       1,     1,     4,     4,     4,     4,     4,     1,     1,     3,
       5,     0,     3,     4,     1,     2,     4,     2,     6,     0,
       1,     4,     0,     2,     0,     1,     1,     3,     1,     3,
       1,     1,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     2,     1,     1,     2,
       2,     4,     1,     1,     3,     3,     1,     1,     1,     1,
       3,     3,     3,     2,     0,     1,     0,     1,     0,     5,
       3,     3,     1,     1,     1,     1,     3,     2,     1,     1,
       1,     1,     1,     2,     2,     4,     2,     0,     5,     3,
       3,     1,     3,     1,     2,     0,     3,     0,     4,     2,
       0,     3,     3,     1,     0,     1,     2,     2,     4,     3,
       3,     2,     4,     2,     4,     1,     1,     1,     1,     1,
       2,     4,     3,     4,     3,     1,     1,     1,     1,     2,
       4,     4,     3,     1,     1,     3,     7,     6,     8,     9,
       8,    10,     7,     6,     1,     2,     4,     4,     1,     1,
       4,     1,     0,     1,     2,     1,     1,     2,     4,     3,
       3,     0,     1,     2,     4,     3,     2,     3,     6,     0,
       1,     4,     2,     0,     5,     3,     3,     1,     6,     4,
       4,     2,     2,     0,     5,     3,     3,     1,     2,     0,
       5,     3,     3,     1,     2,     2,     1,     2,     1,     4,
       3,     3,     6,     3,     1,     1,     1,     4,     4,     2,
       2,     4,     2,     2,     1,     3,     3,     1,     2,     1,
       4,     3,     0,     1,     3,     3,     1,     1,     0,     0,
       2,     3,     1,     5,     3,     2,     2,     2,     1,     4,
       6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     2,     1,     0,     0,     0,     0,     0,     0,
     530,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   591,   458,     0,   464,
     465,    17,   487,   579,    71,   466,     0,    50,     0,     0,
       0,     0,     0,     0,     0,     0,    89,     0,     0,     0,
       0,     0,     0,     0,     0,   451,     0,     0,     0,     0,
     110,     0,     0,     0,   470,   472,   473,   467,   468,     0,
       0,   474,   469,     0,     0,   449,    18,    19,    20,    22,
      21,     0,   471,     0,     0,    70,    40,   583,   459,     0,
       0,     3,    29,    31,    34,   486,     0,   448,     0,     5,
      88,     6,     7,     8,     0,   268,     0,   266,   332,   341,
     340,     0,   339,   546,   450,     0,   489,   331,     0,   549,
     267,     0,     0,   547,   548,   545,   574,   578,     0,   321,
     488,   451,     0,     0,    29,    88,   643,   267,   642,     0,
     640,   639,   334,     0,     0,   305,   306,   307,   308,   330,
     328,   327,   326,   325,   324,   323,   322,   451,     0,   652,
     450,     0,   288,   286,   273,   454,     0,   652,   453,     0,
     463,   586,   585,   455,     0,     0,   457,   329,     0,     0,
       0,   263,     0,    48,   265,     0,     0,    54,    56,     0,
       0,    58,     0,     0,     0,   668,   671,     0,   652,     0,
       0,    60,     0,    40,     0,     0,     0,    24,    25,   174,
       0,     0,   173,   112,   111,   179,    88,     0,     0,     0,
       0,     0,   649,    98,   108,   599,   603,   628,     0,   476,
       0,     0,     0,   626,     0,    13,     0,    32,     0,     0,
     102,   109,   362,   268,     0,   266,   267,     0,     0,   460,
       0,   461,     0,     0,     0,    80,     0,     0,    36,   167,
       0,    16,    87,     0,   107,    94,   106,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   591,    79,   582,   582,   613,     0,     0,     0,
      88,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   287,   285,     0,   550,   535,   582,
       0,   541,   167,   582,     0,   584,   575,   599,     0,     0,
       0,   532,   527,   496,     0,     0,     0,     0,     0,     0,
      36,     0,   167,   259,     0,   587,   535,   543,   456,     0,
      40,   147,     0,    68,     0,     0,   264,     0,     0,     0,
       0,     0,    57,    78,    59,   652,   665,   666,     0,     0,
       0,   653,   667,     0,   648,    61,     0,    77,    26,     0,
      15,     0,     0,   175,     0,    66,     0,     0,     0,    67,
     644,     0,     0,     0,     0,     0,   118,     0,   600,     0,
       0,   607,     0,   496,     0,     0,   475,   627,   487,     0,
       0,   625,   492,   624,    33,     4,    10,    11,    62,    63,
       0,     0,     0,   260,   318,   554,    45,    39,    41,    42,
      43,    44,     0,   333,   490,   491,    30,     0,     0,     0,
     498,   168,     0,   335,    90,   114,   291,   293,   292,     0,
       0,   289,   290,   294,   296,   295,   310,   309,   312,   311,
     313,   315,   316,   314,   304,   303,   298,   299,   297,   300,
     301,   302,   317,   581,     0,     0,   617,     0,   496,   646,
     552,   574,   100,   104,     0,    96,     0,     0,   270,   284,
     283,   282,   281,   280,   279,   278,   277,   276,   275,   274,
       0,   537,   536,     0,     0,     0,     0,     0,     0,   641,
     525,   529,   495,   531,     0,     0,   652,     0,   590,   589,
       0,     0,   537,   536,   261,   149,   151,   262,     0,    40,
     131,    49,   265,     0,     0,     0,     0,   143,   143,    55,
       0,   658,     0,     0,     0,     0,     0,   449,    34,   478,
     448,   483,     0,   477,    38,   482,    83,     0,    23,    27,
       0,   172,   180,   337,   177,     0,     0,   637,   638,     9,
     662,     0,     0,     0,   599,   596,     0,   611,     0,   342,
     495,   602,   636,   635,   634,     0,   630,     0,   631,   633,
       0,     4,   182,   356,   357,   365,   364,     0,     0,   551,
     535,   542,   580,     0,   651,   169,   447,   497,   166,     0,
     534,     0,     0,   116,   320,     0,   345,   346,     0,   343,
     495,   612,     0,   167,   118,     0,    92,   114,   591,   271,
       0,     0,     0,   167,   539,   540,   553,   576,   577,     0,
       0,     0,   503,   504,   505,     0,     0,   512,   511,   523,
     496,     0,   527,   588,   535,   544,   462,     0,   153,     0,
       0,    46,     0,     0,     0,     0,   124,   125,   135,     0,
      40,   133,    74,   143,     0,   143,     0,     0,   669,   657,
     656,     0,   654,   479,   480,   494,     0,     0,     0,   619,
       0,    76,     0,    28,   176,   534,     0,   645,    69,     0,
       0,   650,   117,   119,   182,     0,     0,   597,     0,     0,
     606,     0,   605,   629,     0,    14,     0,   244,     0,     0,
       0,   537,   536,   654,     0,   170,    37,   156,     0,   498,
     533,   675,   534,   113,     0,     0,   319,   616,   615,   167,
       0,     0,   182,     0,   116,   463,    64,   538,   167,     0,
       0,   508,   509,   510,   513,   514,   517,     0,   507,   495,
     524,   526,   528,   538,     0,     0,     0,     0,   150,    51,
       0,   265,   126,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   137,     0,     0,     0,   673,   502,     0,   496,
     485,   484,   623,     0,   496,     0,     0,     0,   178,   661,
     664,     0,   244,   601,   599,     0,   269,   610,   609,     0,
       0,    12,     0,     0,   247,   248,   249,   252,   251,   250,
     242,     0,     0,     0,   103,   181,   183,     0,   241,   245,
       0,   244,   368,     0,     0,   370,   363,   366,     0,   361,
       0,     0,   167,   171,   659,   534,   155,   674,     0,     0,
     115,   182,     0,     0,   573,   182,   244,   534,     0,   272,
     167,     0,   567,   521,     0,   496,   506,   522,     0,    40,
       0,   146,   132,     0,   123,    72,   136,     0,     0,   139,
       0,   144,   145,    40,   138,   670,   655,     0,     0,   481,
     495,   493,     0,   344,   495,   618,     0,    40,   659,     0,
     120,    99,     0,     0,     0,   604,   632,     0,     0,   121,
     211,   209,   449,    22,     0,   205,     0,   210,   221,     0,
     219,   224,     0,   223,     0,   222,     0,    88,   246,   185,
       0,   187,     0,   243,   359,     0,     0,   538,   167,     0,
       0,   351,   154,   675,     0,   158,   659,   244,   614,   572,
     244,   105,     0,   182,     0,   566,     0,   515,   495,   516,
      40,   152,    47,    52,     0,   134,   140,    40,   142,     0,
     501,   500,   622,   621,     0,     0,   351,   663,   598,    65,
     608,     0,     0,   195,   199,     0,     0,   192,   419,   418,
     415,   417,   416,   435,   437,   436,   407,   397,   413,   412,
     375,   384,   385,   387,   386,   406,   390,   388,   389,   391,
     392,   393,   394,   395,   396,   398,   399,   400,   401,   402,
     403,   405,   404,   376,   377,   378,   380,   381,   383,   421,
     422,   431,   430,   429,   428,   427,   426,   414,   432,   423,
     424,   425,   408,   409,   410,   411,   433,   434,   438,   440,
     439,   441,   442,   420,   444,   443,   379,   445,   446,   382,
     374,   216,   371,     0,   193,   237,   238,   236,   229,     0,
     230,   194,   255,     0,     0,     0,     0,    88,   360,   358,
     369,   367,   167,     0,   570,   660,     0,     0,     0,   159,
       0,     0,    95,   101,   659,   244,   568,   520,   519,   148,
       0,    40,   129,    73,   141,   672,     0,     0,     0,    84,
       0,   258,   122,     0,     0,   213,   206,     0,     0,     0,
     218,   220,     0,     0,   225,   232,   233,   231,     0,     0,
     184,     0,     0,     0,     0,     0,   569,     0,    40,     0,
     162,     0,   161,    40,     0,    97,     0,    40,   127,    53,
       0,   499,   620,    40,    40,   196,    29,     0,   197,   198,
       0,     0,   212,   215,   372,   373,     0,   207,   234,   235,
     227,   228,   226,   256,   253,   188,   186,   257,     0,   571,
       0,   354,   498,     0,   163,     0,   160,     0,    40,   518,
       0,     0,     0,     0,     0,   244,   214,   217,     0,   534,
     190,   355,   497,     0,   336,     0,   165,    91,     0,     0,
     130,    82,   338,   203,     0,   243,   254,     0,   534,     0,
     352,   350,   164,    93,   128,    86,     0,     0,   202,   659,
       0,   353,     0,    85,    75,     0,   201,     0,   659,     0,
     200,   239,    40,   189,     0,     0,     0,   191,     0,   240,
       0,    40,     0,    81
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    91,   591,   415,   134,   206,   207,    93,
      94,    95,    96,    97,    98,   247,   427,   428,   355,   182,
    1100,   361,   964,  1225,   691,   692,  1234,   263,   135,   429,
     611,   743,   430,   445,   627,   396,   624,   431,   420,   625,
     265,   223,   240,   104,   613,   735,   573,   702,   908,   774,
     666,  1149,  1103,   531,   672,   360,   539,   674,   883,   526,
     658,   661,   766,   728,   729,   439,   440,   211,   212,   217,
     717,   825,   926,  1075,  1199,  1218,  1113,  1158,  1159,  1160,
     914,   915,   916,  1114,  1120,  1167,   919,   920,   924,  1068,
    1069,  1070,  1243,   826,   827,   828,   829,  1073,   830,   105,
     176,   356,   357,   106,   107,   610,   695,   108,   109,   551,
     110,   111,  1087,  1182,   112,   421,  1079,   422,   718,   597,
     839,   836,  1061,  1062,   113,   114,   115,   170,   177,   250,
     343,   116,   554,   555,   117,   788,   513,   608,   789,   648,
     754,   649,   864,   865,   650,   651,   511,   333,   143,   144,
     118,   731,   317,   318,   601,   119,   171,   137,   121,   122,
     123,   124,   125,   126,   127,   474,   128,   173,   174,   399,
     402,   403,   477,   478,   793,   794,   232,   233,   585,   129,
     391,   130,   199,   224,   258,   370,   681,   941,   571,   200,
     848
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -870
static const yytype_int16 yypact[] =
{
    -870,   120,  2913,  -870,  8593,  8593,   -60,  8593,  8593,  8593,
    -870,  8593,  8593,  8593,  8593,  8593,  8593,  8593,  8593,  8593,
    8593,  8593,  8593,  2261,  2261,  8593,  6575,   -17,     0,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  8593,  -870,     0,   110,
     119,   158,     0,  2383,   351,  6539,  -870,   466,  6697,   -25,
    8593,   798,    41,   209,   223,    43,   192,   201,   225,   258,
    -870,   351,   261,   265,  -870,  -870,  -870,  -870,  -870,   382,
     676,  -870,  -870,   351,  6855,  -870,  -870,  -870,  -870,  -870,
    -870,   351,  -870,    10,  8593,  -870,  -870,   296,   338,   405,
     405,  -870,   322,   323,   283,  -870,   335,  -870,    35,  -870,
     458,  -870,  -870,  -870,   685,  -870,  9378,  -870,   409,  -870,
     427,   433,  -870,    50,   337,   369,  -870,  -870,   860,   -12,
    5608,   104,   345,   113,   115,   350,    24,  -870,   248,  -870,
     465,   429,   365,   388,  -870,   458, 10345,  5766, 10345,  8593,
   10345, 10345,  2899,   498,   351,  -870,  -870,   496,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  1799,   396,
    -870,   425,   447,   447,  -870,   442,  1799,   396,   443,   445,
     408,   135,  -870,   470,   104,  7013,  -870,  -870,  8593,  5591,
      52, 10345,  6381,  -870,  8593,  8593,   351,  -870,  -870,  9419,
     418,  -870,  9479,   466,   466,   436,  -870,   556,    26,   587,
     351,  -870,  9520,  -870,  9561,   351,    56,  -870,    15,  -870,
    2037,    58,  -870,  -870,  -870,   591,   458,    63,  2261,  2261,
    2261,   444,   451,  -870,  -870,  6259,  7171,    40,    -4,  -870,
    8751,  2261,   434,  -870,   351,  -870,   220,   323,   448,  9621,
    -870,  -870,  -870,   454, 10345,   463,   420,  3071,  8593,   205,
     452,   502,   205,   274,   234,  -870,   351,   466,   467,  7329,
     466,  -870,  -870,  1052,  -870,  -870,  -870,  8593,  8593,  8593,
    7487,  8593,  8593,  8593,  8593,  8593,  8593,  8593,  8593,  8593,
    8593,  8593,  8593,  8593,  8593,  8593,  8593,  8593,  8593,  8593,
    8593,  8593,  6575,  -870,  8593,  8593,  8593,   579,   351,   351,
     458,   685,  2170,  8593,  8593,  8593,  8593,  8593,  8593,  8593,
    8593,  8593,  8593,  8593,  -870,  -870,   344,  -870,   140,  8593,
    8593,  -870,  7329,  8593,  8593,   296,   213,  6259,   469,  7645,
    9763,  -870,   471,   637,  1799,   480,   -49,   579,   482,   -24,
    -870,   249,  7329,  -870,   398,  -870,   227,  -870,  -870,  9804,
    -870,  -870,  8593,  -870,   569,  5749,   647,   489, 10238,   649,
      30,     6,  -870,  -870,  -870,   396,  -870,  -870,   466,   494,
     662,  -870,  -870,  9983,  -870,  -870,  3244,  -870,    93,   798,
    -870,   351,  8593,   447,    41,  -870,  9983,   505,   601,  -870,
     447,    38,    55,   179,   508,   351,   559,   513,   447,    60,
    2261,  9845,   517,   679,  1161,   351,  -870,  -870,   631,  1471,
     -22,  -870,  -870,  -870,   323,  -870,  -870,  -870,  -870,  -870,
     525,   535,   -10,   182,   639,   -11,  -870,  -870,  -870,  -870,
    -870,  -870,  8903,  -870,  -870,  -870,  -870,   102,  2261,   534,
     695, 10345,   693,  -870,  -870,   592, 10385,  2729,  2899,  8593,
   10304,  3229,  3401,  3563,  3720,  3887,  4050,  4050,  4050,  4050,
    2310,  2310,  2310,  2310,  1398,  1398,   507,   507,   507,   496,
     496,   496,  -870, 10345,   537,   539, 10046,   543,   708,   171,
     551,   213,  -870,  -870,   351,  -870,  1938,  8593,  2899,  2899,
    2899,  2899,  2899,  2899,  2899,  2899,  2899,  2899,  2899,  2899,
    8593,   171,   552,   546,  8944,   571,   563,  8985,    85,  -870,
    1335,  -870,   351,  -870,   454,   182,   396,   206,   229,  -870,
     573,  8593,  -870,  -870,  -870,  5433,   304, 10345,     0,  -870,
    -870,  -870,  8593,  1448,  9983,   351,  5907,   574,   576,  -870,
     107,   752,   466,  9983,  9983,   580,     8,   605,   289,  -870,
     611,  -870,   584,  -870,  -870,  -870,   653,   351,  -870,  -870,
    9047,  -870,  -870,  -870,   746,  2261,   599,  -870,  -870,  -870,
      62,   597,  1005,   606,  6259,  6417,   757,   447,  7803,  -870,
    7961,  -870,  -870,  -870,  -870,   603,  -870,  8593,  -870,  -870,
    2567,  -870,  -870,  -870,  -870,  -870,  -870,   767,   457,  -870,
     231,  -870,  -870,   466,  -870,   447,  -870,  8119,  -870,  9983,
       3,   615,  1005,   668,  3062,  8593,  -870,  -870,  8593,  -870,
    8593,  -870,   624,  7329,   559,   622,  -870,   592,  6575,   447,
    9662,  9088,   626,  7329,  -870,  -870,   232,  -870,  -870,   780,
     620,   620,  -870,  -870,  -870,   629,    16,  -870,  -870,  -870,
     788,   630,   471,  -870,   245,  -870,  -870,  9129,   328,     0,
    6381,  -870,   636,  3417,   638,  2261,   689,   447,  -870,   785,
    -870,  -870,  -870,  -870,   485,  -870,   217,   466,  -870,  -870,
     807,   650,   651,  -870,  -870,  9983,   698,   351,   351,  9983,
     673,  -870,   657,  -870,  -870,     3,  9983,   447,  -870,   351,
     351,  -870,   832,  -870,  -870,    96,   677,   447,  8277,  2261,
   10345,  2261, 10142,  -870,  1228,  -870,  2740,    20,   299,   -30,
    8593,   171,   678,  -870,  2261, 10345,  -870,  -870,   675,   848,
    -870,   466,     3,  -870,  1005,   692,  3062, 10345, 10183,  7329,
     705,   704,  -870,   696,   668,   408,  -870,   712,  7329,   713,
    8593,  -870,  -870,  -870,  -870,  -870,  1335,   745,  -870,  1335,
    -870,  -870,  -870,  -870,     0,   857,   816,  6381,  -870,  -870,
     719,  8593,   447,  1448,   721,  9983,  3575,   550,   724,  8593,
      39,   237,  -870,   726,   897,   868,  -870,   773,   729,   891,
    -870,  -870,   784,   737,   900,  1005,   741,   744,  -870,  -870,
     905,  1005,   742,  -870,  6259,  8593,  2899,   447,   447,  8435,
     747,  -870,   466,  1005,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  1565,   765,   715,  -870,  -870,  -870,   429,  1312,  -870,
      66,   998,  -870,    27,  8593,  -870,  -870,  -870,  8593,  -870,
    9191,   750,  7329,   447,   895,   105,  -870,  -870,    65,   756,
     832,  -870,  8593,   758,  -870,  -870,   916,     3,   755,  -870,
    7329,   759,  -870,   808,   762,   923,  -870,  -870,   907,  -870,
     766,  -870,  -870,   768,  -870,  -870,  -870,   769,   771,  -870,
    9337,  -870,  -870,  -870,  -870,  -870,  -870,   466,  9983,  -870,
    9983,  -870,  9983,  -870,  9983,  -870,   863,  -870,   895,   351,
    -870,  -870,    99,  9703,  2261, 10345,  -870,   927,    49,  -870,
    -870,  -870,    72,   774,    73,  -870,  9892,  -870,  -870,    74,
    -870,  -870,   976,  -870,   777,  -870,   879,   458,  -870,  -870,
     466,  -870,   429,   998,   809,  9232,  9273,   794,  7329,   800,
     466,   870,  -870,   466,   888,   959,   895,  1522, 10345,  -870,
    1576,  -870,   815,  -870,   817,  -870,  1335,  -870,  1335,  -870,
    -870,  5433,  -870,  -870,  6065,  -870,  -870,  -870,  5433,   818,
    -870,   853,  -870,   854,   820,  3733,   870,  -870,  -870,  -870,
     447,  9983,  1005,  -870,  -870,  1397,  1565,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,   306,  -870,   765,  -870,  -870,  -870,  -870,  -870,    84,
     314,  -870,   970,    76,   351,   879,   975,   458,  -870,  -870,
    -870,  -870,  7329,   822,  -870,  -870,   825,   828,    67,   981,
    9983,   829,  -870,  -870,   895,  1734,  -870,  -870,   881,  5433,
    6223,  -870,  -870,  -870,  5433,  -870,  9983,  9983,   835,  -870,
     840,  -870,  -870,  1286,    29,  -870,  -870,  9983,  9892,  9892,
     950,  -870,   976,   976,   439,  -870,  -870,  -870,  9983,   937,
    -870,   845,    81,  9983,   351,   846,  -870,   106,  -870,   939,
    1003,  9983,  -870,  -870,   850,  -870,  1335,  -870,  -870,  -870,
    3906,  -870,  -870,  -870,  -870,  -870,   940,   889,  -870,  -870,
     941,  1397,  -870,  -870,  -870,  -870,   880,  -870,  1007,  -870,
    -870,  -870,  -870,  -870,  1019,  -870,  -870,  -870,   872,  -870,
     966,  -870,  1033,  4064,  1029,  9983,  -870,  4237,  -870,  -870,
    4410,   876,  4568,  4741,   351,   998,  -870,  -870,  9983,     3,
    -870,  -870,   305,   884,  -870,  9983,  -870,  -870,  4914,   887,
    -870,  -870,  -870,   902,   351,   558,  -870,   885,     3,   985,
    -870,  -870,  -870,  -870,  -870,   166,  1005,   892,  -870,   895,
     894,  -870,   903,  -870,  -870,    82,  -870,   294,   895,  1005,
    -870,  -870,  -870,  -870,   294,   995,  5087,  -870,   904,  -870,
     901,  -870,  5260,  -870
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -870,  -870,  -382,  -870,  -870,  -870,    -2,  -870,   700,     5,
     761,  1275,  -870,   474,  -870,  -148,  -870,    -1,  -870,  -870,
    -870,  -870,  -870,  -870,  -150,  -870,  -870,  -133,    32,     4,
    -870,  -870,     7,  -870,  -870,  -870,  -870,     9,  -870,  -870,
     776,   783,   775,   967,   460,   346,   468,   357,  -130,  -870,
     327,  -870,  -870,  -870,  -870,  -870,  -870,  -462,   221,  -870,
    -870,  -870,  -870,  -670,  -870,  -318,  -870,  -870,   734,  -870,
    -651,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,  -870,
    -870,   133,  -870,  -870,  -870,  -870,  -870,    70,  -870,   307,
    -650,  -870,  -109,  -870,  -819,  -809,  -810,    64,  -870,   -63,
     -23,  1101,  -502,  1751,  1054,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,   164,  -870,   423,  -870,  -870,  -870,  -870,  -870,
    -870,  -870,  -870,  -633,  -870,  1011,    28,  -265,  -870,  -870,
     400,   190,  -328,  -870,  -870,  -870,  -390,  -721,  -870,  -870,
     509,  -728,  -870,  -870,  -870,  -870,   491,  -870,  -870,  -870,
    -570,   310,  -132,  -125,  -107,  -870,  -870,    98,  -870,  -870,
    -870,  -870,    -3,  -108,  -870,   131,  -870,  -870,  -870,  -317,
    -870,  -870,  -870,  -870,  -870,  -870,   593,   861,  -870,  -870,
     933,  -870,  -253,   -78,  -151,  -240,  -870,  -869,  -664,   -85,
     219
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -653
static const yytype_int16 yytable[] =
{
      92,    99,   328,   241,   505,   179,   101,   442,   846,   102,
     508,   103,   932,   581,   321,   183,   340,   437,   928,   187,
     326,   243,   933,   172,   520,   797,   266,   472,   863,   976,
     664,   867,   537,   590,   100,   799,   594,  1161,   535,  -647,
     730,   345,   190,   260,    10,   198,   565,   372,   346,  -558,
    -562,   161,   161,   802,   169,   376,   208,   982,   562,   222,
     352,    10,   849,   565,   379,   881,   384,   347,   575,   227,
     699,   388,   316,   934,   930,   236,   676,  1091,   237,   222,
    -208,   986,  1063,   387,  1129,   323,   686,   216,   621,  1129,
     982,   856,  1122,   575,   757,   335,   944,   316,  1139,   316,
     120,   404,   222,   338,   575,   381,   139,   575,   366,   367,
     603,  -555,   371,  1123,   209,   677,   215,   837,   425,   812,
       3,   162,   163,   928,   813,   730,   814,   815,   816,   817,
     818,   819,   820,   406,   230,   231,   595,  1180,   945,   838,
    1140,    46,   332,   518,   203,   229,    10,   831,   589,   175,
     300,   596,   700,   758,  -564,  -565,   242,  -558,  -562,   256,
     727,   405,   730,   216,  -452,   319,   178,   484,   821,   822,
    -157,   823,   371,   242,  -556,   538,  -557,    92,   351,  1181,
      92,   354,   246,   557,   359,   444,   161,   952,   257,   481,
     824,   365,   365,   324,   161,   365,  -592,   536,   374,  1162,
     947,  -559,   525,   261,   950,   566,   668,   882,   481,   716,
     378,   777,   210,   781,   372,   683,   684,   983,   984,  -555,
     353,   241,   567,   266,   380,  1144,   385,   576,  1097,   481,
    1098,   389,   831,   481,   931,   977,   481,   256,   161,   414,
    -208,   987,  1064,   319,  1130,    92,   161,   161,   161,  1176,
    1240,  1124,   639,   161,   436,   365,   336,   705,   198,   161,
     760,   222,   942,   803,   339,   604,   978,   319,  1232,   873,
     678,   514,  -497,   320,   323,   730,   184,   120,   227,   100,
     120,   726,  -556,   540,  -557,   185,   831,   730,  -593,   172,
    -595,   599,  -560,  -561,   480,   479,   222,   222,   600,   222,
     316,   680,  1095,   598,  -592,   740,  -594,   227,   383,  -559,
     779,   780,   411,   502,   501,   749,   390,   390,   393,  1117,
     169,    33,    33,   398,   186,   -83,   213,   344,   622,   410,
     779,   780,  1118,   257,   480,   516,  1219,  -652,   519,  1125,
     214,   523,   522,   230,   231,   120,   568,   227,   255,  1119,
     632,   320,   411,    92,   530,   161,  1126,   787,   218,  1127,
    1237,   792,   161,   745,   256,   622,   365,   219,   798,  1244,
     659,   660,   230,   231,    92,   320,  1214,   831,  1220,   559,
     831,   663,   324,   599,   208,   653,  1215,   782,   416,   417,
     600,   220,   654,   570,   764,   765,  -593,   340,  -595,   891,
    -560,  -561,   584,   586,   895,   928,   626,   884,   100,   435,
     655,   227,   230,   231,  -594,    31,   249,    33,  1189,   325,
      87,   853,    31,  -652,   221,   398,   475,   225,   161,  -652,
     861,   226,   336,   329,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   257,   832,   876,   434,  -652,
     503,   257,  -652,   120,   506,   227,   371,   682,  -652,   833,
     228,  1203,  1241,  1242,  1170,   248,   161,   256,   834,    31,
    -347,    33,  1168,  1169,   120,   959,   230,   231,   227,   314,
     315,  1171,   222,   252,  1172,  1164,  1165,   902,   294,   262,
     481,   193,    76,    77,   295,    78,    79,    80,   577,    76,
      77,   259,    78,    79,    80,   662,   296,   227,   647,   297,
     652,   322,   411,   500,   161,    87,  -563,   194,   723,   229,
     230,   231,   776,    92,   939,   831,  -348,    46,    31,  1217,
      33,   327,   234,   669,    92,   671,   605,    31,   331,   365,
     365,   316,   954,   230,   231,   292,    76,    77,  1230,    78,
      79,    80,   289,   290,   291,   693,   292,   100,   257,   907,
     970,   161,   971,   553,   972,   337,   973,   521,   316,    87,
     841,   412,   230,   231,   342,   227,   553,   778,   779,   780,
     411,   193,  -451,  -450,   629,   341,   363,   425,    92,    99,
     195,   344,   783,   161,   101,   722,   721,   102,   368,   103,
     373,   365,   161,   161,   386,    76,    77,   194,    78,    79,
      80,   394,   196,   395,    76,    77,   418,    78,    79,    80,
    1083,   423,   100,   120,   433,   172,   720,    31,    87,   730,
     424,   667,   197,   -35,   120,   443,   767,   510,   753,   753,
     230,   231,   878,   779,   780,   512,   847,   515,   730,   517,
      31,   528,    33,  1111,   369,   352,   169,   532,    92,   768,
     541,    92,   534,   697,   814,   815,   816,   817,   818,   819,
     542,   563,   398,   707,   564,   365,   569,  1076,   572,   574,
     195,   251,   253,   254,   579,   790,   791,   580,   120,   751,
     752,    31,   587,   161,   592,   100,   593,   570,   800,   723,
    -349,   606,   196,   607,    76,    77,   609,    78,    79,    80,
     616,   612,   617,   619,    92,    99,   620,   623,   633,   634,
     101,   961,   197,   102,   553,   103,  1228,    76,    77,   365,
      78,    79,    80,   553,   553,   968,   637,   161,   636,   161,
     656,   868,   673,   929,   675,   687,   685,    31,   100,   975,
      87,   688,   161,   689,   647,   690,    31,   647,   120,   696,
     701,   120,  1142,   772,  1135,    92,   871,   698,    76,    77,
     708,    78,    79,    80,    92,   704,   713,   193,  1151,  1152,
     719,   732,   365,    10,   159,   159,    31,   167,   734,  1163,
     739,   742,   748,   750,  1074,   756,   759,   761,   775,   553,
    1173,   161,   969,   194,   769,  1177,   771,   807,   100,   808,
     198,   773,  1099,  1186,   120,   784,   796,   785,   786,  1104,
     234,   923,   843,    31,    76,    77,   365,    78,    79,    80,
     921,   264,   161,    76,    77,   406,    78,    79,    80,   795,
     801,   812,   844,   804,   842,   235,   813,   550,   814,   815,
     816,   817,   818,   819,   820,  1085,   845,  1206,   847,   927,
     550,   851,   857,    76,    77,   120,    78,    79,    80,    31,
    1216,   667,   854,   855,   120,   553,   195,  1222,   860,   553,
     862,   922,   866,   869,   870,   365,   553,   872,   875,   885,
     821,   822,   879,   823,   887,   888,   889,   570,   196,   890,
      76,    77,   398,    78,    79,    80,   892,   893,   894,   679,
     897,   898,   901,   899,  1060,   918,   938,   906,   197,   159,
    1067,   940,   193,   946,   953,   949,   955,   159,   198,   957,
     956,   958,   161,   960,   962,   963,   974,   965,   365,   966,
     981,   365,   205,   985,  1134,  1071,    76,    77,   194,    78,
      79,    80,  1072,  1150,   647,  1078,   647,    10,    46,    92,
    1082,  1089,    92,  1102,  1077,   553,    92,  1084,    31,    53,
      54,   159,  1090,    92,  1086,  1106,  1107,    60,   298,   159,
     159,   159,  1094,  1128,  1096,  1105,   159,  1108,  1133,  1136,
    1183,  1137,   159,   100,  1141,  1187,  1131,  1138,  1143,  1190,
     100,  1166,   980,  1146,  1153,  1192,  1193,   100,   550,  1154,
    1174,  1175,  1184,  1179,   299,   812,  1185,   550,   550,  1188,
     813,   195,   814,   815,   816,   817,   818,   819,   820,  1194,
    -204,  1195,  1198,  1197,   160,   160,  1123,   168,  1200,  1201,
    1208,  1202,  1205,   196,  1210,    76,    77,    31,    78,    79,
      80,  1221,  1229,   167,   886,  1224,  1178,  1226,  1231,   120,
    1236,  1238,   120,   197,   821,   822,   120,   823,  1248,  1239,
    1251,  1250,   222,   120,   483,  1233,    31,   485,   553,   558,
     553,   482,   553,   550,   553,   301,   951,   744,   159,   407,
     858,   850,   741,   413,  1246,   159,  1235,    92,    92,  1148,
     874,   967,    92,  1252,   814,   815,   816,   817,   818,   819,
     407,  1156,   413,   407,   413,   413,  1060,  1060,   561,  1116,
    1067,  1067,  1065,    31,    76,    77,  1066,    78,    79,    80,
     925,   100,   222,  1121,   548,  1247,   100,   180,   245,  1132,
    1110,   835,   922,   762,   647,   859,   133,   548,    92,    73,
     755,    75,   392,    76,    77,   943,    78,    79,    80,   550,
       0,   159,  1088,   550,     0,     0,     0,     0,     0,   160,
     550,   553,     0,     0,     0,  1115,     0,   160,     0,     0,
       0,    92,   100,     0,     0,    92,     0,     0,    92,     0,
      92,    92,  1213,     0,     0,     0,     0,   120,   120,   159,
      76,    77,   120,    78,    79,    80,    92,     0,     0,     0,
       0,     0,  1227,     0,     0,   100,     0,     0,   443,   100,
       0,   160,   100,     0,   100,   100,     0,     0,     0,   160,
     160,   160,    31,     0,   582,   583,   160,   267,   268,   269,
     100,     0,   160,     0,    92,     0,     0,   159,   120,   550,
      92,     0,     0,   270,     0,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,     0,   292,   100,     0,
     553,   120,     0,     0,   100,   120,     0,     0,   120,     0,
     120,   120,     0,     0,   159,   548,   553,   553,     0,     0,
       0,     0,     0,   168,   548,   548,   120,   553,     0,    76,
      77,     0,    78,    79,    80,     0,     0,     0,   553,     0,
       0,     0,     0,   553,     0,     0,   159,     0,     0,     0,
       0,   553,     0,   167,     0,   159,   159,   193,   160,     0,
       0,     0,     0,     0,   120,   160,     0,     0,     0,     0,
     120,  1196,     0,     0,     0,     0,     0,    31,     0,     0,
       0,     0,   550,   194,   550,     0,   550,     0,   550,     0,
     548,     0,     0,   167,     0,   553,     0,   640,   641,     0,
       0,     0,     0,    31,   552,     0,     0,     0,   553,   167,
       0,     0,     0,     0,     0,   553,     0,   552,     0,     0,
       0,   810,     0,     0,   642,   643,    31,     0,     0,     0,
    -243,   160,     0,     0,   644,     0,     0,     0,   814,   815,
     816,   817,   818,   819,     0,     0,   159,   133,     0,     0,
      73,     0,     0,     0,    76,    77,   195,    78,    79,    80,
     286,   287,   288,   289,   290,   291,   548,   292,     0,   160,
     548,     0,     0,     0,     0,   550,  1155,   548,   196,   645,
      76,    77,     0,    78,    79,    80,    29,    30,     0,     0,
     159,   646,   159,     0,     0,     0,    35,     0,   197,   665,
     267,   268,   269,    76,    77,   159,    78,    79,    80,     0,
       0,     0,     0,     0,     0,   167,   270,   160,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,    31,
     292,    33,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,   546,   159,     0,   548,     0,     0,    71,
      72,     0,     0,     0,   160,   552,     0,     0,     0,     0,
       0,     0,     0,    82,   552,   552,   167,     0,     0,   157,
       0,     0,   167,    10,   550,   159,     0,     0,     0,     0,
       0,     0,     0,     0,   167,     0,   160,     0,     0,     0,
     550,   550,   167,   703,     0,   160,   160,  1157,     0,   133,
       0,   550,    73,     0,    75,     0,    76,    77,     0,    78,
      79,    80,   550,     0,     0,     0,     0,   550,     0,     0,
       0,     0,     0,     0,   158,   550,     0,    10,     0,    87,
     552,   812,     0,   733,     0,     0,   813,     0,   814,   815,
     816,   817,   818,   819,   820,     0,    31,     0,     0,   168,
       0,   588,     0,     0,     0,     0,     0,     0,   549,   548,
       0,   548,     0,   548,     0,   548,     0,     0,     0,   550,
       0,   549,     0,     0,     0,   159,     0,     0,     0,     0,
     821,   822,   550,   823,     0,   812,   160,   910,     0,   550,
     813,     0,   814,   815,   816,   817,   818,   819,   820,   911,
       0,     0,  1092,     0,     0,     0,   552,     0,     0,     0,
     552,     0,     0,     0,     0,     0,   133,   552,     0,    73,
       0,   912,     0,    76,    77,     0,    78,   913,    80,     0,
     160,     0,   160,     0,   821,   822,     0,   823,     0,     0,
       0,     0,     0,     0,     0,   160,     0,     0,     0,     0,
       0,     0,   548,   167,     0,   703,  1093,   167,     0,     0,
       0,     0,     0,     0,     0,   136,   138,     0,   140,   141,
     142,     0,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   154,   155,   156,     0,    10,   164,     0,     0,     0,
       0,     0,     0,     0,   160,     0,   552,   181,     0,     0,
       0,     0,     0,     0,   189,     0,   192,     0,     0,   202,
       0,   204,     0,     0,     0,     0,   896,     0,     0,   549,
       0,     0,   900,     0,     0,   160,     0,     0,   549,   549,
       0,     0,     0,     0,   909,   239,     0,     0,     0,     0,
       0,     0,   917,   812,     0,   244,     0,     0,   813,     0,
     814,   815,   816,   817,   818,   819,   820,     0,     0,     0,
       0,   548,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    26,     0,     0,     0,     0,   548,   548,     0,
      31,     0,    33,     0,   167,     0,     0,     0,   548,     0,
       0,     0,   821,   822,   549,   823,     0,     0,     0,   548,
     330,     0,     0,     0,   548,     0,     0,     0,     0,   552,
       0,   552,   548,   552,  1145,   552,     0,     0,     0,     0,
     157,     0,     0,     0,     0,   160,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   349,     0,     0,   349,
       0,     0,     0,     0,     0,   181,   358,     0,     0,     0,
     133,     0,     0,    73,     0,    75,   548,    76,    77,     0,
      78,    79,    80,     0,     0,     0,     0,     0,     0,   548,
     549,     0,     0,     0,   549,   334,   548,     0,     0,     0,
      87,   549,     0,     0,     0,     0,     0,   401,     0,     0,
       0,   409,     0,     0,     0,     0,     0,   167,     0,     0,
       0,     0,   552,  1112,     0,     0,     0,   917,     0,   432,
     167,   628,     0,     0,     0,     0,     0,     0,     0,    31,
     441,    33,     0,     0,     0,     0,     0,     0,   446,   447,
     448,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   467,   468,
     469,   470,   471,     0,     0,   473,   473,   476,     0,   157,
     549,     0,     0,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,     0,     0,     0,     0,     0,
     473,   504,     0,   441,   473,   507,     0,     0,     0,   133,
     488,     0,    73,     0,    75,     0,    76,    77,     0,    78,
      79,    80,     0,   441,     0,     0,     0,     0,     0,     0,
       0,   552,     0,   527,   158,     0,     0,     0,    31,    87,
      33,     0,     0,     0,     0,     0,     0,   552,   552,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   552,     0,
       0,     0,     0,   560,     0,     0,     0,     0,     0,   552,
       0,     0,     0,     0,   552,     0,     0,     0,   157,     0,
       0,     0,   552,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   549,     0,   549,     0,   549,     0,   549,
       0,     0,     0,     4,     5,     6,     7,     8,   133,     0,
       0,    73,     9,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,     0,     0,     0,   552,     0,     0,     0,
     614,   486,     0,   158,     0,     0,   382,     0,    87,   552,
       0,     0,    11,    12,     0,     0,   552,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,     0,     0,   909,   630,    29,
      30,    31,    32,    33,     0,     0,     0,     0,     0,    35,
    1245,   631,     0,     0,     0,     0,   549,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
       0,     0,   657,     0,     0,     0,     0,     0,     0,     0,
       0,   131,     0,   181,    57,    58,     0,     0,     0,     0,
       0,     0,     0,   132,    63,    64,    65,    66,    67,    68,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,   133,    71,    72,    73,   487,    75,     0,    76,    77,
       0,    78,    79,    80,     0,     0,    82,     0,     0,   710,
      83,   712,    31,     0,    33,     0,    84,     0,   714,     0,
       0,    87,    88,     0,    89,    90,  -653,  -653,  -653,  -653,
     284,   285,   286,   287,   288,   289,   290,   291,   725,   292,
       0,     0,     0,     0,     0,   549,   736,     0,     0,   737,
       0,   738,   157,     0,   441,     0,     0,     0,     0,     0,
       0,   549,   549,     0,   441,     0,     4,     5,     6,     7,
       8,     0,   549,     0,     0,     9,     0,     0,     0,     0,
       0,     0,   133,   549,     0,    73,     0,    75,   549,    76,
      77,     0,    78,    79,    80,     0,   549,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,   158,     0,     0,
       0,    13,    87,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,   806,
     549,     0,    35,     0,     0,     0,     0,     0,     0,     0,
       0,   840,     0,   549,     0,     0,     0,     0,     0,     0,
     549,    46,     0,     0,     0,     0,     0,     0,     0,     0,
     441,     0,     0,     0,   131,     0,     0,    57,    58,   441,
       0,   806,     0,     0,     0,     0,   132,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,    69,
       0,     0,   181,     0,   133,    71,    72,    73,     0,    75,
     880,    76,    77,     0,    78,    79,    80,     0,     0,    82,
       0,     0,     0,    83,     0,     0,     0,     0,     0,    84,
       0,   188,     0,     0,    87,    88,   903,    89,    90,     0,
     905,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
       0,     0,     0,     0,     0,   935,     0,     0,     0,   936,
       0,     0,     0,   441,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   948,     0,     0,     0,     0,    10,    11,
      12,   441,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,     0,     0,     0,    29,    30,    31,    32,
      33,     0,    34,     0,     0,     0,    35,    36,    37,    38,
       0,    39,     0,    40,     0,    41,     0,     0,    42,     0,
       0,     0,    43,    44,    45,    46,    47,    48,    49,     0,
      50,    51,    52,     0,     0,     0,    53,    54,    55,     0,
      56,    57,    58,    59,    60,    61,     0,     0,     0,   441,
      62,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,    70,    71,
      72,    73,    74,    75,     0,    76,    77,     0,    78,    79,
      80,    81,     0,    82,     0,     0,     0,    83,     0,     0,
       0,     0,     0,    84,     0,    85,    86,   715,    87,    88,
     269,    89,    90,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,     0,   270,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,     0,   292,     0,
       0,    10,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,     0,     0,     0,    29,
      30,    31,    32,    33,     0,    34,     0,     0,     0,    35,
      36,    37,    38,     0,    39,     0,    40,     0,    41,     0,
       0,    42,     0,   441,     0,    43,    44,    45,    46,    47,
      48,    49,     0,    50,    51,    52,     0,     0,     0,    53,
      54,    55,     0,    56,    57,    58,    59,    60,    61,     0,
       0,     0,     0,    62,    63,    64,    65,    66,    67,    68,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,    70,    71,    72,    73,    74,    75,     0,    76,    77,
       0,    78,    79,    80,    81,     0,    82,     0,     0,     0,
      83,     0,     0,     0,     0,     0,    84,     0,    85,    86,
     811,    87,    88,     0,    89,    90,     4,     5,     6,     7,
       8,     0,     0,     0,   270,     9,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,     0,   292,     0,
       0,     0,     0,     0,    10,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,     0,
       0,     0,    29,    30,    31,    32,    33,     0,    34,     0,
       0,     0,    35,    36,    37,    38,     0,    39,     0,    40,
       0,    41,     0,     0,    42,     0,     0,     0,    43,    44,
      45,    46,    47,    48,    49,     0,    50,    51,    52,     0,
       0,     0,    53,    54,    55,     0,    56,    57,    58,    59,
      60,    61,     0,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,    70,    71,    72,    73,    74,    75,
       0,    76,    77,     0,    78,    79,    80,    81,     0,    82,
       0,     0,     0,    83,     4,     5,     6,     7,     8,    84,
       0,    85,    86,     9,    87,    88,     0,    89,    90,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
       0,   292,    10,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,     0,     0,     0,
      29,    30,    31,    32,    33,     0,    34,     0,     0,     0,
      35,    36,    37,    38,     0,    39,     0,    40,     0,    41,
       0,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    48,    49,     0,    50,     0,    52,     0,     0,     0,
      53,    54,    55,     0,    56,    57,    58,     0,    60,    61,
       0,     0,     0,     0,    62,    63,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,   133,    71,    72,    73,    74,    75,     0,    76,
      77,     0,    78,    79,    80,    81,     0,    82,     0,     0,
       0,    83,     0,     0,     0,     0,     0,    84,     0,    85,
      86,   426,    87,    88,     0,    89,    90,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     9,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,     0,   292,     0,
       0,     0,     0,     0,     0,    10,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
       0,     0,     0,    29,    30,    31,    32,    33,     0,    34,
       0,     0,     0,    35,    36,    37,    38,     0,    39,     0,
      40,     0,    41,     0,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    48,    49,     0,    50,     0,    52,
       0,     0,     0,    53,    54,    55,     0,    56,    57,    58,
       0,    60,    61,     0,     0,     0,     0,    62,    63,    64,
      65,    66,    67,    68,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,   133,    71,    72,    73,    74,
      75,     0,    76,    77,     0,    78,    79,    80,    81,     0,
      82,     0,     0,     0,    83,     0,     0,     0,     0,     0,
      84,     0,    85,    86,   556,    87,    88,     0,    89,    90,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,     0,
     292,     0,     0,     0,     0,     0,     0,     0,    10,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,     0,     0,     0,    29,    30,    31,    32,
      33,     0,    34,     0,     0,     0,    35,    36,    37,    38,
     770,    39,     0,    40,     0,    41,     0,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    48,    49,     0,
      50,     0,    52,     0,     0,     0,    53,    54,    55,     0,
      56,    57,    58,     0,    60,    61,     0,     0,     0,     0,
      62,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,   133,    71,
      72,    73,    74,    75,     0,    76,    77,     0,    78,    79,
      80,    81,     0,    82,     0,     0,     0,    83,     4,     5,
       6,     7,     8,    84,     0,    85,    86,     9,    87,    88,
       0,    89,    90,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,     0,   292,     0,     0,     0,    10,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,     0,     0,     0,    29,    30,    31,    32,    33,     0,
      34,     0,     0,     0,    35,    36,    37,    38,     0,    39,
       0,    40,     0,    41,   877,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    48,    49,     0,    50,     0,
      52,     0,     0,     0,    53,    54,    55,     0,    56,    57,
      58,     0,    60,    61,     0,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,   133,    71,    72,    73,
      74,    75,     0,    76,    77,     0,    78,    79,    80,    81,
       0,    82,     0,     0,     0,    83,     4,     5,     6,     7,
       8,    84,     0,    85,    86,     9,    87,    88,     0,    89,
      90,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,     0,   292,
       0,     0,     0,     0,    10,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,     0,
       0,     0,    29,    30,    31,    32,    33,     0,    34,     0,
       0,     0,    35,    36,    37,    38,     0,    39,     0,    40,
       0,    41,     0,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    48,    49,     0,    50,     0,    52,     0,
       0,     0,    53,    54,    55,     0,    56,    57,    58,     0,
      60,    61,     0,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,   133,    71,    72,    73,    74,    75,
       0,    76,    77,     0,    78,    79,    80,    81,     0,    82,
       0,     0,     0,    83,     0,     0,     0,     0,     0,    84,
       0,    85,    86,  1109,    87,    88,     0,    89,    90,     4,
       5,     6,     7,     8,     0,     0,     0,     0,     9,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,     0,   292,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    10,    11,    12,
       0,     0,     0,     0,    13,     0,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,     0,    25,    26,
      27,    28,     0,     0,     0,    29,    30,    31,    32,    33,
       0,    34,     0,     0,     0,    35,    36,    37,    38,     0,
      39,     0,    40,  1191,    41,     0,     0,    42,     0,     0,
       0,    43,    44,    45,    46,     0,    48,    49,     0,    50,
       0,    52,     0,     0,     0,    53,    54,    55,     0,    56,
      57,    58,     0,    60,    61,     0,     0,     0,     0,    62,
      63,    64,    65,    66,    67,    68,     0,     0,     0,     0,
       0,     0,    69,     0,     0,     0,     0,   133,    71,    72,
      73,    74,    75,     0,    76,    77,     0,    78,    79,    80,
      81,     0,    82,     0,     0,     0,    83,     4,     5,     6,
       7,     8,    84,     0,    85,    86,     9,    87,    88,     0,
      89,    90,  -653,  -653,  -653,  -653,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,     0,   292,
       0,     0,     0,     0,     0,    10,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
       0,     0,     0,    29,    30,    31,    32,    33,     0,    34,
       0,     0,     0,    35,    36,    37,    38,     0,    39,     0,
      40,     0,    41,     0,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    48,    49,     0,    50,     0,    52,
       0,     0,     0,    53,    54,    55,     0,    56,    57,    58,
       0,    60,    61,     0,     0,     0,     0,    62,    63,    64,
      65,    66,    67,    68,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,   133,    71,    72,    73,    74,
      75,     0,    76,    77,     0,    78,    79,    80,    81,     0,
      82,     0,     0,     0,    83,     0,     0,     0,     0,     0,
      84,     0,    85,    86,  1204,    87,    88,     0,    89,    90,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,     0,     0,     0,    29,    30,    31,    32,
      33,     0,    34,     0,     0,     0,    35,    36,    37,    38,
       0,    39,     0,    40,     0,    41,     0,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    48,    49,     0,
      50,     0,    52,     0,     0,     0,    53,    54,    55,     0,
      56,    57,    58,     0,    60,    61,     0,     0,     0,     0,
      62,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,   133,    71,
      72,    73,    74,    75,     0,    76,    77,     0,    78,    79,
      80,    81,     0,    82,     0,     0,     0,    83,     0,     0,
       0,     0,     0,    84,     0,    85,    86,  1207,    87,    88,
       0,    89,    90,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    10,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,     0,     0,     0,    29,
      30,    31,    32,    33,     0,    34,     0,     0,     0,    35,
      36,    37,    38,     0,    39,  1209,    40,     0,    41,     0,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      48,    49,     0,    50,     0,    52,     0,     0,     0,    53,
      54,    55,     0,    56,    57,    58,     0,    60,    61,     0,
       0,     0,     0,    62,    63,    64,    65,    66,    67,    68,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,   133,    71,    72,    73,    74,    75,     0,    76,    77,
       0,    78,    79,    80,    81,     0,    82,     0,     0,     0,
      83,     4,     5,     6,     7,     8,    84,     0,    85,    86,
       9,    87,    88,     0,    89,    90,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    10,
      11,    12,     0,     0,     0,     0,    13,     0,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,     0,
      25,    26,    27,    28,     0,     0,     0,    29,    30,    31,
      32,    33,     0,    34,     0,     0,     0,    35,    36,    37,
      38,     0,    39,     0,    40,     0,    41,     0,     0,    42,
       0,     0,     0,    43,    44,    45,    46,     0,    48,    49,
       0,    50,     0,    52,     0,     0,     0,    53,    54,    55,
       0,    56,    57,    58,     0,    60,    61,     0,     0,     0,
       0,    62,    63,    64,    65,    66,    67,    68,     0,     0,
       0,     0,     0,     0,    69,     0,     0,     0,     0,   133,
      71,    72,    73,    74,    75,     0,    76,    77,     0,    78,
      79,    80,    81,     0,    82,     0,     0,     0,    83,     0,
       0,     0,     0,     0,    84,     0,    85,    86,  1211,    87,
      88,     0,    89,    90,     4,     5,     6,     7,     8,     0,
       0,     0,     0,     9,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    10,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,     0,     0,     0,
      29,    30,    31,    32,    33,     0,    34,     0,     0,     0,
      35,    36,    37,    38,     0,    39,     0,    40,     0,    41,
       0,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    48,    49,     0,    50,     0,    52,     0,     0,     0,
      53,    54,    55,     0,    56,    57,    58,     0,    60,    61,
       0,     0,     0,     0,    62,    63,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,   133,    71,    72,    73,    74,    75,     0,    76,
      77,     0,    78,    79,    80,    81,     0,    82,     0,     0,
       0,    83,     0,     0,     0,     0,     0,    84,     0,    85,
      86,  1212,    87,    88,     0,    89,    90,     4,     5,     6,
       7,     8,     0,     0,     0,     0,     9,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    10,    11,    12,     0,     0,
       0,     0,    13,     0,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,    27,    28,
       0,     0,     0,    29,    30,    31,    32,    33,     0,    34,
       0,     0,     0,    35,    36,    37,    38,     0,    39,     0,
      40,     0,    41,     0,     0,    42,     0,     0,     0,    43,
      44,    45,    46,     0,    48,    49,     0,    50,     0,    52,
       0,     0,     0,    53,    54,    55,     0,    56,    57,    58,
       0,    60,    61,     0,     0,     0,     0,    62,    63,    64,
      65,    66,    67,    68,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,   133,    71,    72,    73,    74,
      75,     0,    76,    77,     0,    78,    79,    80,    81,     0,
      82,     0,     0,     0,    83,     0,     0,     0,     0,     0,
      84,     0,    85,    86,  1223,    87,    88,     0,    89,    90,
       4,     5,     6,     7,     8,     0,     0,     0,     0,     9,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    10,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,     0,     0,     0,    29,    30,    31,    32,
      33,     0,    34,     0,     0,     0,    35,    36,    37,    38,
       0,    39,     0,    40,     0,    41,     0,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    48,    49,     0,
      50,     0,    52,     0,     0,     0,    53,    54,    55,     0,
      56,    57,    58,     0,    60,    61,     0,     0,     0,     0,
      62,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,   133,    71,
      72,    73,    74,    75,     0,    76,    77,     0,    78,    79,
      80,    81,     0,    82,     0,     0,     0,    83,     0,     0,
       0,     0,     0,    84,     0,    85,    86,  1249,    87,    88,
       0,    89,    90,     4,     5,     6,     7,     8,     0,     0,
       0,     0,     9,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    10,    11,    12,     0,     0,     0,     0,    13,     0,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,     0,    25,    26,    27,    28,     0,     0,     0,    29,
      30,    31,    32,    33,     0,    34,     0,     0,     0,    35,
      36,    37,    38,     0,    39,     0,    40,     0,    41,     0,
       0,    42,     0,     0,     0,    43,    44,    45,    46,     0,
      48,    49,     0,    50,     0,    52,     0,     0,     0,    53,
      54,    55,     0,    56,    57,    58,     0,    60,    61,     0,
       0,     0,     0,    62,    63,    64,    65,    66,    67,    68,
       0,     0,     0,     0,     0,     0,    69,     0,     0,     0,
       0,   133,    71,    72,    73,    74,    75,     0,    76,    77,
       0,    78,    79,    80,    81,     0,    82,     0,     0,     0,
      83,     0,     0,     0,     0,     0,    84,     0,    85,    86,
    1253,    87,    88,     0,    89,    90,     4,     5,     6,     7,
       8,     0,     0,     0,     0,     9,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    10,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,     0,
       0,     0,    29,    30,    31,    32,    33,     0,    34,     0,
       0,     0,    35,    36,    37,    38,     0,    39,     0,    40,
       0,    41,     0,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    48,    49,     0,    50,     0,    52,     0,
       0,     0,    53,    54,    55,     0,    56,    57,    58,     0,
      60,    61,     0,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,   133,    71,    72,    73,    74,    75,
       0,    76,    77,     0,    78,    79,    80,    81,     0,    82,
       0,     0,     0,    83,     4,     5,     6,     7,     8,    84,
       0,    85,    86,     9,    87,    88,     0,    89,    90,     0,
       0,     0,     0,     0,     0,     0,     0,   350,     0,     0,
       0,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,     0,     0,     0,
      29,    30,    31,    32,    33,     0,    34,   314,   315,     0,
      35,    36,    37,    38,     0,    39,     0,    40,     0,    41,
       0,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    48,    49,     0,    50,     0,    52,     0,     0,     0,
       0,     0,    55,     0,    56,    57,    58,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,    69,     0,   316,
       0,     0,   133,    71,    72,    73,    74,    75,     0,    76,
      77,     0,    78,    79,    80,     0,     0,    82,     0,     0,
       0,    83,     4,     5,     6,     7,     8,    84,     0,    85,
      86,     9,    87,    88,     0,    89,    90,     0,     0,     0,
       0,     0,     0,     0,     0,   529,     0,     0,     0,   329,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,    28,     0,     0,     0,    29,    30,
      31,    32,    33,     0,    34,   314,   315,     0,    35,    36,
      37,    38,     0,    39,     0,    40,     0,    41,     0,     0,
      42,     0,     0,     0,    43,    44,    45,    46,     0,    48,
      49,     0,    50,     0,    52,     0,     0,     0,     0,     0,
      55,     0,    56,    57,    58,     0,     0,     0,     0,     0,
       0,     0,    62,    63,    64,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,    69,     0,   316,     0,     0,
     133,    71,    72,    73,    74,    75,     0,    76,    77,     0,
      78,    79,    80,     0,     0,    82,     0,     0,     0,    83,
       4,     5,     6,     7,     8,    84,     0,    85,    86,     9,
      87,    88,     0,    89,    90,     0,     0,     0,     0,     0,
       0,     0,     0,   670,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,    28,     0,     0,     0,    29,    30,    31,    32,
      33,     0,    34,     0,     0,     0,    35,    36,    37,    38,
       0,    39,     0,    40,     0,    41,     0,     0,    42,     0,
       0,     0,    43,    44,    45,    46,     0,    48,    49,     0,
      50,     0,    52,     0,     0,     0,     0,     0,    55,     0,
      56,    57,    58,     0,     0,     0,     0,     0,     0,     0,
      62,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,   133,    71,
      72,    73,    74,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,    82,     0,     0,     0,    83,     4,     5,
       6,     7,     8,    84,     0,    85,    86,     9,    87,    88,
       0,    89,    90,     0,     0,     0,     0,     0,     0,     0,
       0,  1101,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
      28,     0,     0,     0,    29,    30,    31,    32,    33,     0,
      34,     0,     0,     0,    35,    36,    37,    38,     0,    39,
       0,    40,     0,    41,     0,     0,    42,     0,     0,     0,
      43,    44,    45,    46,     0,    48,    49,     0,    50,     0,
      52,     0,     0,     0,     0,     0,    55,     0,    56,    57,
      58,     0,     0,     0,     0,     0,     0,     0,    62,    63,
      64,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,   133,    71,    72,    73,
      74,    75,     0,    76,    77,     0,    78,    79,    80,     0,
       0,    82,     0,     0,     0,    83,     4,     5,     6,     7,
       8,    84,     0,    85,    86,     9,    87,    88,     0,    89,
      90,     0,     0,     0,     0,     0,     0,     0,     0,  1147,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,     0,
       0,     0,    29,    30,    31,    32,    33,     0,    34,     0,
       0,     0,    35,    36,    37,    38,     0,    39,     0,    40,
       0,    41,     0,     0,    42,     0,     0,     0,    43,    44,
      45,    46,     0,    48,    49,     0,    50,     0,    52,     0,
      31,     0,    33,     0,    55,     0,    56,    57,    58,     0,
       0,     0,     0,     0,     0,     0,    62,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,   133,    71,    72,    73,    74,    75,
     157,    76,    77,     0,    78,    79,    80,     0,     0,    82,
       0,     0,   397,    83,     4,     5,     6,     7,     8,    84,
       0,    85,    86,     9,    87,    88,     0,    89,    90,     0,
     133,     0,     0,    73,     0,    75,     0,    76,    77,     0,
      78,    79,    80,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,   158,     0,     0,     0,    13,
      87,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,    28,     0,     0,     0,
      29,    30,    31,    32,    33,     0,    34,     0,     0,     0,
      35,    36,    37,    38,     0,    39,     0,    40,     0,    41,
       0,     0,    42,     0,     0,     0,    43,    44,    45,    46,
       0,    48,    49,     0,    50,     0,    52,     0,    31,     0,
      33,     0,    55,     0,    56,    57,    58,     0,     0,     0,
       0,     0,     0,     0,    62,    63,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,   133,    71,    72,    73,    74,    75,   157,    76,
      77,     0,    78,    79,    80,     0,     0,    82,     0,     0,
     706,    83,     4,     5,     6,     7,     8,    84,     0,    85,
      86,     9,    87,    88,     0,    89,    90,     0,   133,     0,
       0,    73,     0,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,   158,     0,     0,     0,    13,    87,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,     0,     0,     0,     0,    29,    30,
      31,    32,    33,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,     0,     0,     0,     0,     0,    31,     0,    33,     0,
     131,     0,     0,    57,    58,     0,     0,     0,     0,     0,
       0,     0,   132,    63,    64,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,    69,     0,     0,     0,     0,
     133,    71,    72,    73,     0,    75,   165,    76,    77,     0,
      78,    79,    80,     0,     0,    82,     0,     0,     0,    83,
       4,     5,     6,     7,     8,    84,     0,   191,     0,     9,
      87,    88,     0,    89,    90,     0,   133,     0,     0,    73,
       0,    75,     0,    76,    77,     0,    78,    79,    80,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,   166,     0,     0,     0,    13,    87,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,     0,     0,     0,     0,    29,    30,    31,    32,
      33,     0,     0,     0,     0,     0,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   131,     0,
       0,    57,    58,     0,     0,     0,     0,     0,     0,     0,
     132,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,   133,    71,
      72,    73,     0,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,    82,     0,     0,     0,    83,     4,     5,
       6,     7,     8,    84,     0,   201,     0,     9,    87,    88,
       0,    89,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
       0,     0,     0,     0,    29,    30,    31,    32,    33,     0,
       0,     0,     0,     0,    35,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     238,     0,     0,    46,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   131,     0,     0,    57,
      58,     0,     0,     0,     0,     0,     0,     0,   132,    63,
      64,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,   133,    71,    72,    73,
       0,    75,     0,    76,    77,     0,    78,    79,    80,     0,
       0,    82,     0,     0,     0,    83,     4,     5,     6,     7,
       8,    84,     0,     0,     0,     9,    87,    88,     0,    89,
      90,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,     0,
       0,     0,    35,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   131,     0,     0,    57,    58,     0,
       0,     0,     0,     0,     0,     0,   132,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,   133,    71,    72,    73,     0,    75,
       0,    76,    77,     0,    78,    79,    80,     0,     0,    82,
       0,     0,     0,    83,     4,     5,     6,     7,     8,    84,
     348,     0,     0,     9,    87,    88,     0,    89,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   400,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,     0,     0,     0,     0,
      29,    30,    31,    32,    33,     0,     0,     0,     0,     0,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   131,     0,     0,    57,    58,     0,     0,     0,
       0,     0,     0,     0,   132,    63,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,   133,    71,    72,    73,     0,    75,     0,    76,
      77,     0,    78,    79,    80,     0,     0,    82,     0,     0,
       0,    83,     4,     5,     6,     7,     8,    84,     0,     0,
       0,     9,    87,    88,     0,    89,    90,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     438,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,     0,     0,     0,     0,    29,    30,
      31,    32,    33,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     131,     0,     0,    57,    58,     0,     0,     0,     0,     0,
       0,     0,   132,    63,    64,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,    69,     0,     0,     0,     0,
     133,    71,    72,    73,     0,    75,     0,    76,    77,     0,
      78,    79,    80,     0,     0,    82,     0,     0,     0,    83,
       4,     5,     6,     7,     8,    84,     0,     0,     0,     9,
      87,    88,     0,    89,    90,     0,     0,     0,     0,     0,
       0,     0,     0,   449,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,     0,     0,     0,     0,    29,    30,    31,    32,
      33,     0,     0,     0,     0,     0,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   131,     0,
       0,    57,    58,     0,     0,     0,     0,     0,     0,     0,
     132,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,   133,    71,
      72,    73,     0,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,    82,     0,     0,     0,    83,     4,     5,
       6,     7,     8,    84,     0,     0,     0,     9,    87,    88,
       0,    89,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   486,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
       0,     0,     0,     0,    29,    30,    31,    32,    33,     0,
       0,     0,     0,     0,    35,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   131,     0,     0,    57,
      58,     0,     0,     0,     0,     0,     0,     0,   132,    63,
      64,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,   133,    71,    72,    73,
       0,    75,     0,    76,    77,     0,    78,    79,    80,     0,
       0,    82,     0,     0,     0,    83,     4,     5,     6,     7,
       8,    84,     0,     0,     0,     9,    87,    88,     0,    89,
      90,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   709,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,     0,
       0,     0,    35,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   131,     0,     0,    57,    58,     0,
       0,     0,     0,     0,     0,     0,   132,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,   133,    71,    72,    73,     0,    75,
       0,    76,    77,     0,    78,    79,    80,     0,     0,    82,
       0,     0,     0,    83,     4,     5,     6,     7,     8,    84,
       0,     0,     0,     9,    87,    88,     0,    89,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   711,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,     0,     0,     0,     0,
      29,    30,    31,    32,    33,     0,     0,     0,     0,     0,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   131,     0,     0,    57,    58,     0,     0,     0,
       0,     0,     0,     0,   132,    63,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,   133,    71,    72,    73,     0,    75,     0,    76,
      77,     0,    78,    79,    80,     0,     0,    82,     0,     0,
       0,    83,     4,     5,     6,     7,     8,    84,     0,     0,
       0,     9,    87,    88,     0,    89,    90,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     724,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    11,    12,     0,     0,     0,     0,    13,     0,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
       0,    25,    26,    27,     0,     0,     0,     0,    29,    30,
      31,    32,    33,     0,     0,     0,     0,     0,    35,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     131,     0,     0,    57,    58,     0,     0,     0,     0,     0,
       0,     0,   132,    63,    64,    65,    66,    67,    68,     0,
       0,     0,     0,     0,     0,    69,     0,     0,     0,     0,
     133,    71,    72,    73,     0,    75,     0,    76,    77,     0,
      78,    79,    80,     0,     0,    82,     0,     0,     0,    83,
       4,     5,     6,     7,     8,    84,     0,     0,     0,     9,
      87,    88,     0,    89,    90,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    11,
      12,     0,     0,     0,     0,    13,     0,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,     0,    25,
      26,    27,     0,     0,     0,     0,    29,    30,    31,    32,
      33,     0,     0,     0,     0,     0,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   131,     0,
       0,    57,    58,     0,     0,     0,     0,     0,     0,     0,
     132,    63,    64,    65,    66,    67,    68,     0,     0,     0,
       0,     0,     0,    69,     0,     0,     0,     0,   133,    71,
      72,    73,   805,    75,     0,    76,    77,     0,    78,    79,
      80,     0,     0,    82,     0,     0,     0,    83,     4,     5,
       6,     7,     8,    84,     0,     0,     0,     9,    87,    88,
       0,    89,    90,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   904,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    11,    12,     0,
       0,     0,     0,    13,     0,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,    27,
       0,     0,     0,     0,    29,    30,    31,    32,    33,     0,
       0,     0,     0,     0,    35,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   131,     0,     0,    57,
      58,     0,     0,     0,     0,     0,     0,     0,   132,    63,
      64,    65,    66,    67,    68,     0,     0,     0,     0,     0,
       0,    69,     0,     0,     0,     0,   133,    71,    72,    73,
       0,    75,     0,    76,    77,     0,    78,    79,    80,     0,
       0,    82,     0,     0,     0,    83,     4,     5,     6,     7,
       8,    84,     0,     0,     0,     9,    87,    88,     0,    89,
      90,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    11,    12,     0,     0,     0,
       0,    13,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,     0,     0,
       0,     0,    29,    30,    31,    32,    33,     0,     0,     0,
       0,     0,    35,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   131,     0,     0,    57,    58,     0,
       0,     0,     0,     0,     0,     0,   132,    63,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,    69,
       0,     0,     0,     0,   133,    71,    72,    73,     0,    75,
       0,    76,    77,     0,    78,    79,    80,     0,     0,    82,
       0,     0,     0,    83,     4,     5,     6,     7,     8,    84,
       0,     0,     0,     9,    87,    88,     0,    89,    90,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,    12,     0,     0,     0,     0,    13,
       0,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,     0,    25,    26,    27,     0,     0,     0,     0,
      29,    30,    31,   408,    33,     0,     0,     0,     0,     0,
      35,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   131,     0,     0,    57,    58,     0,     0,     0,
       0,     0,     0,     0,   132,    63,    64,    65,    66,    67,
      68,     0,     0,     0,     0,     0,     0,    69,     0,     0,
       0,     0,   133,    71,    72,    73,     0,    75,     0,    76,
      77,     0,    78,    79,    80,     0,     0,    82,     0,     0,
       0,    83,   267,   268,   269,     0,     0,    84,     0,     0,
       0,     0,    87,    88,     0,    89,    90,     0,   270,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,     0,   292,   267,   268,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   270,
       0,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,     0,   292,   267,   268,   269,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,     0,   292,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   267,   268,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   270,   602,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,     0,   292,   267,   268,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   270,   635,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,     0,   292,   267,   268,
     269,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,   638,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,     0,   292,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     267,   268,   269,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   270,   694,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,     0,
     292,   267,   268,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   747,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
       0,   292,   267,   268,   269,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   763,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,     0,   292,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   267,   268,   269,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   937,   270,   881,   271,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,     0,   292,   267,   268,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,  1080,   270,     0,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,     0,   292,   267,   268,
     269,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1081,   270,     0,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,     0,   292,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   267,   268,
     269,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   270,   882,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,     0,   292,   267,
     268,   269,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   270,   293,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,     0,   292,
     267,   268,   269,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   270,   362,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,     0,
     292,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     267,   268,   269,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   270,   364,   271,   272,
     273,   274,   275,   276,   277,   278,   279,   280,   281,   282,
     283,   284,   285,   286,   287,   288,   289,   290,   291,     0,
     292,   267,   268,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   375,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
       0,   292,   267,   268,   269,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   377,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,     0,   292,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   267,   268,   269,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,   419,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,     0,   292,   267,   268,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   270,
     746,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,     0,   292,   267,   268,   269,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,   979,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,     0,   292,   988,   989,   990,   991,   992,
       0,   993,   994,   995,   996,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     509,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   997,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   998,   999,  1000,  1001,  1002,  1003,
    1004,     0,     0,    31,     0,     0,     0,   578,     0,     0,
       0,   524,  1005,  1006,  1007,  1008,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1022,
    1023,  1024,  1025,  1026,  1027,  1028,  1029,  1030,  1031,  1032,
    1033,  1034,  1035,  1036,  1037,  1038,  1039,  1040,  1041,  1042,
    1043,  1044,  1045,     0,     0,  1046,  1047,  1048,  1049,  1050,
    1051,  1052,     0,     0,     0,   543,   544,     0,     0,     0,
       0,     0,     0,  1053,  1054,  1055,     0,  1056,     0,     0,
      76,    77,     0,    78,    79,    80,  1057,     0,  1058,     0,
       0,  1059,    29,    30,    31,   267,   268,   269,     0,     0,
       0,     0,    35,     0,     0,     0,     0,     0,     0,     0,
       0,   270,     0,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,     0,   292,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   545,    64,    65,
      66,    67,    68,     0,     0,     0,     0,     0,     0,   546,
       0,     0,     0,     0,   133,    71,    72,    73,     0,   547,
       0,    76,    77,     0,    78,    79,    80,     0,     0,    82,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   267,   268,   269,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   270,   618,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
       0,   292,   267,   268,   269,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   270,     0,
     271,   272,   273,   274,   275,   276,   277,   278,   279,   280,
     281,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     291,     0,   292,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   267,   268,   269,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   270,   809,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,     0,   292,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   852,     0,     0,     0,     0,
       0,     0,     0,   267,   268,   269,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   533,   270,
     615,   271,   272,   273,   274,   275,   276,   277,   278,   279,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,     0,   292,   267,   268,   269,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,     0,   292,   268,   269,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     270,     0,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,     0,   292
};

static const yytype_int16 yycheck[] =
{
       2,     2,   135,    81,   322,    28,     2,   260,   729,     2,
     327,     2,   831,   403,   121,    38,   167,   257,   828,    42,
     128,    84,   831,    26,   342,   695,   104,   292,   756,   898,
     532,   759,    26,   415,     2,   699,    46,     8,     8,    13,
     610,   173,    44,     8,    41,    47,     8,   198,   173,    61,
      61,    23,    24,   704,    26,   203,    51,     8,   386,    61,
       8,    41,   732,     8,     8,    26,     8,   174,     8,    73,
       8,     8,   121,    46,     8,    70,   538,   946,    73,    81,
       8,     8,     8,   216,     8,    61,    78,    55,   478,     8,
       8,   742,     8,     8,    78,   158,    31,   121,    31,   121,
       2,    61,   104,   166,     8,    90,   166,     8,   193,   194,
       8,    61,   197,    29,    73,     8,    73,   147,   167,    99,
       0,    23,    24,   933,   104,   695,   106,   107,   108,   109,
     110,   111,   112,   137,   138,   139,   146,    31,    73,   169,
      73,    98,   144,   167,   169,   137,    41,   717,   170,   166,
     118,   161,    90,   137,   166,   166,   146,   169,   169,   144,
     157,   121,   732,   131,   140,    61,   166,   300,   148,   149,
     167,   151,   257,   146,    61,   169,    61,   179,   179,    73,
     182,   182,    84,    90,   186,   263,   158,   857,   162,   297,
     170,   193,   194,   169,   166,   197,    61,   167,   200,   170,
     851,    61,   350,   168,   855,   167,   534,   168,   316,   591,
     205,   673,   171,   675,   365,   543,   544,   168,   169,   169,
     168,   299,   167,   301,   168,  1094,   168,   167,   956,   337,
     958,   168,   802,   341,   168,   899,   344,   144,   210,   234,
     168,   168,   168,    61,   168,   247,   218,   219,   220,   168,
     168,   167,   167,   225,   256,   257,   158,   574,   260,   231,
     650,   263,   157,   167,   166,   163,   167,    61,   102,   771,
     163,   334,   167,   169,    61,   845,   166,   179,    73,   247,
     182,   609,   169,   368,   169,   166,   856,   857,    61,   292,
      61,   423,    61,    61,   297,   297,   298,   299,   423,   301,
     121,   541,   953,   121,   169,   623,    61,    73,   210,   169,
      93,    94,    78,   316,   316,   633,   218,   219,   220,    13,
     292,    73,    73,   225,   166,   159,   117,   121,   479,   231,
      93,    94,    26,   162,   337,   337,    31,   166,   341,    25,
     117,   344,   344,   138,   139,   247,   167,    73,    26,    43,
     501,   169,    78,   355,   355,   327,    42,   685,   166,    45,
    1229,   689,   334,   628,   144,   516,   368,   166,   696,  1238,
      66,    67,   138,   139,   376,   169,  1195,   947,    73,   381,
     950,   529,   169,   515,   379,   517,  1195,   170,   168,   169,
     515,   166,   517,   395,    66,    67,   169,   548,   169,   789,
     169,   169,   404,   405,   794,  1215,   484,   170,   376,   175,
     517,    73,   138,   139,   169,    71,    78,    73,  1146,   171,
     171,   739,    71,   140,   166,   327,   295,   166,   400,   140,
     748,   166,   334,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   162,   147,   775,   174,   166,
     319,   162,   169,   355,   323,    73,   541,   542,   169,   160,
      78,  1182,   168,   169,    25,   169,   438,   144,   169,    71,
      61,    73,  1122,  1123,   376,   865,   138,   139,    73,    59,
      60,    42,   484,    78,    45,  1118,  1119,   804,    61,    31,
     598,    25,   148,   149,    61,   151,   152,   153,   400,   148,
     149,   166,   151,   152,   153,   528,   169,    73,   510,   140,
     512,   166,    78,   169,   486,   171,   166,    51,   603,   137,
     138,   139,   670,   525,   842,  1095,    61,    98,    71,  1199,
      73,   166,   144,   535,   536,   536,   438,    71,    40,   541,
     542,   121,   860,   138,   139,    49,   148,   149,  1218,   151,
     152,   153,    45,    46,    47,   557,    49,   525,   162,   812,
     888,   533,   890,   373,   892,   140,   894,   169,   121,   171,
     721,   137,   138,   139,   166,    73,   386,    92,    93,    94,
      78,    25,   140,   140,   486,   140,   168,   167,   590,   590,
     124,   121,   677,   565,   590,   598,   598,   590,   162,   590,
      13,   603,   574,   575,    13,   148,   149,    51,   151,   152,
     153,   167,   146,   162,   148,   149,   168,   151,   152,   153,
     938,   167,   590,   525,   172,   628,   169,    71,   171,  1199,
     167,   533,   166,   166,   536,   166,   659,   166,   640,   641,
     138,   139,    92,    93,    94,     8,   731,   167,  1218,   167,
      71,    82,    73,   981,    98,     8,   628,   168,   660,   660,
     166,   663,    13,   565,   106,   107,   108,   109,   110,   111,
       8,   166,   574,   575,    73,   677,   168,   930,   119,   166,
     124,    88,    89,    90,   167,   687,   688,     8,   590,    69,
      70,    71,    61,   665,   169,   663,   161,   699,   700,   784,
      61,   167,   146,     8,   148,   149,    13,   151,   152,   153,
     173,   119,   173,   170,   716,   716,     8,   166,   166,   173,
     716,   869,   166,   716,   534,   716,   168,   148,   149,   731,
     151,   152,   153,   543,   544,   883,   173,   709,   167,   711,
     167,   764,   168,   828,   168,   140,   166,    71,   716,   897,
     171,   140,   724,   169,   756,   102,    71,   759,   660,    13,
     163,   663,  1090,   665,  1082,   767,   767,   168,   148,   149,
      13,   151,   152,   153,   776,   169,   173,    25,  1106,  1107,
      13,   166,   784,    41,    23,    24,    71,    26,   120,  1117,
     166,   169,   166,    13,   927,   166,     8,   167,    13,   609,
    1128,   773,   887,    51,   168,  1133,   168,   709,   776,   711,
     812,   122,   960,  1141,   716,     8,   159,   167,   167,   967,
     144,   823,   724,    71,   148,   149,   828,   151,   152,   153,
     115,   146,   804,   148,   149,   137,   151,   152,   153,   166,
       8,    99,   167,   166,   166,   169,   104,   373,   106,   107,
     108,   109,   110,   111,   112,   940,     8,  1185,   943,   827,
     386,   169,   166,   148,   149,   767,   151,   152,   153,    71,
    1198,   773,   167,   169,   776,   685,   124,  1205,   166,   689,
     167,   166,   137,    26,    68,   887,   696,   168,   167,   163,
     148,   149,   168,   151,    26,   122,   167,   899,   146,     8,
     148,   149,   804,   151,   152,   153,   122,   170,     8,   157,
     169,   167,   170,     8,   916,   150,   166,   170,   166,   158,
     922,    26,    25,   167,   169,   167,   167,   166,   930,   167,
     122,     8,   904,    26,   168,   167,    73,   168,   940,   168,
      13,   943,   144,   169,  1077,   168,   148,   149,    51,   151,
     152,   153,    73,  1101,   956,   146,   958,    41,    98,   961,
     166,    73,   964,   964,   932,   775,   968,   167,    71,   109,
     110,   210,    13,   975,   104,   122,   122,   117,   118,   218,
     219,   220,   167,    13,   167,   167,   225,   167,    13,   167,
    1138,   166,   231,   961,    13,  1143,  1074,   169,   169,  1147,
     968,    51,   904,   122,   169,  1153,  1154,   975,   534,   169,
      73,   166,    73,   167,   154,    99,    13,   543,   544,   169,
     104,   124,   106,   107,   108,   109,   110,   111,   112,   140,
      90,    90,    13,   153,    23,    24,    29,    26,   166,    73,
    1188,     8,    13,   146,   168,   148,   149,    71,   151,   152,
     153,   167,   167,   292,   157,   168,  1134,   155,    73,   961,
     168,   167,   964,   166,   148,   149,   968,   151,    73,   166,
     169,   167,  1074,   975,   299,  1225,    71,   301,   888,   379,
     890,   298,   892,   609,   894,   118,   170,   627,   327,   228,
     744,   734,   624,   232,  1242,   334,  1226,  1099,  1100,  1100,
     773,   880,  1104,  1251,   106,   107,   108,   109,   110,   111,
     249,  1113,   251,   252,   253,   254,  1118,  1119,   384,   986,
    1122,  1123,   146,    71,   148,   149,   150,   151,   152,   153,
     823,  1099,  1134,  1063,   373,  1244,  1104,    36,    84,  1075,
     976,   718,   166,   652,  1146,   745,   141,   386,  1150,   144,
     641,   146,   219,   148,   149,   845,   151,   152,   153,   685,
      -1,   400,   943,   689,    -1,    -1,    -1,    -1,    -1,   158,
     696,   981,    -1,    -1,    -1,   985,    -1,   166,    -1,    -1,
      -1,  1183,  1150,    -1,    -1,  1187,    -1,    -1,  1190,    -1,
    1192,  1193,  1194,    -1,    -1,    -1,    -1,  1099,  1100,   438,
     148,   149,  1104,   151,   152,   153,  1208,    -1,    -1,    -1,
      -1,    -1,  1214,    -1,    -1,  1183,    -1,    -1,   166,  1187,
      -1,   210,  1190,    -1,  1192,  1193,    -1,    -1,    -1,   218,
     219,   220,    71,    -1,    73,    74,   225,     9,    10,    11,
    1208,    -1,   231,    -1,  1246,    -1,    -1,   486,  1150,   775,
    1252,    -1,    -1,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,  1246,    -1,
    1090,  1183,    -1,    -1,  1252,  1187,    -1,    -1,  1190,    -1,
    1192,  1193,    -1,    -1,   533,   534,  1106,  1107,    -1,    -1,
      -1,    -1,    -1,   292,   543,   544,  1208,  1117,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,    -1,  1128,    -1,
      -1,    -1,    -1,  1133,    -1,    -1,   565,    -1,    -1,    -1,
      -1,  1141,    -1,   572,    -1,   574,   575,    25,   327,    -1,
      -1,    -1,    -1,    -1,  1246,   334,    -1,    -1,    -1,    -1,
    1252,  1161,    -1,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      -1,    -1,   888,    51,   890,    -1,   892,    -1,   894,    -1,
     609,    -1,    -1,   612,    -1,  1185,    -1,    42,    43,    -1,
      -1,    -1,    -1,    71,   373,    -1,    -1,    -1,  1198,   628,
      -1,    -1,    -1,    -1,    -1,  1205,    -1,   386,    -1,    -1,
      -1,   173,    -1,    -1,    69,    70,    71,    -1,    -1,    -1,
      98,   400,    -1,    -1,    79,    -1,    -1,    -1,   106,   107,
     108,   109,   110,   111,    -1,    -1,   665,   141,    -1,    -1,
     144,    -1,    -1,    -1,   148,   149,   124,   151,   152,   153,
      42,    43,    44,    45,    46,    47,   685,    49,    -1,   438,
     689,    -1,    -1,    -1,    -1,   981,   170,   696,   146,   124,
     148,   149,    -1,   151,   152,   153,    69,    70,    -1,    -1,
     709,   136,   711,    -1,    -1,    -1,    79,    -1,   166,    31,
       9,    10,    11,   148,   149,   724,   151,   152,   153,    -1,
      -1,    -1,    -1,    -1,    -1,   734,    25,   486,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    71,
      49,    73,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,   773,    -1,   775,    -1,    -1,   142,
     143,    -1,    -1,    -1,   533,   534,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   156,   543,   544,   795,    -1,    -1,   111,
      -1,    -1,   801,    41,  1090,   804,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   813,    -1,   565,    -1,    -1,    -1,
    1106,  1107,   821,   572,    -1,   574,   575,  1113,    -1,   141,
      -1,  1117,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,  1128,    -1,    -1,    -1,    -1,  1133,    -1,    -1,
      -1,    -1,    -1,    -1,   166,  1141,    -1,    41,    -1,   171,
     609,    99,    -1,   612,    -1,    -1,   104,    -1,   106,   107,
     108,   109,   110,   111,   112,    -1,    71,    -1,    -1,   628,
      -1,   170,    -1,    -1,    -1,    -1,    -1,    -1,   373,   888,
      -1,   890,    -1,   892,    -1,   894,    -1,    -1,    -1,  1185,
      -1,   386,    -1,    -1,    -1,   904,    -1,    -1,    -1,    -1,
     148,   149,  1198,   151,    -1,    99,   665,   112,    -1,  1205,
     104,    -1,   106,   107,   108,   109,   110,   111,   112,   124,
      -1,    -1,   170,    -1,    -1,    -1,   685,    -1,    -1,    -1,
     689,    -1,    -1,    -1,    -1,    -1,   141,   696,    -1,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
     709,    -1,   711,    -1,   148,   149,    -1,   151,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   724,    -1,    -1,    -1,    -1,
      -1,    -1,   981,   982,    -1,   734,   170,   986,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     4,     5,    -1,     7,     8,
       9,    -1,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    -1,    41,    25,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   773,    -1,   775,    36,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    -1,    45,    -1,    -1,    48,
      -1,    50,    -1,    -1,    -1,    -1,   795,    -1,    -1,   534,
      -1,    -1,   801,    -1,    -1,   804,    -1,    -1,   543,   544,
      -1,    -1,    -1,    -1,   813,    74,    -1,    -1,    -1,    -1,
      -1,    -1,   821,    99,    -1,    84,    -1,    -1,   104,    -1,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,  1090,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    -1,  1106,  1107,    -1,
      71,    -1,    73,    -1,  1113,    -1,    -1,    -1,  1117,    -1,
      -1,    -1,   148,   149,   609,   151,    -1,    -1,    -1,  1128,
     139,    -1,    -1,    -1,  1133,    -1,    -1,    -1,    -1,   888,
      -1,   890,  1141,   892,   170,   894,    -1,    -1,    -1,    -1,
     111,    -1,    -1,    -1,    -1,   904,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   175,    -1,    -1,   178,
      -1,    -1,    -1,    -1,    -1,   184,   185,    -1,    -1,    -1,
     141,    -1,    -1,   144,    -1,   146,  1185,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,  1198,
     685,    -1,    -1,    -1,   689,   166,  1205,    -1,    -1,    -1,
     171,   696,    -1,    -1,    -1,    -1,    -1,   226,    -1,    -1,
      -1,   230,    -1,    -1,    -1,    -1,    -1,  1226,    -1,    -1,
      -1,    -1,   981,   982,    -1,    -1,    -1,   986,    -1,   248,
    1239,    63,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
     259,    73,    -1,    -1,    -1,    -1,    -1,    -1,   267,   268,
     269,   270,   271,   272,   273,   274,   275,   276,   277,   278,
     279,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   291,    -1,    -1,   294,   295,   296,    -1,   111,
     775,    -1,    -1,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,    -1,    -1,    -1,    -1,    -1,
     319,   320,    -1,   322,   323,   324,    -1,    -1,    -1,   141,
     329,    -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,   342,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,  1090,    -1,   352,   166,    -1,    -1,    -1,    71,   171,
      73,    -1,    -1,    -1,    -1,    -1,    -1,  1106,  1107,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1117,    -1,
      -1,    -1,    -1,   382,    -1,    -1,    -1,    -1,    -1,  1128,
      -1,    -1,    -1,    -1,  1133,    -1,    -1,    -1,   111,    -1,
      -1,    -1,  1141,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   888,    -1,   890,    -1,   892,    -1,   894,
      -1,    -1,    -1,     3,     4,     5,     6,     7,   141,    -1,
      -1,   144,    12,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,    -1,    -1,  1185,    -1,    -1,    -1,
     449,    31,    -1,   166,    -1,    -1,   169,    -1,   171,  1198,
      -1,    -1,    42,    43,    -1,    -1,  1205,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    -1,    -1,  1226,   487,    69,
      70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,
    1239,   500,    -1,    -1,    -1,    -1,   981,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
      -1,    -1,   521,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   111,    -1,   532,   114,   115,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,   578,
     160,   580,    71,    -1,    73,    -1,   166,    -1,   587,    -1,
      -1,   171,   172,    -1,   174,   175,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   607,    49,
      -1,    -1,    -1,    -1,    -1,  1090,   615,    -1,    -1,   618,
      -1,   620,   111,    -1,   623,    -1,    -1,    -1,    -1,    -1,
      -1,  1106,  1107,    -1,   633,    -1,     3,     4,     5,     6,
       7,    -1,  1117,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,   141,  1128,    -1,   144,    -1,   146,  1133,   148,
     149,    -1,   151,   152,   153,    -1,  1141,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,   166,    -1,    -1,
      -1,    48,   171,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,   708,
    1185,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   720,    -1,  1198,    -1,    -1,    -1,    -1,    -1,    -1,
    1205,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     739,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,   748,
      -1,   750,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,   771,    -1,   141,   142,   143,   144,    -1,   146,
     779,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,    -1,    -1,    -1,    -1,    -1,   166,
      -1,   168,    -1,    -1,   171,   172,   805,   174,   175,    -1,
     809,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,   834,    -1,    -1,    -1,   838,
      -1,    -1,    -1,   842,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   852,    -1,    -1,    -1,    -1,    41,    42,
      43,   860,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      -1,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    99,   100,   101,    -1,
     103,   104,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,   938,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,   168,   169,   170,   171,   172,
      11,   174,   175,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,  1082,    -1,    95,    96,    97,    98,    99,
     100,   101,    -1,   103,   104,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,   113,   114,   115,   116,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,   168,   169,
     170,   171,   172,    -1,   174,   175,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    25,    12,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    99,   100,   101,    -1,   103,   104,   105,    -1,
      -1,    -1,   109,   110,   111,    -1,   113,   114,   115,   116,
     117,   118,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,   154,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,   168,   169,    12,   171,   172,    -1,   174,   175,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,   168,
     169,   170,   171,   172,    -1,   174,   175,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    63,    64,    65,
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
     166,    -1,   168,   169,   170,   171,   172,    -1,   174,   175,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    65,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,
      83,    84,    -1,    86,    -1,    88,    -1,    -1,    91,    -1,
      -1,    -1,    95,    96,    97,    98,    -1,   100,   101,    -1,
     103,    -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,
     113,   114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,   168,   169,    12,   171,   172,
      -1,   174,   175,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    -1,    88,    89,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,   168,   169,    12,   171,   172,    -1,   174,
     175,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    65,    -1,
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
      -1,   168,   169,   170,   171,   172,    -1,   174,   175,     3,
       4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,
      -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    -1,    62,    63,
      64,    65,    -1,    -1,    -1,    69,    70,    71,    72,    73,
      -1,    75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,
      84,    -1,    86,    87,    88,    -1,    -1,    91,    -1,    -1,
      -1,    95,    96,    97,    98,    -1,   100,   101,    -1,   103,
      -1,   105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,
     114,   115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,
     124,   125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,
      -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,
     144,   145,   146,    -1,   148,   149,    -1,   151,   152,   153,
     154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,
       6,     7,   166,    -1,   168,   169,    12,   171,   172,    -1,
     174,   175,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    63,    64,    65,
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
     166,    -1,   168,   169,   170,   171,   172,    -1,   174,   175,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
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
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,   168,   169,   170,   171,   172,
      -1,   174,   175,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    85,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,     3,     4,     5,     6,     7,   166,    -1,   168,   169,
      12,   171,   172,    -1,   174,   175,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,
      42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    -1,
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
      -1,    -1,    -1,    -1,   166,    -1,   168,   169,   170,   171,
     172,    -1,   174,   175,     3,     4,     5,     6,     7,    -1,
      -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,    -1,    -1,    -1,    -1,    -1,   166,    -1,   168,
     169,   170,   171,   172,    -1,   174,   175,     3,     4,     5,
       6,     7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,
      -1,    -1,    48,    -1,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    -1,    62,    63,    64,    65,
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
     166,    -1,   168,   169,   170,   171,   172,    -1,   174,   175,
       3,     4,     5,     6,     7,    -1,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
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
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,   154,    -1,   156,    -1,    -1,    -1,   160,    -1,    -1,
      -1,    -1,    -1,   166,    -1,   168,   169,   170,   171,   172,
      -1,   174,   175,     3,     4,     5,     6,     7,    -1,    -1,
      -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    -1,    62,    63,    64,    65,    -1,    -1,    -1,    69,
      70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,
      80,    81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,
      -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,
     100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,
     110,   111,    -1,   113,   114,   115,    -1,   117,   118,    -1,
      -1,    -1,    -1,   123,   124,   125,   126,   127,   128,   129,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,
      -1,   141,   142,   143,   144,   145,   146,    -1,   148,   149,
      -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,
     160,    -1,    -1,    -1,    -1,    -1,   166,    -1,   168,   169,
     170,   171,   172,    -1,   174,   175,     3,     4,     5,     6,
       7,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    65,    -1,
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
      -1,   168,   169,    12,   171,   172,    -1,   174,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,
      -1,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    59,    60,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,
      -1,    -1,   111,    -1,   113,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,   121,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,   168,
     169,    12,   171,   172,    -1,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      -1,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    59,    60,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,
     111,    -1,   113,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,   121,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,   168,   169,    12,
     171,   172,    -1,   174,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
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
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,   168,   169,    12,   171,   172,
      -1,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,    63,    64,
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
       7,   166,    -1,   168,   169,    12,   171,   172,    -1,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    -1,    62,    63,    64,    65,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    75,    -1,
      -1,    -1,    79,    80,    81,    82,    -1,    84,    -1,    86,
      -1,    88,    -1,    -1,    91,    -1,    -1,    -1,    95,    96,
      97,    98,    -1,   100,   101,    -1,   103,    -1,   105,    -1,
      71,    -1,    73,    -1,   111,    -1,   113,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
     111,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,   123,   160,     3,     4,     5,     6,     7,   166,
      -1,   168,   169,    12,   171,   172,    -1,   174,   175,    -1,
     141,    -1,    -1,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,   166,    -1,    -1,    -1,    48,
     171,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    -1,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,    -1,    71,    -1,
      73,    -1,   111,    -1,   113,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,   111,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
     123,   160,     3,     4,     5,     6,     7,   166,    -1,   168,
     169,    12,   171,   172,    -1,   174,   175,    -1,   141,    -1,
      -1,   144,    -1,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,   166,    -1,    -1,    -1,    48,   171,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      -1,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    71,    -1,    73,    -1,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,   111,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,   168,    -1,    12,
     171,   172,    -1,   174,   175,    -1,   141,    -1,    -1,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,   166,    -1,    -1,    -1,    48,   171,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
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
       5,     6,     7,   166,    -1,   168,    -1,    12,   171,   172,
      -1,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,    63,    64,
      -1,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      95,    -1,    -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,    -1,   114,
     115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
      -1,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
     167,    -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    63,    64,    -1,    -1,    -1,    -1,
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
      -1,    12,   171,   172,    -1,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      -1,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
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
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,    63,    64,
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
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
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
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    63,    64,    -1,    -1,    -1,    -1,
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
      -1,    12,   171,   172,    -1,   174,   175,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      -1,    62,    63,    64,    -1,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    -1,    -1,    -1,    -1,    79,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     111,    -1,    -1,   114,   115,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,    -1,   146,    -1,   148,   149,    -1,
     151,   152,   153,    -1,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,
      43,    -1,    -1,    -1,    -1,    48,    -1,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    -1,    62,
      63,    64,    -1,    -1,    -1,    -1,    69,    70,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    79,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   111,    -1,
      -1,   114,   115,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     123,   124,   125,   126,   127,   128,   129,    -1,    -1,    -1,
      -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,   141,   142,
     143,   144,   145,   146,    -1,   148,   149,    -1,   151,   152,
     153,    -1,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,    62,    63,    64,
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
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    -1,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     9,    10,    11,    -1,    -1,   166,    -1,    -1,
      -1,    -1,   171,   172,    -1,   174,   175,    -1,    25,    -1,
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
      45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   170,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   170,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   170,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   170,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   170,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   170,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   170,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   170,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   170,    25,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,
      11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    25,   168,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,     9,
      10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    25,   168,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   168,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,   168,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   168,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   168,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   168,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
     168,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      25,   168,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     167,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    66,    67,
      68,    -1,    -1,    71,    -1,    -1,    -1,   122,    -1,    -1,
      -1,   167,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,    -1,    -1,   123,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    -1,    -1,   141,   142,   143,    -1,   145,    -1,    -1,
     148,   149,    -1,   151,   152,   153,   154,    -1,   156,    -1,
      -1,   159,    69,    70,    71,     9,    10,    11,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,   122,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     9,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    25,   122,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   122,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,    -1,
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
      45,    46,    47,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   177,   178,     0,     3,     4,     5,     6,     7,    12,
      41,    42,    43,    48,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    62,    63,    64,    65,    69,
      70,    71,    72,    73,    75,    79,    80,    81,    82,    84,
      86,    88,    91,    95,    96,    97,    98,    99,   100,   101,
     103,   104,   105,   109,   110,   111,   113,   114,   115,   116,
     117,   118,   123,   124,   125,   126,   127,   128,   129,   136,
     141,   142,   143,   144,   145,   146,   148,   149,   151,   152,
     153,   154,   156,   160,   166,   168,   169,   171,   172,   174,
     175,   179,   182,   185,   186,   187,   188,   189,   190,   193,
     204,   205,   208,   213,   219,   275,   279,   280,   283,   284,
     286,   287,   290,   300,   301,   302,   307,   310,   326,   331,
     333,   334,   335,   336,   337,   338,   339,   340,   342,   355,
     357,   111,   123,   141,   182,   204,   279,   333,   279,   166,
     279,   279,   279,   324,   325,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   111,   166,   186,
     301,   302,   333,   333,   279,   111,   166,   186,   301,   302,
     303,   332,   338,   343,   344,   166,   276,   304,   166,   276,
     277,   279,   195,   276,   166,   166,   166,   276,   168,   279,
     182,   168,   279,    25,    51,   124,   146,   166,   182,   358,
     365,   168,   279,   169,   279,   144,   183,   184,   185,    73,
     171,   243,   244,   117,   117,    73,   204,   245,   166,   166,
     166,   166,   182,   217,   359,   166,   166,    73,    78,   137,
     138,   139,   352,   353,   144,   169,   185,   185,    95,   279,
     218,   359,   146,   275,   279,   280,   333,   191,   169,    78,
     305,   352,    78,   352,   352,    26,   144,   162,   360,   166,
       8,   168,    31,   203,   146,   216,   359,     9,    10,    11,
      25,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    49,   168,    61,    61,   169,   140,   118,   154,
     204,   219,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    59,    60,   121,   328,   329,    61,
     169,   330,   166,    61,   169,   171,   339,   166,   203,    13,
     279,    40,   182,   323,   166,   275,   333,   140,   275,   333,
     360,   140,   166,   306,   121,   328,   329,   330,   167,   279,
      26,   193,     8,   168,   193,   194,   277,   278,   279,   182,
     231,   197,   168,   168,   168,   182,   365,   365,   162,    98,
     361,   365,   360,    13,   182,   168,   191,   168,   185,     8,
     168,    90,   169,   333,     8,   168,    13,   203,     8,   168,
     333,   356,   356,   333,   167,   162,   211,   123,   333,   345,
      31,   279,   346,   347,    61,   121,   137,   353,    72,   279,
     333,    78,   137,   353,   185,   181,   168,   169,   168,   168,
     214,   291,   293,   167,   167,   167,   170,   192,   193,   205,
     208,   213,   279,   172,   174,   175,   182,   361,    31,   241,
     242,   279,   358,   166,   359,   209,   279,   279,   279,    26,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   303,   279,   341,   341,   279,   348,   349,   182,
     338,   339,   217,   218,   203,   216,    31,   145,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     169,   182,   338,   341,   279,   241,   341,   279,   345,   167,
     166,   322,     8,   312,   275,   167,   182,   167,   167,   338,
     241,   169,   182,   338,   167,   191,   235,   279,    82,    26,
     193,   229,   168,    90,    13,     8,   167,    26,   169,   232,
     365,   166,     8,    42,    43,   124,   136,   146,   186,   187,
     189,   285,   301,   307,   308,   309,   170,    90,   184,   182,
     279,   244,   308,   166,    73,     8,   167,   167,   167,   168,
     182,   364,   119,   222,   166,     8,   167,   333,   122,   167,
       8,   312,    73,    74,   182,   354,   182,    61,   170,   170,
     178,   180,   169,   161,    46,   146,   161,   295,   121,   328,
     329,   330,   170,     8,   163,   333,   167,     8,   313,    13,
     281,   206,   119,   220,   279,    26,   173,   173,   122,   170,
       8,   312,   360,   166,   212,   215,   359,   210,    63,   333,
     279,   279,   360,   166,   173,   170,   167,   173,   170,   167,
      42,    43,    69,    70,    79,   124,   136,   182,   315,   317,
     320,   321,   182,   328,   329,   330,   167,   279,   236,    66,
      67,   237,   276,   191,   278,    31,   226,   333,   308,   182,
      26,   193,   230,   168,   233,   168,   233,     8,   163,   157,
     361,   362,   365,   308,   308,   166,    78,   140,   140,   169,
     102,   200,   201,   182,   170,   282,    13,   333,   168,     8,
      90,   163,   223,   301,   169,   345,   123,   333,    13,    31,
     279,    31,   279,   173,   279,   170,   178,   246,   294,    13,
     169,   182,   338,   365,    31,   279,   308,   157,   239,   240,
     326,   327,   166,   301,   120,   221,   279,   279,   279,   166,
     241,   222,   169,   207,   220,   303,   168,   170,   166,   241,
      13,    69,    70,   182,   316,   316,   166,    78,   137,     8,
     312,   167,   322,   170,    66,    67,   238,   276,   193,   168,
      83,   168,   333,   122,   225,    13,   191,   233,    92,    93,
      94,   233,   170,   365,     8,   167,   167,   308,   311,   314,
     182,   182,   308,   350,   351,   166,   159,   239,   308,   364,
     182,     8,   246,   167,   166,   145,   279,   333,   333,   122,
     173,   170,    99,   104,   106,   107,   108,   109,   110,   111,
     112,   148,   149,   151,   170,   247,   269,   270,   271,   272,
     274,   326,   147,   160,   169,   290,   297,   147,   169,   296,
     279,   360,   166,   333,   167,     8,   313,   365,   366,   239,
     223,   169,   122,   241,   167,   169,   246,   166,   221,   306,
     166,   241,   167,   317,   318,   319,   137,   317,   276,    26,
      68,   193,   168,   278,   226,   167,   308,    89,    92,   168,
     279,    26,   168,   234,   170,   163,   157,    26,   122,   167,
       8,   312,   122,   170,     8,   312,   301,   169,   167,     8,
     301,   170,   345,   279,    31,   279,   170,   358,   224,   301,
     112,   124,   146,   152,   256,   257,   258,   301,   150,   262,
     263,   115,   166,   182,   264,   265,   248,   204,   272,   365,
       8,   168,   270,   271,    46,   279,   279,   170,   166,   241,
      26,   363,   157,   327,    31,    73,   167,   246,   279,   167,
     246,   170,   239,   169,   241,   167,   122,   167,     8,   312,
      26,   191,   168,   167,   198,   168,   168,   234,   191,   365,
     308,   308,   308,   308,    73,   191,   363,   364,   167,   168,
     333,    13,     8,   168,   169,   169,     8,   168,     3,     4,
       5,     6,     7,     9,    10,    11,    12,    49,    62,    63,
      64,    65,    66,    67,    68,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   123,   124,   125,   126,
     127,   128,   129,   141,   142,   143,   145,   154,   156,   159,
     182,   298,   299,     8,   168,   146,   150,   182,   265,   266,
     267,   168,    73,   273,   203,   249,   358,   204,   146,   292,
     170,   170,   166,   241,   167,   365,   104,   288,   366,    73,
      13,   363,   170,   170,   167,   246,   167,   317,   317,   191,
     196,    26,   193,   228,   191,   167,   122,   122,   167,   170,
     288,   308,   301,   252,   259,   307,   257,    13,    26,    43,
     260,   263,     8,    29,   167,    25,    42,    45,    13,     8,
     168,   359,   273,    13,   203,   241,   167,   166,   169,    31,
      73,    13,   308,   169,   363,   170,   122,    26,   193,   227,
     191,   308,   308,   169,   169,   170,   182,   189,   253,   254,
     255,     8,   170,   308,   299,   299,    51,   261,   266,   266,
      25,    42,    45,   308,    73,   166,   168,   308,   359,   167,
      31,    73,   289,   191,    73,    13,   308,   191,   169,   317,
     191,    87,   191,   191,   140,    90,   307,   153,    13,   250,
     166,    73,     8,   313,   170,    13,   308,   170,   191,    85,
     168,   170,   170,   182,   270,   271,   308,   239,   251,    31,
      73,   167,   308,   170,   168,   199,   155,   182,   168,   167,
     239,    73,   102,   200,   202,   224,   168,   363,   167,   166,
     168,   168,   169,   268,   363,   301,   191,   268,    73,   170,
     167,   169,   191,   170
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
#line 934 "../../../hphp/util/parser/hphp.y"
    { _p->popLabelInfo();
                                         _p->saveParseTree((yyval));;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 940 "../../../hphp/util/parser/hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 941 "../../../hphp/util/parser/hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 944 "../../../hphp/util/parser/hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num() == T_DECLARE);
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 946 "../../../hphp/util/parser/hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 947 "../../../hphp/util/parser/hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 948 "../../../hphp/util/parser/hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 949 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 950 "../../../hphp/util/parser/hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());
                                         (yyval).reset();;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 952 "../../../hphp/util/parser/hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 953 "../../../hphp/util/parser/hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 954 "../../../hphp/util/parser/hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 955 "../../../hphp/util/parser/hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 956 "../../../hphp/util/parser/hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 957 "../../../hphp/util/parser/hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 962 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 963 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 964 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 965 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 966 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 967 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 972 "../../../hphp/util/parser/hphp.y"
    { ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 973 "../../../hphp/util/parser/hphp.y"
    { ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 976 "../../../hphp/util/parser/hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 977 "../../../hphp/util/parser/hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 978 "../../../hphp/util/parser/hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 980 "../../../hphp/util/parser/hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 984 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 986 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]);;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 989 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 1;;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 990 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 0;;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 992 "../../../hphp/util/parser/hphp.y"
    { (yyval).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         (yyval) = 0;;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 996 "../../../hphp/util/parser/hphp.y"
    { if ((yyvsp[(1) - (1)]).num())
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 1002 "../../../hphp/util/parser/hphp.y"
    { if ((yyvsp[(1) - (2)]).num())
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 1008 "../../../hphp/util/parser/hphp.y"
    { if ((yyvsp[(1) - (2)]).num())
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 1014 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                          on_constant(_p,(yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 1016 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                          on_constant(_p,(yyval),  0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 1022 "../../../hphp/util/parser/hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 1023 "../../../hphp/util/parser/hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 1026 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 1027 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 1028 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 1029 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 1032 "../../../hphp/util/parser/hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 1036 "../../../hphp/util/parser/hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 1041 "../../../hphp/util/parser/hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 1042 "../../../hphp/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 1043 "../../../hphp/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 1046 "../../../hphp/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 1048 "../../../hphp/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 1051 "../../../hphp/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 1052 "../../../hphp/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 1054 "../../../hphp/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 1055 "../../../hphp/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 1057 "../../../hphp/util/parser/hphp.y"
    { _p->onBreak((yyval), NULL);;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 1058 "../../../hphp/util/parser/hphp.y"
    { _p->onBreak((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 1059 "../../../hphp/util/parser/hphp.y"
    { _p->onContinue((yyval), NULL);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 1060 "../../../hphp/util/parser/hphp.y"
    { _p->onContinue((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 1061 "../../../hphp/util/parser/hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 1062 "../../../hphp/util/parser/hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 1063 "../../../hphp/util/parser/hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 1064 "../../../hphp/util/parser/hphp.y"
    { _p->onYield((yyval), &(yyvsp[(2) - (3)]), false);;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 1065 "../../../hphp/util/parser/hphp.y"
    { on_yield_assign(_p, (yyval), (yyvsp[(1) - (5)]), &(yyvsp[(4) - (5)]));;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 1067 "../../../hphp/util/parser/hphp.y"
    { on_yield_list_assign(_p, (yyval), (yyvsp[(3) - (8)]), &(yyvsp[(7) - (8)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 1068 "../../../hphp/util/parser/hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 1069 "../../../hphp/util/parser/hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 1070 "../../../hphp/util/parser/hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 1071 "../../../hphp/util/parser/hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 1072 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 1073 "../../../hphp/util/parser/hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 1076 "../../../hphp/util/parser/hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 1077 "../../../hphp/util/parser/hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 1080 "../../../hphp/util/parser/hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 1088 "../../../hphp/util/parser/hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 1091 "../../../hphp/util/parser/hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 1092 "../../../hphp/util/parser/hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 1093 "../../../hphp/util/parser/hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 1097 "../../../hphp/util/parser/hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 1098 "../../../hphp/util/parser/hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 1110 "../../../hphp/util/parser/hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 1111 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 1115 "../../../hphp/util/parser/hphp.y"
    { finally_statement(_p);;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 1117 "../../../hphp/util/parser/hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (5)]));;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 1122 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 1126 "../../../hphp/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 1127 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 1131 "../../../hphp/util/parser/hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 1136 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1141 "../../../hphp/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onFunction((yyval),0,t,(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1146 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1151 "../../../hphp/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onFunction((yyval),0,t,(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1159 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1162 "../../../hphp/util/parser/hphp.y"
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

  case 96:

/* Line 1455 of yacc.c  */
#line 1177 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1180 "../../../hphp/util/parser/hphp.y"
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

  case 98:

/* Line 1455 of yacc.c  */
#line 1194 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1197 "../../../hphp/util/parser/hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1202 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1205 "../../../hphp/util/parser/hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1212 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1214 "../../../hphp/util/parser/hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (6)]),t_ext,t_imp,
                                                     (yyvsp[(5) - (6)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1222 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1224 "../../../hphp/util/parser/hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (7)]),t_ext,t_imp,
                                                     (yyvsp[(6) - (7)]), &(yyvsp[(1) - (7)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1232 "../../../hphp/util/parser/hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1233 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1237 "../../../hphp/util/parser/hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1240 "../../../hphp/util/parser/hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1243 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1244 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1245 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1249 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1250 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1253 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1254 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1257 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1258 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1261 "../../../hphp/util/parser/hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1263 "../../../hphp/util/parser/hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1266 "../../../hphp/util/parser/hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1268 "../../../hphp/util/parser/hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1272 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1273 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1276 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1277 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1281 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1283 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1286 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1288 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1291 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1293 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1296 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1298 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1308 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1309 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1310 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1311 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1316 "../../../hphp/util/parser/hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1318 "../../../hphp/util/parser/hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1319 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1322 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1323 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1328 "../../../hphp/util/parser/hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1329 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1334 "../../../hphp/util/parser/hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1335 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1338 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1339 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1342 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1343 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1348 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1350 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1351 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1352 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1357 "../../../hphp/util/parser/hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,NULL,&(yyvsp[(1) - (3)]));;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1359 "../../../hphp/util/parser/hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,NULL,&(yyvsp[(1) - (4)]));;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1362 "../../../hphp/util/parser/hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,&(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1365 "../../../hphp/util/parser/hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,&(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1368 "../../../hphp/util/parser/hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,NULL,&(yyvsp[(3) - (5)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1371 "../../../hphp/util/parser/hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,NULL,&(yyvsp[(3) - (6)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1375 "../../../hphp/util/parser/hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,&(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1379 "../../../hphp/util/parser/hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,&(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1384 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1385 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1388 "../../../hphp/util/parser/hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1389 "../../../hphp/util/parser/hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1391 "../../../hphp/util/parser/hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1393 "../../../hphp/util/parser/hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1397 "../../../hphp/util/parser/hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1398 "../../../hphp/util/parser/hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1401 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1402 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1403 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1407 "../../../hphp/util/parser/hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1409 "../../../hphp/util/parser/hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1410 "../../../hphp/util/parser/hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1411 "../../../hphp/util/parser/hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1416 "../../../hphp/util/parser/hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1417 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1420 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1421 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1424 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1425 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1427 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1431 "../../../hphp/util/parser/hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1436 "../../../hphp/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onMethod((yyval),(yyvsp[(1) - (10)]),t,(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1443 "../../../hphp/util/parser/hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1448 "../../../hphp/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onMethod((yyval),(yyvsp[(2) - (11)]),t,(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1453 "../../../hphp/util/parser/hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1455 "../../../hphp/util/parser/hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1457 "../../../hphp/util/parser/hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1458 "../../../hphp/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1461 "../../../hphp/util/parser/hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1464 "../../../hphp/util/parser/hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1465 "../../../hphp/util/parser/hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1466 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1472 "../../../hphp/util/parser/hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1476 "../../../hphp/util/parser/hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1479 "../../../hphp/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1486 "../../../hphp/util/parser/hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1487 "../../../hphp/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1492 "../../../hphp/util/parser/hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1495 "../../../hphp/util/parser/hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1502 "../../../hphp/util/parser/hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1504 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1508 "../../../hphp/util/parser/hphp.y"
    { (yyval) = 4;;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1509 "../../../hphp/util/parser/hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1515 "../../../hphp/util/parser/hphp.y"
    { (yyval) = 6;;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1517 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1521 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1523 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1527 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1528 "../../../hphp/util/parser/hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1532 "../../../hphp/util/parser/hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1533 "../../../hphp/util/parser/hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1537 "../../../hphp/util/parser/hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1540 "../../../hphp/util/parser/hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1545 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1550 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1551 "../../../hphp/util/parser/hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1553 "../../../hphp/util/parser/hphp.y"
    { (yyval) = 0;;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1557 "../../../hphp/util/parser/hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1558 "../../../hphp/util/parser/hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1559 "../../../hphp/util/parser/hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1560 "../../../hphp/util/parser/hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1564 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1565 "../../../hphp/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1566 "../../../hphp/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1567 "../../../hphp/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1568 "../../../hphp/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1570 "../../../hphp/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1572 "../../../hphp/util/parser/hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1576 "../../../hphp/util/parser/hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1579 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1580 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1584 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1585 "../../../hphp/util/parser/hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1588 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1589 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1592 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1593 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1596 "../../../hphp/util/parser/hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1598 "../../../hphp/util/parser/hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1601 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1602 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1603 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1604 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1605 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1606 "../../../hphp/util/parser/hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1610 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1612 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1613 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1614 "../../../hphp/util/parser/hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1618 "../../../hphp/util/parser/hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1619 "../../../hphp/util/parser/hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1624 "../../../hphp/util/parser/hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1625 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1629 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1633 "../../../hphp/util/parser/hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1634 "../../../hphp/util/parser/hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1638 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1639 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1643 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1644 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1645 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1649 "../../../hphp/util/parser/hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1650 "../../../hphp/util/parser/hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1651 "../../../hphp/util/parser/hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1654 "../../../hphp/util/parser/hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1655 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1656 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1657 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1658 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1659 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1660 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1661 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1662 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1663 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1664 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1665 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1666 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1667 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1668 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1669 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1670 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1671 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1672 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1673 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1674 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1675 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1676 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1677 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1678 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1679 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1680 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1681 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1682 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1683 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1684 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1685 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1686 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1687 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1688 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1689 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1690 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1691 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1692 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1693 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1694 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1695 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1696 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1698 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1699 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1702 "../../../hphp/util/parser/hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1703 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1704 "../../../hphp/util/parser/hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1705 "../../../hphp/util/parser/hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1706 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1707 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1708 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1709 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1710 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1711 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1712 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1713 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1714 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1715 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1716 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1717 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1718 "../../../hphp/util/parser/hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1719 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1721 "../../../hphp/util/parser/hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1725 "../../../hphp/util/parser/hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1729 "../../../hphp/util/parser/hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1733 "../../../hphp/util/parser/hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),1);
                                         _p->popLabelInfo();;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1736 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1737 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1738 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1742 "../../../hphp/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1747 "../../../hphp/util/parser/hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1754 "../../../hphp/util/parser/hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1761 "../../../hphp/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1763 "../../../hphp/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1767 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1768 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1769 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1776 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1777 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1781 "../../../hphp/util/parser/hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1782 "../../../hphp/util/parser/hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1783 "../../../hphp/util/parser/hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1784 "../../../hphp/util/parser/hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1791 "../../../hphp/util/parser/hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1794 "../../../hphp/util/parser/hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         (yyval).setText("");;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1801 "../../../hphp/util/parser/hphp.y"
    { _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1808 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1809 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1814 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1815 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1818 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1819 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1822 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1826 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1829 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1832 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1839 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1840 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1844 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1846 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1848 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1851 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1852 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1853 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1854 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1855 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1856 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1857 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1858 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1859 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1860 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1861 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1862 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1863 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1864 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1865 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1866 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1867 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1868 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1869 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1870 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1871 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1872 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1873 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1874 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1875 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1876 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1877 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1878 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1879 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1880 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1881 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1882 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1883 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1884 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1885 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1886 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1887 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1888 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1889 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1890 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1891 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1892 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1893 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1894 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1895 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1896 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1897 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1898 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1899 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1900 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1901 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1902 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1903 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1904 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1905 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1906 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1907 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1908 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1909 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1910 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1911 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1912 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1913 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1914 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1915 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1916 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1917 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1918 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1919 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1920 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1921 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1922 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1923 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1928 "../../../hphp/util/parser/hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1932 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1933 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1936 "../../../hphp/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1937 "../../../hphp/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1938 "../../../hphp/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1942 "../../../hphp/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1943 "../../../hphp/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1944 "../../../hphp/util/parser/hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1948 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1949 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1950 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1954 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1955 "../../../hphp/util/parser/hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1956 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1960 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1961 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1965 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1966 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1967 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1969 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1970 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1971 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1972 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1973 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1974 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1975 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1976 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1979 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1981 "../../../hphp/util/parser/hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1985 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1986 "../../../hphp/util/parser/hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1987 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1988 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1990 "../../../hphp/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1991 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1992 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1997 "../../../hphp/util/parser/hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1999 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2003 "../../../hphp/util/parser/hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2004 "../../../hphp/util/parser/hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2005 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2006 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2007 "../../../hphp/util/parser/hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2008 "../../../hphp/util/parser/hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2010 "../../../hphp/util/parser/hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2015 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2016 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2019 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2020 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2023 "../../../hphp/util/parser/hphp.y"
    { only_in_hphp_syntax(_p); (yyval).reset();;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2024 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2029 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2031 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2033 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2034 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2038 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2039 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2040 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2044 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2046 "../../../hphp/util/parser/hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2049 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2050 "../../../hphp/util/parser/hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2051 "../../../hphp/util/parser/hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2054 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2055 "../../../hphp/util/parser/hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2056 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2057 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2059 "../../../hphp/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2063 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2064 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2069 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2071 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2073 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2074 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2078 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2079 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2083 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2084 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2087 "../../../hphp/util/parser/hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2088 "../../../hphp/util/parser/hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2094 "../../../hphp/util/parser/hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2096 "../../../hphp/util/parser/hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2099 "../../../hphp/util/parser/hphp.y"
    { user_attribute_check(_p);;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2101 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2104 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2107 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2108 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2112 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2114 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2118 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2119 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2123 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2124 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2128 "../../../hphp/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2129 "../../../hphp/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2134 "../../../hphp/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2135 "../../../hphp/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2139 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2140 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2141 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2142 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2143 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2144 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2145 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2148 "../../../hphp/util/parser/hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2150 "../../../hphp/util/parser/hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2151 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2155 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2156 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2157 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2158 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2160 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2162 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2164 "../../../hphp/util/parser/hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2165 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2169 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2170 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2171 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2177 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2180 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2183 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2186 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2189 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2192 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2199 "../../../hphp/util/parser/hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2203 "../../../hphp/util/parser/hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2207 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2209 "../../../hphp/util/parser/hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2214 "../../../hphp/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2215 "../../../hphp/util/parser/hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2216 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2219 "../../../hphp/util/parser/hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2220 "../../../hphp/util/parser/hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2223 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2224 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2228 "../../../hphp/util/parser/hphp.y"
    { (yyval) = 1;;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2229 "../../../hphp/util/parser/hphp.y"
    { (yyval)++;;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2233 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2234 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2235 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2236 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2239 "../../../hphp/util/parser/hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2240 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2244 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2246 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2248 "../../../hphp/util/parser/hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2249 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2253 "../../../hphp/util/parser/hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2254 "../../../hphp/util/parser/hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2256 "../../../hphp/util/parser/hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2257 "../../../hphp/util/parser/hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2258 "../../../hphp/util/parser/hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2259 "../../../hphp/util/parser/hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2264 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2265 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset();;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2269 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2270 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2271 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2272 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2275 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2277 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2278 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2279 "../../../hphp/util/parser/hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2284 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2285 "../../../hphp/util/parser/hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2289 "../../../hphp/util/parser/hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2290 "../../../hphp/util/parser/hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2291 "../../../hphp/util/parser/hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2292 "../../../hphp/util/parser/hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2297 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2298 "../../../hphp/util/parser/hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2303 "../../../hphp/util/parser/hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2305 "../../../hphp/util/parser/hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2307 "../../../hphp/util/parser/hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2308 "../../../hphp/util/parser/hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2312 "../../../hphp/util/parser/hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2314 "../../../hphp/util/parser/hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2315 "../../../hphp/util/parser/hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2317 "../../../hphp/util/parser/hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2322 "../../../hphp/util/parser/hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2324 "../../../hphp/util/parser/hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2326 "../../../hphp/util/parser/hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2328 "../../../hphp/util/parser/hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2330 "../../../hphp/util/parser/hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2331 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2334 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2335 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2336 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2340 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2341 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2342 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 2343 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2344 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2345 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2346 "../../../hphp/util/parser/hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2350 "../../../hphp/util/parser/hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2351 "../../../hphp/util/parser/hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2356 "../../../hphp/util/parser/hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2364 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2365 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2371 "../../../hphp/util/parser/hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2375 "../../../hphp/util/parser/hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]);
                                         only_in_strict_mode(_p); ;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2382 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2383 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2392 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2393 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2394 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2395 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2399 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2400 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2404 "../../../hphp/util/parser/hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2405 "../../../hphp/util/parser/hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2407 "../../../hphp/util/parser/hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (5)]).text()); ;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2408 "../../../hphp/util/parser/hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2416 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2417 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2418 "../../../hphp/util/parser/hphp.y"
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

  case 668:

/* Line 1455 of yacc.c  */
#line 2429 "../../../hphp/util/parser/hphp.y"
    { (yyval).setText("array"); ;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2431 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p);
                                         (yyval).setText("array"); ;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2434 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p);
                                         (yyval).setText("array"); ;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2436 "../../../hphp/util/parser/hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2439 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2440 "../../../hphp/util/parser/hphp.y"
    { only_in_strict_mode(_p); (yyval).reset(); ;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2444 "../../../hphp/util/parser/hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2445 "../../../hphp/util/parser/hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10247 "hphp.tab.cpp"
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
#line 2448 "../../../hphp/util/parser/hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

