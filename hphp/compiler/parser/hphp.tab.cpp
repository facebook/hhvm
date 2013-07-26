
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

#ifdef XHPAST2_PARSER
#include "hphp/util/parser/xhpast2/parser.h"
#elif TEST_PARSER
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
// continuation transformations

void prepare_generator(Parser *_p, Token &stmt) {
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
#line 753 "new_hphp.tab.cpp"

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
#line 960 "new_hphp.tab.cpp"

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
#define YYLAST   10903

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  182
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  213
/* YYNRULES -- Number of rules.  */
#define YYNRULES  735
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1372

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
     556,   561,   567,   575,   582,   589,   597,   607,   616,   620,
     623,   625,   626,   630,   635,   642,   648,   654,   661,   670,
     678,   681,   682,   684,   687,   691,   696,   700,   702,   704,
     707,   712,   716,   722,   724,   728,   731,   732,   733,   738,
     739,   745,   748,   749,   760,   761,   773,   777,   781,   785,
     789,   795,   798,   801,   802,   809,   815,   820,   824,   826,
     828,   832,   837,   839,   841,   843,   845,   850,   852,   856,
     859,   860,   863,   864,   866,   870,   872,   874,   876,   878,
     882,   887,   892,   897,   899,   901,   904,   907,   910,   914,
     918,   920,   922,   924,   926,   930,   932,   934,   936,   937,
     939,   942,   944,   946,   948,   950,   952,   954,   956,   957,
     959,   961,   963,   967,   973,   975,   979,   985,   990,   994,
     998,  1001,  1003,  1007,  1011,  1013,  1015,  1016,  1019,  1024,
    1028,  1035,  1037,  1039,  1041,  1048,  1052,  1057,  1064,  1068,
    1072,  1076,  1080,  1084,  1088,  1092,  1096,  1100,  1104,  1108,
    1111,  1114,  1117,  1120,  1124,  1128,  1132,  1136,  1140,  1144,
    1148,  1152,  1156,  1160,  1164,  1168,  1172,  1176,  1180,  1184,
    1187,  1190,  1193,  1196,  1200,  1204,  1208,  1212,  1216,  1220,
    1224,  1228,  1232,  1236,  1242,  1247,  1249,  1252,  1255,  1258,
    1261,  1264,  1267,  1270,  1273,  1276,  1278,  1280,  1282,  1286,
    1289,  1290,  1302,  1303,  1316,  1318,  1320,  1322,  1328,  1332,
    1338,  1342,  1345,  1346,  1349,  1350,  1355,  1360,  1364,  1369,
    1374,  1379,  1384,  1386,  1388,  1392,  1398,  1399,  1403,  1408,
    1410,  1413,  1418,  1421,  1428,  1429,  1431,  1436,  1437,  1440,
    1441,  1443,  1445,  1449,  1451,  1455,  1457,  1459,  1463,  1467,
    1469,  1471,  1473,  1475,  1477,  1479,  1481,  1483,  1485,  1487,
    1489,  1491,  1493,  1495,  1497,  1499,  1501,  1503,  1505,  1507,
    1509,  1511,  1513,  1515,  1517,  1519,  1521,  1523,  1525,  1527,
    1529,  1531,  1533,  1535,  1537,  1539,  1541,  1543,  1545,  1547,
    1549,  1551,  1553,  1555,  1557,  1559,  1561,  1563,  1565,  1567,
    1569,  1571,  1573,  1575,  1577,  1579,  1581,  1583,  1585,  1587,
    1589,  1591,  1593,  1595,  1597,  1599,  1601,  1603,  1605,  1607,
    1609,  1611,  1613,  1615,  1617,  1619,  1621,  1623,  1628,  1630,
    1632,  1634,  1636,  1638,  1640,  1642,  1644,  1647,  1649,  1650,
    1651,  1653,  1655,  1659,  1660,  1662,  1664,  1666,  1668,  1670,
    1672,  1674,  1676,  1678,  1680,  1682,  1684,  1688,  1691,  1693,
    1695,  1698,  1701,  1706,  1710,  1715,  1717,  1719,  1723,  1727,
    1729,  1731,  1733,  1735,  1739,  1743,  1747,  1750,  1751,  1753,
    1754,  1756,  1757,  1763,  1767,  1771,  1773,  1775,  1777,  1779,
    1783,  1786,  1788,  1790,  1792,  1794,  1796,  1799,  1802,  1807,
    1811,  1816,  1819,  1820,  1826,  1830,  1834,  1836,  1840,  1842,
    1845,  1846,  1852,  1856,  1859,  1860,  1864,  1865,  1870,  1873,
    1874,  1878,  1882,  1884,  1885,  1887,  1890,  1893,  1898,  1902,
    1906,  1909,  1914,  1917,  1922,  1924,  1926,  1928,  1930,  1932,
    1935,  1940,  1944,  1949,  1953,  1955,  1957,  1959,  1961,  1964,
    1969,  1974,  1978,  1980,  1982,  1986,  1994,  2001,  2010,  2020,
    2029,  2040,  2048,  2055,  2057,  2060,  2065,  2070,  2072,  2074,
    2079,  2081,  2082,  2084,  2087,  2089,  2091,  2094,  2099,  2103,
    2107,  2108,  2110,  2113,  2118,  2122,  2125,  2129,  2136,  2137,
    2139,  2144,  2147,  2148,  2154,  2158,  2162,  2164,  2171,  2176,
    2181,  2184,  2187,  2188,  2194,  2198,  2202,  2204,  2207,  2208,
    2214,  2218,  2222,  2224,  2227,  2230,  2232,  2235,  2237,  2242,
    2246,  2250,  2257,  2261,  2263,  2265,  2267,  2272,  2277,  2280,
    2283,  2288,  2291,  2294,  2296,  2300,  2304,  2305,  2308,  2314,
    2321,  2323,  2326,  2328,  2333,  2337,  2338,  2340,  2344,  2348,
    2350,  2352,  2353,  2354,  2357,  2361,  2363,  2369,  2373,  2377,
    2381,  2383,  2386,  2387,  2392,  2395,  2398,  2400,  2402,  2404,
    2409,  2416,  2418,  2427,  2433,  2435
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     183,     0,    -1,    -1,   184,   185,    -1,   185,   186,    -1,
      -1,   200,    -1,   212,    -1,   215,    -1,   220,    -1,   381,
      -1,   116,   172,   173,   174,    -1,   141,   192,   174,    -1,
      -1,   141,   192,   175,   187,   185,   176,    -1,    -1,   141,
     175,   188,   185,   176,    -1,   104,   190,   174,    -1,   197,
     174,    -1,    71,    -1,   148,    -1,   149,    -1,   151,    -1,
     153,    -1,   152,    -1,   190,     8,   191,    -1,   191,    -1,
     192,    -1,   144,   192,    -1,   192,    90,   189,    -1,   144,
     192,    90,   189,    -1,   189,    -1,   192,   144,   189,    -1,
     192,    -1,   141,   144,   192,    -1,   144,   192,    -1,   193,
      -1,   193,   384,    -1,   193,   384,    -1,   197,     8,   382,
      13,   328,    -1,    99,   382,    13,   328,    -1,   198,   199,
      -1,    -1,   200,    -1,   212,    -1,   215,    -1,   220,    -1,
     175,   198,   176,    -1,    65,   287,   200,   242,   244,    -1,
      65,   287,    26,   198,   243,   245,    68,   174,    -1,    -1,
      82,   287,   201,   236,    -1,    -1,    81,   202,   200,    82,
     287,   174,    -1,    -1,    84,   172,   289,   174,   289,   174,
     289,   173,   203,   234,    -1,    -1,    91,   287,   204,   239,
      -1,    95,   174,    -1,    95,   293,   174,    -1,    97,   174,
      -1,    97,   293,   174,    -1,   100,   174,    -1,   100,   293,
     174,    -1,   145,    95,   174,    -1,   105,   252,   174,    -1,
     111,   254,   174,    -1,    80,   288,   174,    -1,   113,   172,
     378,   173,   174,    -1,   174,    -1,    75,    -1,    -1,    86,
     172,   293,    90,   233,   232,   173,   205,   235,    -1,    88,
     172,   238,   173,   237,    -1,   101,   175,   198,   176,   102,
     172,   321,    73,   173,   175,   198,   176,   206,   209,    -1,
     101,   175,   198,   176,   207,    -1,   103,   293,   174,    -1,
      96,   189,   174,    -1,   293,   174,    -1,   290,   174,    -1,
     291,   174,    -1,   292,   174,    -1,   189,    26,    -1,   206,
     102,   172,   321,    73,   173,   175,   198,   176,    -1,    -1,
      -1,   208,   159,   175,   198,   176,    -1,   207,    -1,    -1,
      31,    -1,    -1,    98,    -1,    -1,   211,   210,   383,   213,
     172,   248,   173,   387,   175,   198,   176,    -1,    -1,   348,
     211,   210,   383,   214,   172,   248,   173,   387,   175,   198,
     176,    -1,    -1,   226,   223,   216,   227,   228,   175,   255,
     176,    -1,    -1,   348,   226,   223,   217,   227,   228,   175,
     255,   176,    -1,    -1,   118,   224,   218,   229,   175,   255,
     176,    -1,    -1,   348,   118,   224,   219,   229,   175,   255,
     176,    -1,    -1,   154,   225,   221,   228,   175,   255,   176,
      -1,    -1,   348,   154,   225,   222,   228,   175,   255,   176,
      -1,   383,    -1,   146,    -1,   383,    -1,   383,    -1,   117,
      -1,   110,   117,    -1,   109,   117,    -1,   119,   321,    -1,
      -1,   120,   230,    -1,    -1,   119,   230,    -1,    -1,   321,
      -1,   230,     8,   321,    -1,   321,    -1,   231,     8,   321,
      -1,   122,   233,    -1,    -1,   355,    -1,    31,   355,    -1,
     200,    -1,    26,   198,    85,   174,    -1,   200,    -1,    26,
     198,    87,   174,    -1,   200,    -1,    26,   198,    83,   174,
      -1,   200,    -1,    26,   198,    89,   174,    -1,   189,    13,
     328,    -1,   238,     8,   189,    13,   328,    -1,   175,   240,
     176,    -1,   175,   174,   240,   176,    -1,    26,   240,    92,
     174,    -1,    26,   174,   240,    92,   174,    -1,   240,    93,
     293,   241,   198,    -1,   240,    94,   241,   198,    -1,    -1,
      26,    -1,   174,    -1,   242,    66,   287,   200,    -1,    -1,
     243,    66,   287,    26,   198,    -1,    -1,    67,   200,    -1,
      -1,    67,    26,   198,    -1,    -1,   247,     8,   157,    -1,
     247,   333,    -1,   157,    -1,    -1,   349,   282,   394,    73,
      -1,   349,   282,   394,    31,    73,    -1,   349,   282,   394,
      31,    73,    13,   328,    -1,   349,   282,   394,    73,    13,
     328,    -1,   247,     8,   282,   349,   394,    73,    -1,   247,
       8,   282,   349,   394,    31,    73,    -1,   247,     8,   282,
     349,   394,    31,    73,    13,   328,    -1,   247,     8,   282,
     349,   394,    73,    13,   328,    -1,   249,     8,   157,    -1,
     249,   333,    -1,   157,    -1,    -1,   349,   394,    73,    -1,
     349,   394,    31,    73,    -1,   349,   394,    31,    73,    13,
     328,    -1,   349,   394,    73,    13,   328,    -1,   249,     8,
     349,   394,    73,    -1,   249,     8,   349,   394,    31,    73,
      -1,   249,     8,   349,   394,    31,    73,    13,   328,    -1,
     249,     8,   349,   394,    73,    13,   328,    -1,   251,   333,
      -1,    -1,   293,    -1,    31,   355,    -1,   251,     8,   293,
      -1,   251,     8,    31,   355,    -1,   252,     8,   253,    -1,
     253,    -1,    73,    -1,   177,   355,    -1,   177,   175,   293,
     176,    -1,   254,     8,    73,    -1,   254,     8,    73,    13,
     328,    -1,    73,    -1,    73,    13,   328,    -1,   255,   256,
      -1,    -1,    -1,   278,   257,   284,   174,    -1,    -1,   280,
     393,   258,   284,   174,    -1,   285,   174,    -1,    -1,   279,
     211,   210,   383,   172,   259,   246,   173,   387,   277,    -1,
      -1,   348,   279,   211,   210,   383,   172,   260,   246,   173,
     387,   277,    -1,   148,   265,   174,    -1,   149,   271,   174,
      -1,   151,   273,   174,    -1,   104,   231,   174,    -1,   104,
     231,   175,   261,   176,    -1,   261,   262,    -1,   261,   263,
      -1,    -1,   196,   140,   189,   155,   231,   174,    -1,   264,
      90,   279,   189,   174,    -1,   264,    90,   280,   174,    -1,
     196,   140,   189,    -1,   189,    -1,   266,    -1,   265,     8,
     266,    -1,   267,   318,   269,   270,    -1,   146,    -1,   124,
      -1,   321,    -1,   112,    -1,   152,   175,   268,   176,    -1,
     327,    -1,   268,     8,   327,    -1,    13,   328,    -1,    -1,
      51,   153,    -1,    -1,   272,    -1,   271,     8,   272,    -1,
     150,    -1,   274,    -1,   189,    -1,   115,    -1,   172,   275,
     173,    -1,   172,   275,   173,    45,    -1,   172,   275,   173,
      25,    -1,   172,   275,   173,    42,    -1,   274,    -1,   276,
      -1,   276,    45,    -1,   276,    25,    -1,   276,    42,    -1,
     275,     8,   275,    -1,   275,    29,   275,    -1,   189,    -1,
     146,    -1,   150,    -1,   174,    -1,   175,   198,   176,    -1,
     280,    -1,   112,    -1,   280,    -1,    -1,   281,    -1,   280,
     281,    -1,   106,    -1,   107,    -1,   108,    -1,   111,    -1,
     110,    -1,   109,    -1,   283,    -1,    -1,   106,    -1,   107,
      -1,   108,    -1,   284,     8,    73,    -1,   284,     8,    73,
      13,   328,    -1,    73,    -1,    73,    13,   328,    -1,   285,
       8,   382,    13,   328,    -1,    99,   382,    13,   328,    -1,
     172,   286,   173,    -1,    63,   323,   326,    -1,    62,   293,
      -1,   310,    -1,   172,   293,   173,    -1,   288,     8,   293,
      -1,   293,    -1,   288,    -1,    -1,   145,   293,    -1,   145,
     293,   122,   293,    -1,   355,    13,   290,    -1,   123,   172,
     367,   173,    13,   290,    -1,   294,    -1,   355,    -1,   286,
      -1,   123,   172,   367,   173,    13,   293,    -1,   355,    13,
     293,    -1,   355,    13,    31,   355,    -1,   355,    13,    31,
      63,   323,   326,    -1,   355,    24,   293,    -1,   355,    23,
     293,    -1,   355,    22,   293,    -1,   355,    21,   293,    -1,
     355,    20,   293,    -1,   355,    19,   293,    -1,   355,    18,
     293,    -1,   355,    17,   293,    -1,   355,    16,   293,    -1,
     355,    15,   293,    -1,   355,    14,   293,    -1,   355,    60,
      -1,    60,   355,    -1,   355,    59,    -1,    59,   355,    -1,
     293,    27,   293,    -1,   293,    28,   293,    -1,   293,     9,
     293,    -1,   293,    11,   293,    -1,   293,    10,   293,    -1,
     293,    29,   293,    -1,   293,    31,   293,    -1,   293,    30,
     293,    -1,   293,    44,   293,    -1,   293,    42,   293,    -1,
     293,    43,   293,    -1,   293,    45,   293,    -1,   293,    46,
     293,    -1,   293,    47,   293,    -1,   293,    41,   293,    -1,
     293,    40,   293,    -1,    42,   293,    -1,    43,   293,    -1,
      48,   293,    -1,    50,   293,    -1,   293,    33,   293,    -1,
     293,    32,   293,    -1,   293,    35,   293,    -1,   293,    34,
     293,    -1,   293,    36,   293,    -1,   293,    39,   293,    -1,
     293,    37,   293,    -1,   293,    38,   293,    -1,   293,    49,
     323,    -1,   172,   294,   173,    -1,   293,    25,   293,    26,
     293,    -1,   293,    25,    26,   293,    -1,   377,    -1,    58,
     293,    -1,    57,   293,    -1,    56,   293,    -1,    55,   293,
      -1,    54,   293,    -1,    53,   293,    -1,    52,   293,    -1,
      64,   324,    -1,    51,   293,    -1,   330,    -1,   303,    -1,
     302,    -1,   178,   325,   178,    -1,    12,   293,    -1,    -1,
     211,   210,   172,   295,   248,   173,   387,   308,   175,   198,
     176,    -1,    -1,   111,   211,   210,   172,   296,   248,   173,
     387,   308,   175,   198,   176,    -1,   306,    -1,   304,    -1,
      79,    -1,   298,     8,   297,   122,   293,    -1,   297,   122,
     293,    -1,   299,     8,   297,   122,   328,    -1,   297,   122,
     328,    -1,   298,   332,    -1,    -1,   299,   332,    -1,    -1,
     166,   172,   300,   173,    -1,   124,   172,   368,   173,    -1,
      61,   368,   179,    -1,   321,   175,   370,   176,    -1,   321,
     175,   372,   176,    -1,   306,    61,   363,   179,    -1,   307,
      61,   363,   179,    -1,   303,    -1,   379,    -1,   172,   294,
     173,    -1,   104,   172,   309,   333,   173,    -1,    -1,   309,
       8,    73,    -1,   309,     8,    31,    73,    -1,    73,    -1,
      31,    73,    -1,   160,   146,   311,   161,    -1,   313,    46,
      -1,   313,   161,   314,   160,    46,   312,    -1,    -1,   146,
      -1,   313,   315,    13,   316,    -1,    -1,   314,   317,    -1,
      -1,   146,    -1,   147,    -1,   175,   293,   176,    -1,   147,
      -1,   175,   293,   176,    -1,   310,    -1,   319,    -1,   318,
      26,   319,    -1,   318,    43,   319,    -1,   189,    -1,    64,
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
      -1,   128,    -1,   129,    -1,   143,    -1,   142,    -1,   171,
      -1,   154,    -1,   156,    -1,   155,    -1,   167,    -1,   169,
      -1,   166,    -1,   195,   172,   250,   173,    -1,   196,    -1,
     146,    -1,   321,    -1,   111,    -1,   361,    -1,   321,    -1,
     111,    -1,   365,    -1,   172,   173,    -1,   287,    -1,    -1,
      -1,    78,    -1,   374,    -1,   172,   250,   173,    -1,    -1,
      69,    -1,    70,    -1,    79,    -1,   128,    -1,   129,    -1,
     143,    -1,   125,    -1,   156,    -1,   126,    -1,   127,    -1,
     142,    -1,   171,    -1,   136,    78,   137,    -1,   136,   137,
      -1,   327,    -1,   194,    -1,    42,   328,    -1,    43,   328,
      -1,   124,   172,   331,   173,    -1,    61,   331,   179,    -1,
     166,   172,   301,   173,    -1,   329,    -1,   305,    -1,   196,
     140,   189,    -1,   146,   140,   189,    -1,   194,    -1,    72,
      -1,   379,    -1,   327,    -1,   180,   374,   180,    -1,   181,
     374,   181,    -1,   136,   374,   137,    -1,   334,   332,    -1,
      -1,     8,    -1,    -1,     8,    -1,    -1,   334,     8,   328,
     122,   328,    -1,   334,     8,   328,    -1,   328,   122,   328,
      -1,   328,    -1,    69,    -1,    70,    -1,    79,    -1,   136,
      78,   137,    -1,   136,   137,    -1,    69,    -1,    70,    -1,
     189,    -1,   335,    -1,   189,    -1,    42,   336,    -1,    43,
     336,    -1,   124,   172,   338,   173,    -1,    61,   338,   179,
      -1,   166,   172,   341,   173,    -1,   339,   332,    -1,    -1,
     339,     8,   337,   122,   337,    -1,   339,     8,   337,    -1,
     337,   122,   337,    -1,   337,    -1,   340,     8,   337,    -1,
     337,    -1,   342,   332,    -1,    -1,   342,     8,   297,   122,
     337,    -1,   297,   122,   337,    -1,   340,   332,    -1,    -1,
     172,   343,   173,    -1,    -1,   345,     8,   189,   344,    -1,
     189,   344,    -1,    -1,   347,   345,   332,    -1,    41,   346,
      40,    -1,   348,    -1,    -1,   351,    -1,   121,   360,    -1,
     121,   189,    -1,   121,   175,   293,   176,    -1,    61,   363,
     179,    -1,   175,   293,   176,    -1,   356,   352,    -1,   172,
     286,   173,   352,    -1,   366,   352,    -1,   172,   286,   173,
     352,    -1,   360,    -1,   320,    -1,   358,    -1,   359,    -1,
     353,    -1,   355,   350,    -1,   172,   286,   173,   350,    -1,
     322,   140,   360,    -1,   357,   172,   250,   173,    -1,   172,
     355,   173,    -1,   320,    -1,   358,    -1,   359,    -1,   353,
      -1,   355,   351,    -1,   172,   286,   173,   351,    -1,   357,
     172,   250,   173,    -1,   172,   355,   173,    -1,   360,    -1,
     353,    -1,   172,   355,   173,    -1,   355,   121,   189,   384,
     172,   250,   173,    -1,   355,   121,   360,   172,   250,   173,
      -1,   355,   121,   175,   293,   176,   172,   250,   173,    -1,
     172,   286,   173,   121,   189,   384,   172,   250,   173,    -1,
     172,   286,   173,   121,   360,   172,   250,   173,    -1,   172,
     286,   173,   121,   175,   293,   176,   172,   250,   173,    -1,
     322,   140,   189,   384,   172,   250,   173,    -1,   322,   140,
     360,   172,   250,   173,    -1,   361,    -1,   364,   361,    -1,
     361,    61,   363,   179,    -1,   361,   175,   293,   176,    -1,
     362,    -1,    73,    -1,   177,   175,   293,   176,    -1,   293,
      -1,    -1,   177,    -1,   364,   177,    -1,   360,    -1,   354,
      -1,   365,   350,    -1,   172,   286,   173,   350,    -1,   322,
     140,   360,    -1,   172,   355,   173,    -1,    -1,   354,    -1,
     365,   351,    -1,   172,   286,   173,   351,    -1,   172,   355,
     173,    -1,   367,     8,    -1,   367,     8,   355,    -1,   367,
       8,   123,   172,   367,   173,    -1,    -1,   355,    -1,   123,
     172,   367,   173,    -1,   369,   332,    -1,    -1,   369,     8,
     293,   122,   293,    -1,   369,     8,   293,    -1,   293,   122,
     293,    -1,   293,    -1,   369,     8,   293,   122,    31,   355,
      -1,   369,     8,    31,   355,    -1,   293,   122,    31,   355,
      -1,    31,   355,    -1,   371,   332,    -1,    -1,   371,     8,
     293,   122,   293,    -1,   371,     8,   293,    -1,   293,   122,
     293,    -1,   293,    -1,   373,   332,    -1,    -1,   373,     8,
     328,   122,   328,    -1,   373,     8,   328,    -1,   328,   122,
     328,    -1,   328,    -1,   374,   375,    -1,   374,    78,    -1,
     375,    -1,    78,   375,    -1,    73,    -1,    73,    61,   376,
     179,    -1,    73,   121,   189,    -1,   138,   293,   176,    -1,
     138,    72,    61,   293,   179,   176,    -1,   139,   355,   176,
      -1,   189,    -1,    74,    -1,    73,    -1,   114,   172,   378,
     173,    -1,   115,   172,   355,   173,    -1,     7,   293,    -1,
       6,   293,    -1,     5,   172,   293,   173,    -1,     4,   293,
      -1,     3,   293,    -1,   355,    -1,   378,     8,   355,    -1,
     322,   140,   189,    -1,    -1,    90,   393,    -1,   167,   383,
      13,   393,   174,    -1,   169,   383,   380,    13,   393,   174,
      -1,   189,    -1,   393,   189,    -1,   189,    -1,   189,   162,
     388,   163,    -1,   162,   385,   163,    -1,    -1,   393,    -1,
     385,     8,   393,    -1,   385,     8,   157,    -1,   385,    -1,
     157,    -1,    -1,    -1,    26,   393,    -1,   388,     8,   189,
      -1,   189,    -1,   388,     8,   189,    90,   393,    -1,   189,
      90,   393,    -1,    79,   122,   393,    -1,   390,     8,   389,
      -1,   389,    -1,   390,   332,    -1,    -1,   166,   172,   391,
     173,    -1,    25,   393,    -1,    51,   393,    -1,   196,    -1,
     124,    -1,   392,    -1,   124,   162,   393,   163,    -1,   124,
     162,   393,     8,   393,   163,    -1,   146,    -1,   172,    98,
     172,   386,   173,    26,   393,   173,    -1,   172,   385,     8,
     393,   173,    -1,   393,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   814,   814,   814,   819,   821,   824,   825,   826,   827,
     828,   829,   832,   834,   834,   836,   836,   838,   839,   844,
     845,   846,   847,   848,   849,   853,   855,   858,   859,   860,
     861,   866,   867,   871,   872,   874,   877,   883,   890,   897,
     901,   907,   909,   912,   913,   914,   915,   918,   919,   923,
     928,   928,   932,   932,   937,   936,   940,   940,   943,   944,
     945,   946,   947,   948,   949,   950,   951,   952,   953,   954,
     955,   958,   956,   961,   963,   971,   974,   975,   979,   980,
     981,   982,   983,   990,   996,  1000,  1000,  1006,  1007,  1011,
    1012,  1016,  1021,  1020,  1030,  1029,  1042,  1041,  1060,  1058,
    1077,  1076,  1085,  1083,  1095,  1094,  1106,  1104,  1117,  1118,
    1122,  1125,  1128,  1129,  1130,  1133,  1135,  1138,  1139,  1142,
    1143,  1146,  1147,  1151,  1152,  1157,  1158,  1161,  1162,  1166,
    1167,  1171,  1172,  1176,  1177,  1181,  1182,  1187,  1188,  1193,
    1194,  1195,  1196,  1199,  1202,  1204,  1207,  1208,  1212,  1214,
    1217,  1220,  1223,  1224,  1227,  1228,  1232,  1234,  1236,  1237,
    1241,  1245,  1249,  1254,  1259,  1264,  1269,  1275,  1284,  1286,
    1288,  1289,  1293,  1296,  1299,  1303,  1307,  1311,  1315,  1320,
    1328,  1330,  1333,  1334,  1335,  1337,  1342,  1343,  1346,  1347,
    1348,  1352,  1353,  1355,  1356,  1360,  1362,  1365,  1365,  1369,
    1368,  1372,  1376,  1374,  1387,  1384,  1395,  1397,  1399,  1401,
    1403,  1407,  1408,  1409,  1412,  1418,  1421,  1427,  1430,  1435,
    1437,  1442,  1447,  1451,  1452,  1458,  1459,  1464,  1465,  1470,
    1471,  1475,  1476,  1480,  1482,  1488,  1493,  1494,  1496,  1500,
    1501,  1502,  1503,  1507,  1508,  1509,  1510,  1511,  1512,  1514,
    1519,  1522,  1523,  1527,  1528,  1531,  1532,  1535,  1536,  1539,
    1540,  1544,  1545,  1546,  1547,  1548,  1549,  1552,  1553,  1556,
    1557,  1558,  1561,  1563,  1565,  1566,  1569,  1571,  1575,  1576,
    1578,  1579,  1582,  1586,  1587,  1591,  1592,  1596,  1597,  1601,
    1605,  1610,  1611,  1612,  1615,  1617,  1618,  1619,  1622,  1623,
    1624,  1625,  1626,  1627,  1628,  1629,  1630,  1631,  1632,  1633,
    1634,  1635,  1636,  1637,  1638,  1639,  1640,  1641,  1642,  1643,
    1644,  1645,  1646,  1647,  1648,  1649,  1650,  1651,  1652,  1653,
    1654,  1655,  1656,  1657,  1658,  1659,  1660,  1661,  1662,  1664,
    1665,  1667,  1669,  1670,  1671,  1672,  1673,  1674,  1675,  1676,
    1677,  1678,  1679,  1680,  1681,  1682,  1683,  1684,  1685,  1686,
    1688,  1687,  1696,  1695,  1703,  1704,  1708,  1712,  1716,  1722,
    1726,  1732,  1734,  1738,  1740,  1744,  1749,  1750,  1754,  1761,
    1768,  1770,  1775,  1776,  1777,  1781,  1785,  1789,  1790,  1791,
    1792,  1796,  1802,  1811,  1824,  1825,  1828,  1831,  1834,  1835,
    1838,  1842,  1845,  1848,  1855,  1856,  1860,  1861,  1863,  1867,
    1868,  1869,  1870,  1871,  1872,  1873,  1874,  1875,  1876,  1877,
    1878,  1879,  1880,  1881,  1882,  1883,  1884,  1885,  1886,  1887,
    1888,  1889,  1890,  1891,  1892,  1893,  1894,  1895,  1896,  1897,
    1898,  1899,  1900,  1901,  1902,  1903,  1904,  1905,  1906,  1907,
    1908,  1909,  1910,  1911,  1912,  1913,  1914,  1915,  1916,  1917,
    1918,  1919,  1920,  1921,  1922,  1923,  1924,  1925,  1926,  1927,
    1928,  1929,  1930,  1931,  1932,  1933,  1934,  1935,  1936,  1937,
    1938,  1939,  1940,  1941,  1942,  1943,  1944,  1948,  1953,  1954,
    1957,  1958,  1959,  1963,  1964,  1965,  1969,  1970,  1971,  1975,
    1976,  1977,  1980,  1982,  1986,  1987,  1988,  1990,  1991,  1992,
    1993,  1994,  1995,  1996,  1997,  1998,  1999,  2002,  2007,  2008,
    2009,  2010,  2011,  2013,  2014,  2017,  2018,  2022,  2025,  2031,
    2032,  2033,  2034,  2035,  2036,  2037,  2042,  2044,  2048,  2049,
    2052,  2053,  2057,  2060,  2062,  2064,  2068,  2069,  2070,  2072,
    2075,  2079,  2080,  2081,  2084,  2085,  2086,  2087,  2088,  2090,
    2091,  2097,  2099,  2102,  2105,  2107,  2109,  2112,  2114,  2118,
    2120,  2123,  2126,  2132,  2134,  2137,  2138,  2143,  2146,  2150,
    2150,  2155,  2158,  2159,  2163,  2164,  2169,  2170,  2174,  2175,
    2179,  2180,  2185,  2187,  2192,  2193,  2194,  2195,  2196,  2197,
    2198,  2200,  2203,  2205,  2209,  2210,  2211,  2212,  2213,  2215,
    2217,  2219,  2223,  2224,  2225,  2229,  2232,  2235,  2238,  2242,
    2246,  2253,  2257,  2264,  2265,  2270,  2272,  2273,  2276,  2277,
    2280,  2281,  2285,  2286,  2290,  2291,  2292,  2293,  2295,  2298,
    2301,  2302,  2303,  2305,  2307,  2311,  2312,  2313,  2315,  2316,
    2317,  2321,  2323,  2326,  2328,  2329,  2330,  2331,  2334,  2336,
    2337,  2341,  2343,  2346,  2348,  2349,  2350,  2354,  2356,  2359,
    2362,  2364,  2366,  2370,  2371,  2373,  2374,  2380,  2381,  2383,
    2385,  2387,  2389,  2392,  2393,  2394,  2398,  2399,  2400,  2401,
    2402,  2403,  2404,  2408,  2409,  2413,  2421,  2423,  2427,  2431,
    2439,  2440,  2446,  2447,  2455,  2458,  2462,  2465,  2470,  2471,
    2472,  2473,  2477,  2478,  2482,  2484,  2485,  2487,  2491,  2497,
    2499,  2503,  2506,  2509,  2518,  2521,  2524,  2525,  2528,  2529,
    2533,  2538,  2542,  2548,  2556,  2557
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
  "new_elseif_list", "else_single", "new_else_single",
  "method_parameter_list", "non_empty_method_parameter_list",
  "parameter_list", "non_empty_parameter_list",
  "function_call_parameter_list", "non_empty_fcall_parameter_list",
  "global_var_list", "global_var", "static_var_list",
  "class_statement_list", "class_statement", "$@18", "$@19", "$@20",
  "$@21", "trait_rules", "trait_precedence_rule", "trait_alias_rule",
  "trait_alias_rule_method", "xhp_attribute_stmt", "xhp_attribute_decl",
  "xhp_attribute_decl_type", "xhp_attribute_enum", "xhp_attribute_default",
  "xhp_attribute_is_required", "xhp_category_stmt", "xhp_category_decl",
  "xhp_children_stmt", "xhp_children_paren_expr", "xhp_children_decl_expr",
  "xhp_children_decl_tag", "method_body", "variable_modifiers",
  "method_modifiers", "non_empty_member_modifiers", "member_modifier",
  "parameter_modifiers", "parameter_modifier",
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
     248,   248,   249,   249,   249,   249,   249,   249,   249,   249,
     250,   250,   251,   251,   251,   251,   252,   252,   253,   253,
     253,   254,   254,   254,   254,   255,   255,   257,   256,   258,
     256,   256,   259,   256,   260,   256,   256,   256,   256,   256,
     256,   261,   261,   261,   262,   263,   263,   264,   264,   265,
     265,   266,   266,   267,   267,   267,   267,   268,   268,   269,
     269,   270,   270,   271,   271,   272,   273,   273,   273,   274,
     274,   274,   274,   275,   275,   275,   275,   275,   275,   275,
     276,   276,   276,   277,   277,   278,   278,   279,   279,   280,
     280,   281,   281,   281,   281,   281,   281,   282,   282,   283,
     283,   283,   284,   284,   284,   284,   285,   285,   286,   286,
     286,   286,   287,   288,   288,   289,   289,   290,   290,   291,
     292,   293,   293,   293,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     294,   294,   294,   294,   294,   294,   294,   294,   294,   294,
     295,   294,   296,   294,   294,   294,   297,   298,   298,   299,
     299,   300,   300,   301,   301,   302,   303,   303,   304,   305,
     306,   306,   307,   307,   307,   308,   308,   309,   309,   309,
     309,   310,   311,   311,   312,   312,   313,   313,   314,   314,
     315,   316,   316,   317,   317,   317,   318,   318,   318,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,   319,   319,   319,   319,   319,   319,   320,   321,   321,
     322,   322,   322,   323,   323,   323,   324,   324,   324,   325,
     325,   325,   326,   326,   327,   327,   327,   327,   327,   327,
     327,   327,   327,   327,   327,   327,   327,   327,   328,   328,
     328,   328,   328,   328,   328,   328,   328,   329,   329,   330,
     330,   330,   330,   330,   330,   330,   331,   331,   332,   332,
     333,   333,   334,   334,   334,   334,   335,   335,   335,   335,
     335,   336,   336,   336,   337,   337,   337,   337,   337,   337,
     337,   338,   338,   339,   339,   339,   339,   340,   340,   341,
     341,   342,   342,   343,   343,   344,   344,   345,   345,   347,
     346,   348,   349,   349,   350,   350,   351,   351,   352,   352,
     353,   353,   354,   354,   355,   355,   355,   355,   355,   355,
     355,   355,   355,   355,   356,   356,   356,   356,   356,   356,
     356,   356,   357,   357,   357,   358,   358,   358,   358,   358,
     358,   359,   359,   360,   360,   361,   361,   361,   362,   362,
     363,   363,   364,   364,   365,   365,   365,   365,   365,   365,
     366,   366,   366,   366,   366,   367,   367,   367,   367,   367,
     367,   368,   368,   369,   369,   369,   369,   369,   369,   369,
     369,   370,   370,   371,   371,   371,   371,   372,   372,   373,
     373,   373,   373,   374,   374,   374,   374,   375,   375,   375,
     375,   375,   375,   376,   376,   376,   377,   377,   377,   377,
     377,   377,   377,   378,   378,   379,   380,   380,   381,   381,
     382,   382,   383,   383,   384,   384,   385,   385,   386,   386,
     386,   386,   387,   387,   388,   388,   388,   388,   389,   390,
     390,   391,   391,   392,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   394,   394
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
       4,     5,     7,     6,     6,     7,     9,     8,     3,     2,
       1,     0,     3,     4,     6,     5,     5,     6,     8,     7,
       2,     0,     1,     2,     3,     4,     3,     1,     1,     2,
       4,     3,     5,     1,     3,     2,     0,     0,     4,     0,
       5,     2,     0,    10,     0,    11,     3,     3,     3,     3,
       5,     2,     2,     0,     6,     5,     4,     3,     1,     1,
       3,     4,     1,     1,     1,     1,     4,     1,     3,     2,
       0,     2,     0,     1,     3,     1,     1,     1,     1,     3,
       4,     4,     4,     1,     1,     2,     2,     2,     3,     3,
       1,     1,     1,     1,     3,     1,     1,     1,     0,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       1,     1,     3,     5,     1,     3,     5,     4,     3,     3,
       2,     1,     3,     3,     1,     1,     0,     2,     4,     3,
       6,     1,     1,     1,     6,     3,     4,     6,     3,     3,
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
       1,     1,     1,     1,     1,     1,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     1,     0,     0,
       1,     1,     3,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     1,     1,
       2,     2,     4,     3,     4,     1,     1,     3,     3,     1,
       1,     1,     1,     3,     3,     3,     2,     0,     1,     0,
       1,     0,     5,     3,     3,     1,     1,     1,     1,     3,
       2,     1,     1,     1,     1,     1,     2,     2,     4,     3,
       4,     2,     0,     5,     3,     3,     1,     3,     1,     2,
       0,     5,     3,     2,     0,     3,     0,     4,     2,     0,
       3,     3,     1,     0,     1,     2,     2,     4,     3,     3,
       2,     4,     2,     4,     1,     1,     1,     1,     1,     2,
       4,     3,     4,     3,     1,     1,     1,     1,     2,     4,
       4,     3,     1,     1,     3,     7,     6,     8,     9,     8,
      10,     7,     6,     1,     2,     4,     4,     1,     1,     4,
       1,     0,     1,     2,     1,     1,     2,     4,     3,     3,
       0,     1,     2,     4,     3,     2,     3,     6,     0,     1,
       4,     2,     0,     5,     3,     3,     1,     6,     4,     4,
       2,     2,     0,     5,     3,     3,     1,     2,     0,     5,
       3,     3,     1,     2,     2,     1,     2,     1,     4,     3,
       3,     6,     3,     1,     1,     1,     4,     4,     2,     2,
       4,     2,     2,     1,     3,     3,     0,     2,     5,     6,
       1,     2,     1,     4,     3,     0,     1,     3,     3,     1,
       1,     0,     0,     2,     3,     1,     5,     3,     3,     3,
       1,     2,     0,     4,     2,     2,     1,     1,     1,     4,
       6,     1,     8,     5,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,     0,     0,     0,
       0,   579,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   652,     0,   640,   498,
       0,   504,   505,    19,   530,   628,    70,   506,     0,    52,
       0,     0,     0,     0,     0,     0,     0,     0,    91,     0,
       0,     0,     0,     0,     0,     0,     0,   491,     0,     0,
       0,     0,   112,     0,     0,     0,   510,   512,   513,   507,
     508,     0,     0,   514,   509,     0,     0,   489,    20,    21,
      22,    24,    23,     0,   511,     0,     0,     0,     0,   515,
       0,    69,    42,   632,   499,     0,     0,     4,    31,    33,
      36,   529,     0,   488,     0,     6,    90,     7,     8,     9,
       0,   293,     0,     0,     0,     0,   291,   357,   356,   365,
     364,     0,   281,   595,   490,     0,   532,   355,     0,   598,
     292,     0,     0,   596,   597,   594,   623,   627,     0,   345,
     531,    10,   491,     0,     0,    31,    90,   692,   292,   691,
       0,   689,   688,   359,     0,     0,   329,   330,   331,   332,
     354,   352,   351,   350,   349,   348,   347,   346,   491,     0,
     705,   490,     0,   312,   310,     0,   656,     0,   539,   280,
     494,     0,   705,   493,     0,   503,   635,   634,   495,     0,
       0,   497,   353,     0,     0,     0,   284,     0,    50,   286,
       0,     0,    56,    58,     0,     0,    60,     0,     0,     0,
     727,   731,     0,     0,    31,   726,     0,   728,     0,    62,
       0,    42,     0,     0,     0,    26,    27,   188,     0,     0,
     187,   114,   113,   193,    90,     0,     0,     0,     0,     0,
     702,   100,   110,   648,   652,   677,     0,   517,     0,     0,
       0,   675,     0,    15,     0,    35,     0,   287,   104,   111,
     397,   372,     0,   696,   293,     0,   291,   292,     0,     0,
     500,     0,   501,     0,     0,     0,    82,     0,     0,    38,
     181,     0,    18,    89,     0,   109,    96,   108,    79,    80,
      81,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   640,    78,   631,   631,
     662,     0,     0,     0,    90,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   311,   309,
       0,   599,   584,   631,     0,   590,   181,   631,     0,   633,
     624,   648,     0,     0,     0,   581,   576,   539,     0,     0,
       0,     0,   660,     0,   377,   538,   651,     0,     0,    38,
       0,   181,   279,     0,   636,   584,   592,   496,     0,    42,
     149,     0,    67,     0,     0,   285,     0,     0,     0,     0,
       0,    59,    77,    61,   724,   725,     0,   722,     0,     0,
     706,     0,   701,    63,     0,    76,    28,     0,    17,     0,
       0,   189,     0,    65,     0,     0,     0,    66,   693,     0,
       0,     0,     0,     0,   120,     0,   649,     0,     0,     0,
       0,   516,   676,   530,     0,     0,   674,   535,   673,    34,
       5,    12,    13,    64,     0,   118,     0,     0,   366,     0,
     539,     0,     0,     0,     0,   278,   342,   603,    47,    41,
      43,    44,    45,    46,     0,   358,   533,   534,    32,     0,
       0,     0,   541,   182,     0,   360,    92,   116,   315,   317,
     316,     0,     0,   313,   314,   318,   320,   319,   334,   333,
     336,   335,   337,   339,   340,   338,   328,   327,   322,   323,
     321,   324,   325,   326,   341,   630,     0,     0,   666,     0,
     539,   695,   601,   623,   102,   106,     0,    98,     0,     0,
     289,   295,   308,   307,   306,   305,   304,   303,   302,   301,
     300,   299,   298,     0,   586,   585,     0,     0,     0,     0,
       0,     0,   690,   574,   578,   538,   580,     0,     0,   705,
       0,   655,     0,   654,     0,   639,   638,     0,     0,   586,
     585,   282,   151,   153,   283,     0,    42,   133,    51,   286,
       0,     0,     0,     0,   145,   145,    57,     0,     0,   720,
     539,     0,   711,     0,     0,     0,   537,     0,     0,   489,
       0,    36,   519,   488,   526,     0,   518,    40,   525,    85,
       0,    25,    29,     0,   186,   194,   362,   191,     0,     0,
     686,   687,    11,   715,     0,     0,     0,   648,   645,     0,
     376,   685,   684,   683,     0,   679,     0,   680,   682,     0,
       5,   288,     0,     0,   391,   392,   400,   399,     0,     0,
     538,   371,   375,     0,   697,     0,     0,   600,   584,   591,
     629,     0,   704,   183,   487,   540,   180,     0,   583,     0,
       0,   118,   344,     0,   380,   381,     0,   378,   538,   661,
       0,   181,   120,   118,    94,   116,   640,   296,     0,     0,
     181,   588,   589,   602,   625,   626,     0,     0,     0,   562,
     546,   547,   548,     0,     0,     0,   555,   554,   568,   539,
       0,   576,   659,   658,     0,   637,   584,   593,   502,     0,
     155,     0,     0,    48,     0,     0,     0,     0,   126,   127,
     137,     0,    42,   135,    73,   145,     0,   145,     0,     0,
     729,     0,   538,   721,   723,   710,   709,     0,   707,   520,
     521,   545,     0,   539,   537,     0,     0,   374,     0,   668,
       0,    75,     0,    30,   190,   583,     0,   694,    68,     0,
       0,   703,   119,   121,   196,     0,     0,   646,     0,   678,
       0,    16,     0,   117,   196,     0,     0,   368,     0,   698,
       0,     0,   586,   585,   707,     0,   184,    39,   170,     0,
     541,   582,   735,   583,   115,     0,   343,   665,   664,   181,
       0,     0,     0,     0,   118,   503,   587,   181,     0,     0,
     551,   552,   553,   556,   557,   566,     0,   539,   562,     0,
     550,   570,   538,   573,   575,   577,     0,   653,   587,     0,
       0,     0,     0,   152,    53,     0,   286,   128,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   139,     0,   718,
     719,     0,     0,   733,     0,   523,   538,   536,     0,   528,
       0,   539,     0,   527,   672,     0,   539,     0,     0,     0,
     192,   717,   714,     0,   258,   650,   648,   290,   294,     0,
      14,   258,   403,     0,     0,   405,   398,   401,     0,   396,
       0,   699,     0,     0,   181,   185,   712,   583,   169,   734,
       0,     0,   196,     0,     0,   622,   196,   196,   583,     0,
     297,   181,     0,   616,     0,   559,   538,   561,     0,   549,
       0,     0,   539,   567,   657,     0,    42,     0,   148,   134,
       0,   125,    71,   138,     0,     0,   141,     0,   146,   147,
      42,   140,   730,   708,     0,   544,   543,   522,     0,   538,
     373,   524,     0,   379,   538,   667,     0,    42,   712,     0,
     122,     0,     0,   261,   262,   263,   266,   265,   264,   256,
       0,     0,     0,   101,   195,   197,     0,   255,   259,     0,
     258,     0,   681,   105,   394,     0,     0,   367,   587,   181,
       0,     0,   386,   168,   735,     0,   172,   712,   258,   663,
     621,   258,   258,     0,   196,     0,   615,   565,   564,   558,
       0,   560,   538,   569,    42,   154,    49,    54,     0,   136,
     142,    42,   144,     0,     0,   370,     0,   671,   670,     0,
       0,   386,   716,     0,     0,   123,   225,   223,   489,    24,
       0,   219,     0,   224,   235,     0,   233,   238,     0,   237,
       0,   236,     0,    90,   260,   199,     0,   201,     0,   257,
     647,   395,   393,   404,   402,   181,     0,   619,   713,     0,
       0,     0,   173,     0,     0,    97,   103,   107,   712,   258,
     617,     0,   572,     0,   150,     0,    42,   131,    72,   143,
     732,   542,     0,     0,     0,    86,     0,     0,     0,   209,
     213,     0,     0,   206,   454,   453,   450,   452,   451,   470,
     472,   471,   442,   432,   448,   447,   410,   419,   420,   422,
     421,   441,   425,   423,   424,   426,   427,   428,   429,   430,
     431,   433,   434,   435,   436,   437,   438,   440,   439,   411,
     412,   413,   415,   416,   418,   456,   457,   466,   465,   464,
     463,   462,   461,   449,   467,   458,   459,   460,   443,   444,
     445,   446,   468,   469,   473,   475,   474,   476,   477,   455,
     479,   478,   414,   481,   483,   482,   417,   486,   484,   485,
     480,   409,   230,   406,     0,   207,   251,   252,   250,   243,
       0,   244,   208,   274,     0,     0,     0,     0,    90,     0,
     618,     0,    42,     0,   176,     0,   175,    42,     0,    99,
     563,     0,    42,   129,    55,     0,   369,   669,    42,    42,
     277,   124,     0,     0,   227,   220,     0,     0,     0,   232,
     234,     0,     0,   239,   246,   247,   245,     0,     0,   198,
       0,     0,     0,     0,   620,     0,   389,   541,     0,   177,
       0,   174,     0,    42,   571,     0,     0,     0,     0,   210,
      31,     0,   211,   212,     0,     0,   226,   229,   407,   408,
       0,   221,   248,   249,   241,   242,   240,   275,   272,   202,
     200,   276,     0,   390,   540,     0,   361,     0,   179,    93,
       0,     0,   132,    84,   363,     0,   258,   228,   231,     0,
     583,   204,     0,   387,   385,   178,    95,   130,    88,   217,
       0,   257,   273,   158,     0,   541,   268,   583,   388,     0,
      87,    74,     0,     0,   216,   712,   268,   157,   269,   270,
     271,   735,   267,     0,     0,     0,   215,     0,   156,   583,
       0,   712,     0,   214,   253,    42,   203,   735,     0,   160,
       0,     0,     0,     0,   161,     0,   205,     0,   254,     0,
     164,     0,   163,    42,   165,     0,   162,     0,     0,   167,
      83,   166
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    97,   630,   440,   145,   224,   225,
      99,   100,   101,   102,   103,   104,   268,   459,   460,   384,
     197,  1085,   390,  1018,  1308,   751,   752,  1321,   284,   146,
     461,   659,   803,   462,   477,   675,   424,   672,   463,   445,
     673,   286,   241,   258,   110,   661,   633,   616,   762,  1034,
     839,   718,  1214,  1088,   568,   724,   389,   576,   726,   940,
     563,   710,   713,   831,  1314,  1315,   789,   790,   471,   472,
     229,   230,   235,   874,   974,  1052,  1196,  1300,  1317,  1222,
    1262,  1263,  1264,  1040,  1041,  1042,  1223,  1229,  1271,  1045,
    1046,  1050,  1189,  1190,  1191,  1346,   975,   976,   977,   978,
    1331,  1332,  1194,   979,   111,   191,   385,   386,   112,   113,
     114,   115,   116,   658,   755,   449,   450,   861,   451,   862,
     117,   118,   119,   594,   120,   121,  1070,  1247,   122,   446,
    1062,   447,   775,   638,   889,   886,  1182,  1183,   123,   124,
     125,   185,   192,   271,   372,   126,   741,   598,   127,   742,
     366,   656,   743,   697,   813,   815,   816,   817,   699,   921,
     922,   700,   544,   357,   154,   155,   128,   792,   341,   342,
     649,   129,   186,   148,   131,   132,   133,   134,   135,   136,
     137,   506,   138,   188,   189,   427,   177,   178,   509,   510,
     865,   866,   250,   251,   624,   139,   419,   140,   454,   141,
     216,   242,   279,   399,   737,   992,   614,   579,   580,   581,
     217,   218,   900
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -951
static const yytype_int16 yypact[] =
{
    -951,   118,  -951,  -951,  3597,  9443,  9443,   -47,  9443,  9443,
    9443,  -951,  9443,  9443,  9443,  9443,  9443,  9443,  9443,  9443,
    9443,  9443,  9443,  9443,  1798,  1798,  7231,  9443,  2724,   -41,
      11,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  9443,  -951,
      11,    22,   121,   131,    11,  7389,   280,  7547,  -951,  1208,
    7705,   -36,  9443,   848,    90,   182,   188,    37,   157,   159,
     162,   191,  -951,   280,   229,   237,  -951,  -951,  -951,  -951,
    -951,   326,   514,  -951,  -951,   280,  7863,  -951,  -951,  -951,
    -951,  -951,  -951,   280,  -951,    18,   240,   280,   280,  -951,
    9443,  -951,  -951,   179,   384,   399,   399,  -951,   348,   271,
     -35,  -951,   258,  -951,    47,  -951,   396,  -951,  -951,  -951,
     728,  -951,   261,   270,   274, 10074,  -951,  -951,   385,  -951,
     394,   395,  -951,    40,   283,   320,  -951,  -951,   561,    17,
    2140,    88,   296,    95,   106,   301,    26,  -951,    97,  -951,
     414,  -951,   383,   330,   338,  -951,   396, 10734,  2474, 10734,
    9443, 10734, 10734,  9758,   460,   280,  -951,  -951,   461,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  1753,
     349,  -951,   373,   405,   405,  1798,  2240,   351,   524,  -951,
     402,  1753,   349,   415,   417,   363,   114,  -951,   419,    88,
    8021,  -951,  -951,  9443,  6125,    48, 10734,  6915,  -951,  9443,
    9443,   280,  -951,  -951, 10115,   371,  -951, 10156,  1208,  1208,
     401,  -951,   392,   609,   569,  -951,   571,  -951,   280,  -951,
   10197,  -951, 10241,   280,    49,  -951,     3,  -951,  1780,    50,
    -951,  -951,  -951,   573,   396,    52,  1798,  1798,  1798,   420,
     426,  -951,  -951,  2061,  7231,    27,   233,  -951,  9601,  1798,
     567,  -951,   280,  -951,   -20,   271,   423, 10389,  -951,  -951,
    -951,   521,   593,   520,   439, 10734,   440,   474,  3755,  9443,
      34,   436,   481,    34,   272,   257,  -951,   280,  1208,   444,
    8179,  1208,  -951,  -951,   639,  -951,  -951,  -951,  -951,  -951,
    -951,  9443,  9443,  9443,  8337,  9443,  9443,  9443,  9443,  9443,
    9443,  9443,  9443,  9443,  9443,  9443,  9443,  9443,  9443,  9443,
    9443,  9443,  9443,  9443,  9443,  9443,  2724,  -951,  9443,  9443,
    9443,   366,   280,   280,   396,   728,  7073,  9443,  9443,  9443,
    9443,  9443,  9443,  9443,  9443,  9443,  9443,  9443,  -951,  -951,
     456,  -951,   123,  9443,  9443,  -951,  8179,  9443,  9443,   179,
     127,  2061,   449,  8495, 10282,  -951,   451,   616,  1753,   453,
      23,   366,   405,  8653,  -951,  8811,  -951,   470,   169,  -951,
      98,  8179,  -951,  1144,  -951,   137,  -951,  -951, 10323,  -951,
    -951,  9443,  -951,   550,  6283,   646,   483, 10627,   642,    38,
      87,  -951,  -951,  -951,  -951,  -951,  1208,   585,   497,   665,
    -951, 10332,  -951,  -951,  3913,  -951,    13,   848,  -951,   280,
    9443,   405,    90,  -951, 10332,   502,   603,  -951,   405,    64,
      72,   170,   507,   280,   568,   519,   405,    77,   529,  1356,
     280,  -951,  -951,   633,   860,   -17,  -951,  -951,  -951,   271,
    -951,  -951,  -951,  -951,  9443,   583,   552,   115,  -951,   594,
     709,   546,  1208,  1208,   707,    20,   660,    33,  -951,  -951,
    -951,  -951,  -951,  -951,  1976,  -951,  -951,  -951,  -951,    46,
    1798,   551,   715, 10734,   716,  -951,  -951,   611, 10774, 10811,
    9758,  9443, 10693, 10833, 10854,  7123,  2682,  6907,  7279,  7279,
    7279,  7279,  2898,  2898,  2898,  2898,  1563,  1563,   404,   404,
     404,   461,   461,   461,  -951, 10734,   549,   555, 10490,   559,
     723,   -46,   564,   127,  -951,  -951,   280,  -951,   678,  9443,
    -951,  9758,  9758,  9758,  9758,  9758,  9758,  9758,  9758,  9758,
    9758,  9758,  9758,  9443,   -46,   574,   560,  2348,   572,   575,
    2884,    78,  -951,   400,  -951,   280,  -951,   439,    20,   349,
    1798, 10734,  1798, 10531,    41,   145,  -951,   579,  9443,  -951,
    -951,  -951,  5967,   249, 10734,    11,  -951,  -951,  -951,  9443,
     500, 10332,   280,  6441,   590,   592,  -951,    67,   626,  -951,
     751,   596,   807,  1208, 10332, 10332, 10332,   591,    21,   627,
     598,   -29,  -951,   631,  -951,   599,  -951,  -951,  -951,   671,
     280,  -951,  -951,  3105,  -951,  -951,  -951,   763,  1798,   612,
    -951,  -951,  -951,   703,    69,  1373,   619,  2061,  2619,   784,
    -951,  -951,  -951,  -951,   617,  -951,  9443,  -951,  -951,  3281,
    -951, 10734,  1373,   623,  -951,  -951,  -951,  -951,   788,  9443,
     521,  -951,  -951,   629,  -951,  1208,  1310,  -951,   152,  -951,
    -951,  1208,  -951,   405,  -951,  8969,  -951, 10332,   109,   634,
    1373,   583, 10650,  9443,  -951,  -951,  9443,  -951,  9443,  -951,
     637,  8179,   568,   583,  -951,   611,  2724,   405,  3227,   638,
    8179,  -951,  -951,   164,  -951,  -951,   792,   714,   714,   400,
    -951,  -951,  -951,   640,    39,   641,  -951,  -951,  -951,   808,
     647,   451,   405,   405,  9127,  -951,   174,  -951,  -951,  9910,
     256,    11,  6915,  -951,   649,  4071,   651,  1798,   693,   405,
    -951,   820,  -951,  -951,  -951,  -951,   328,  -951,     4,  1208,
    -951,  1208,   585,  -951,  -951,  -951,   813,   661,   666,  -951,
    -951,   718,   662,   835, 10332,   708,   280,   521,   280, 10332,
     672,  -951,   687,  -951,  -951,   109, 10332,   405,  -951,  1208,
     280,  -951,   845,  -951,  -951,    82,   682,   405,  9285,  -951,
    1551,  -951,  3439,   845,  -951,   -32,   -33, 10734,   734,  -951,
     683,  9443,   -46,   688,  -951,  1798, 10734,  -951,  -951,   686,
     853,  -951,  1208,   109,  -951,   689, 10650, 10734, 10586,  8179,
     695,   733,   736,   710,   583,   363,   711,  8179,   740,  9443,
    -951,  -951,  -951,  -951,  -951,   762,   735,   908,   400,   781,
    -951,   521,   400,  -951,  -951,  -951,  1798, 10734,  -951,    11,
     895,   862,  6915,  -951,  -951,   758,  9443,   405,   500,   761,
   10332,  4229,   413,   765,  9443,    36,   202,  -951,   772,  -951,
    -951,   952,   910,  -951, 10332,  -951, 10332,  -951,   764,  -951,
     827,   942,   789,  -951,   832,   785,   955,  1373,   790,   793,
    -951,  -951,   877,  1373,   589,  -951,  2061,  -951,  9758,   794,
    -951,   834,  -951,   141,  9443,  -951,  -951,  -951,  9443,  -951,
    9443,  -951,  9951,   797,  8179,   405,   945,   111,  -951,  -951,
     255,   799,  -951,  9443,   801,  -951,  -951,  -951,   109,   800,
    -951,  8179,   803,  -951,   400,  -951,   400,  -951,   805,  -951,
     858,   811,   978,  -951,   405,   961,  -951,   815,  -951,  -951,
     817,  -951,  -951,  -951,   821,   828,  -951,  1669,  -951,  -951,
    -951,  -951,  -951,  -951,  1208,  -951,   869,  -951, 10332,   521,
    -951,  -951, 10332,  -951, 10332,  -951,   934,  -951,   945,  1208,
    -951,  1208,  1373,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    2284,   859,   450,  -951,  -951,  -951,   383,  1928,  -951,    53,
    1092,    84,  -951,  -951,   866,  9992, 10033, 10734,   841,  8179,
     842,  1208,   912,  -951,  1208,   944,  1006,   945,  1046, 10734,
    -951,  1165,  1298,   847,  -951,   849,  -951,  -951,   899,  -951,
     400,  -951,   521,  -951,  -951,  5967,  -951,  -951,  6599,  -951,
    -951,  -951,  5967,   854, 10332,  -951,   909,  -951,   913,   857,
    4387,   912,  -951,  1024,    45,  -951,  -951,  -951,    55,   867,
      57,  -951,  9759,  -951,  -951,    60,  -951,  -951,  1021,  -951,
     871,  -951,   968,   396,  -951,  -951,  1208,  -951,   383,  1092,
    -951,  -951,  -951,  -951,  -951,  8179,   861,  -951,  -951,   874,
     868,   293,  1035, 10332,   875,  -951,  -951,  -951,   945,  1387,
    -951,   400,  -951,   927,  5967,  6757,  -951,  -951,  -951,  5967,
    -951,  -951, 10332, 10332,   876,  -951,   884, 10332,  1373,  -951,
    -951,  1296,  2284,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,   473,  -951,   859,  -951,  -951,  -951,  -951,  -951,
      44,   459,  -951,  1048,    62,   280,   968,  1050,   396,   891,
    -951,   302,  -951,   992,  1053, 10332,  -951,  -951,   893,  -951,
    -951,   400,  -951,  -951,  -951,  4545,  -951,  -951,  -951,  -951,
    -951,  -951,   776,    42,  -951,  -951, 10332,  9759,  9759,  1018,
    -951,  1021,  1021,   525,  -951,  -951,  -951, 10332,   997,  -951,
     900,    66, 10332,   280,  -951,   998,  -951,  1066,  4703,  1062,
   10332,  -951,  4861,  -951,  -951,  5019,   904,  5177,  5335,  -951,
     993,   946,  -951,  -951,   994,  1296,  -951,  -951,  -951,  -951,
     929,  -951,  1059,  -951,  -951,  -951,  -951,  -951,  1084,  -951,
    -951,  -951,   917,  -951,   306,   935,  -951, 10332,  -951,  -951,
    5493,   928,  -951,  -951,  -951,   280,  1092,  -951,  -951, 10332,
     112,  -951,  1037,  -951,  -951,  -951,  -951,  -951,    19,   957,
     280,  1076,  -951,  -951,   941,  1107,   468,   112,  -951,   947,
    -951,  -951,  1373,   943,  -951,   945,   310,  -951,  -951,  -951,
    -951,  1208,  -951,   949,  1373,    68,  -951,   151,  -951,  1079,
     315,   945,  1052,  -951,  -951,  -951,  -951,  1208,  1054,  1110,
     151,   956,  5651,   325,  1115, 10332,  -951,   958,  -951,  1057,
    1119, 10332,  -951,  -951,  1124, 10332,  -951,  5809, 10332,  -951,
    -951,  -951
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -951,  -951,  -951,  -397,  -951,  -951,  -951,    -4,  -951,   731,
     -24,   898,  1952,  -951,  1462,  -951,  -214,  -951,     6,  -951,
    -951,  -951,  -951,  -951,  -951,  -169,  -951,  -951,  -138,    32,
       0,  -951,  -951,     7,  -951,  -951,  -951,  -951,     8,  -951,
    -951,   818,   822,   819,  1012,   471,  -539,   476,   527,  -162,
    -951,   324,  -951,  -951,  -951,  -951,  -951,  -951,  -467,   226,
    -951,  -951,  -951,  -951,  -149,  -951,  -664,  -951,  -333,  -951,
    -951,   766,  -951,  -725,  -951,  -951,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,    73,  -951,  -951,  -951,  -951,  -951,
      -7,  -951,   208,  -872,  -951,  -160,  -951,  -948,  -941,  -947,
    -145,  -951,    12,  -951,   -49,   -21,  1150,  -529,  -309,  -951,
    -951,  2322,  1102,  -951,  -951,  -609,  -951,  -951,  -951,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,   165,  -951,   429,  -951,
    -951,  -951,  -951,  -951,  -951,  -951,  -951,  -843,  -951,  1217,
    2232,  -290,  -951,  -951,   407,   580,  2003,  -951,  -951,   465,
    -341,  -769,  -951,  -951,   522,  -525,   398,  -951,  -951,  -951,
    -951,  -951,   512,  -951,  -951,  -951,  -625,  -894,  -161,  -141,
    -107,  -951,  -951,    10,  -951,  -951,  -951,  -951,    -8,   -69,
    -951,  -224,  -951,  -951,  -951,  -329,   975,  -951,  -951,  -951,
    -951,  -951,   496,   783,  -951,  -951,   983,  -951,  -951,  -951,
    -279,   -82,  -167,  -241,  -951,  -933,  -951,   489,  -951,  -951,
    -951,  -142,  -950
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -706
static const yytype_int16 yytable[] =
{
      98,   259,   474,   994,   107,   262,   263,   404,   352,   194,
     105,   108,   109,   538,   130,   369,   546,   520,   698,   198,
     187,   898,   541,   202,   345,  1031,   504,   374,   287,   226,
    1054,   778,  1058,   791,   173,   174,   106,   469,   557,  1059,
     716,   264,   205,   629,  1071,   214,   572,   375,   254,   881,
    1265,   255,  1231,  1098,   651,   281,   381,   407,   412,   240,
     416,  1056,   938,  -222,  1074,  1102,   394,   395,  1184,   350,
    1238,   400,   608,  1232,  1238,   729,  1098,   760,  -607,   240,
     608,   343,   376,   240,   240,   618,   618,   347,   429,   234,
     618,   869,   618,   409,  -611,   507,   415,   844,   845,   745,
     267,  -604,   343,   600,   340,  -705,   240,   245,   728,   641,
     233,  -705,  1054,   574,   887,   882,   278,   819,     3,   536,
     359,  1319,   795,   539,   277,   150,  -705,   278,   883,   901,
     791,   190,   367,   278,   802,    48,   400,  -705,   860,   221,
    -705,   646,   888,   884,   340,  1208,  -705,   277,   430,   343,
      11,   356,    11,    11,   441,   442,  -605,   277,   247,   628,
     324,   635,   373,   227,   260,   562,  -492,  -606,   791,   669,
      35,    35,   248,   249,   234,  -641,   820,   998,   -85,   360,
     847,  1001,  1002,   193,  -608,   362,   516,   984,   347,  -613,
      98,   368,  -607,    98,   199,   344,   457,   388,  -642,   406,
     380,   348,   476,   383,   130,  -614,  -644,   130,  -611,   652,
     939,   573,   920,  -609,   402,  -604,   344,  1233,  1266,  1099,
    1100,   282,   382,   408,   413,  -610,   417,  1057,   439,  -222,
     730,  1103,   761,   772,  1185,  -643,  1239,   609,   411,   733,
    1280,   259,  1343,   287,  1003,   610,   418,   418,   421,   980,
     619,   686,   513,   426,   577,   875,   980,  1060,   842,   435,
     846,   636,   575,   344,    98,   909,   788,   228,   993,  1313,
    -605,   513,   791,   468,   349,    93,   637,   214,   130,  1079,
     240,  -606,  -171,   791,  -540,  -159,   995,   260,   765,  -641,
     340,   340,   513,   200,   647,   844,   845,   923,  -608,   231,
     106,   513,   348,   201,   513,   232,   245,   930,   187,   547,
     643,   644,  -642,   512,   648,   711,   712,   511,   240,   240,
    -644,   240,   829,   830,  1203,  1344,  1345,  -609,   996,   236,
     245,   237,   535,  1245,   238,   436,   534,  1302,   800,  -610,
    1026,   736,   555,   611,   670,   245,  1348,   808,  1310,  -643,
     436,    33,   715,   512,   269,  1311,  1359,   549,   823,  1272,
    1273,   426,   556,   239,  1054,   560,  1204,   679,   360,   559,
     431,   248,   249,   980,   276,  1246,   980,   980,   941,  1303,
      98,  1340,   670,   226,  1268,  1269,   805,   647,  1349,  1007,
     567,  1008,  1337,   705,   130,   248,   249,  1353,  1360,   245,
      98,   243,   857,  1083,   246,   602,  1316,   648,  1350,   244,
     248,   249,   261,   706,   130,   277,  1328,  1329,  1330,   613,
     843,   844,   845,  1316,   369,   623,   625,   283,    78,    79,
     280,    80,    81,    82,   674,   288,   106,    33,   467,    35,
     400,   738,   687,   688,   289,  1347,  -382,   707,   290,   313,
     314,   315,   466,   316,   980,   318,   319,   245,   320,   877,
     321,   689,   270,   247,   248,   249,   904,  1338,   346,   690,
     691,    33,   245,  -612,   912,  -383,   917,   273,  1285,   692,
     653,    48,   252,  -540,  1234,  1082,  1226,   353,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,  1227,
     355,  1235,   351,   780,  1236,   935,   844,   845,   841,   784,
     316,   278,   240,   361,    78,    79,  1228,    80,    81,    82,
     950,    33,   248,   249,   693,   955,   340,    33,   677,    35,
     364,   717,   365,   338,   339,   371,   694,   248,   249,   696,
     373,   701,  -491,    93,   714,   392,  1327,   981,    78,    79,
    1274,    80,    81,    82,   245,  -490,  1210,   370,    98,   436,
     702,   990,   703,   396,   397,  1047,   695,  1275,   721,    98,
    1276,    33,   130,    35,  1328,  1329,  1330,   513,  1005,   723,
     719,  1013,  -700,   130,   401,    33,   414,   848,   423,   849,
     272,   274,   275,   422,   106,   340,   753,   443,    78,    79,
     448,    80,    81,    82,    78,    79,   452,    80,    81,    82,
     453,   168,   455,   456,   465,   893,   -37,   871,   757,   248,
     249,   475,  1048,   543,   545,    98,   548,   426,   767,   107,
      11,   533,   565,    93,   208,   105,   108,   109,   783,   130,
     245,   144,   782,   554,    75,   436,    77,   457,    78,    79,
     899,    80,    81,    82,   381,   571,  1066,   569,   252,    48,
     209,   106,    78,    79,   578,    80,    81,    82,   187,   582,
      55,    56,   169,   583,   606,   791,   607,    93,    62,   322,
      33,   612,  1033,   812,   812,   696,  1254,   615,   961,   253,
     832,   617,   791,   962,   626,   963,   964,   965,   966,   967,
     968,   969,   620,   632,   437,   248,   249,   398,    98,   784,
      33,    98,  1015,   634,   791,   323,   639,   640,   833,   642,
     645,  -384,   130,   655,   654,   130,  1022,   837,   664,   657,
     660,   668,  1199,   210,   665,   667,   671,   970,   971,   681,
     972,   676,   859,  1030,   863,   683,   680,   106,   731,    33,
     144,    35,   708,    75,   684,   211,   872,    78,    79,   732,
      80,    81,    82,   744,   725,   973,   727,   746,    98,   734,
     747,   748,   107,   750,   749,   212,   756,  1197,   105,   108,
     109,   213,   130,   810,   811,    33,   758,    78,    79,   168,
      80,    81,    82,   759,   764,   895,   769,   768,   774,    33,
    1084,   776,  1023,   779,   106,   809,   793,  1089,   925,   799,
     807,   475,   818,   821,   696,   838,   822,  1032,   696,   144,
     824,   851,    75,   834,    77,   836,    78,    79,    98,    80,
      81,    82,   208,   840,   852,  1055,   924,    98,   928,   853,
     854,   855,   130,   856,   867,   431,   868,    33,   719,  1068,
     169,   130,   899,   873,   876,    93,   890,   891,   209,   896,
     894,   897,    78,    79,   902,    80,    81,    82,   905,   291,
     292,   293,  1215,   106,   285,    11,    78,    79,    33,    80,
      81,    82,   908,   911,   914,   294,   426,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   906,   316,
     696,   907,   696,   913,   915,  1195,   916,   144,   919,    33,
      75,   926,   170,   170,    78,    79,   182,    80,    81,    82,
     927,   210,   929,   961,   932,   942,   944,   947,   962,   936,
     963,   964,   965,   966,   967,   968,   969,   182,   144,   948,
     949,    75,  1259,   211,   952,    78,    79,   214,    80,    81,
      82,   953,   951,   954,   735,   957,   958,   959,  1049,   989,
     982,   991,   997,   212,  1000,  1004,  1006,   208,  1009,   213,
    1010,   596,   970,   971,  1011,   972,  1012,  1014,  1248,  1016,
    1017,  1024,   223,  1252,   596,  1019,    78,    79,  1255,    80,
      81,    82,  1020,   209,  1257,  1258,   696,  1029,  1053,  1044,
     983,    98,  1061,  1065,    98,  1067,  1069,  1072,    98,  1073,
    1078,  1081,  1080,    33,  1087,   130,    98,  1090,   130,   432,
    1094,  1092,   130,   438,  1200,  1093,   627,  1097,  1181,  1290,
     130,  1193,  1101,  1202,  1188,  1192,  1201,   106,  1205,  1211,
    1207,  1218,   214,   432,   106,   438,   432,   438,   438,  1219,
    1243,  1237,   106,  1242,  1244,  1249,  1250,   170,  1253,  1270,
    1278,  1283,  1279,   170,  1284,  1287,   210,   696,  1292,   170,
      98,    98,  1298,  -218,  1296,    98,  1295,    11,  1232,  1301,
    1198,  1213,    33,   144,   130,   130,    75,  1299,   211,   130,
      78,    79,  1307,    80,    81,    82,   182,   182,  1304,   943,
    1318,   182,  1322,  1240,  1325,  1326,   106,  1336,   212,  1334,
      11,   106,  1341,  1355,   213,  1351,   170,  1354,  1361,  1357,
    1364,  1352,  1365,  1363,   170,   170,   170,  1368,   601,  1320,
     325,   170,   515,   517,   514,   961,   804,   170,   801,  1367,
     962,   596,   963,   964,   965,   966,   967,   968,   969,   773,
    1335,  1282,   931,  1021,   596,   596,   596,  1186,  1333,    78,
      79,  1187,    80,    81,    82,  1225,   182,  1230,   604,   182,
    1051,  1339,   963,   964,   965,   966,   967,   968,   195,   899,
    1356,   240,   266,  1048,   970,   971,  1096,   972,   963,   964,
     965,   966,   967,   968,   885,   899,    11,   696,  1241,   858,
     814,    98,   910,   825,   182,    33,   918,    35,  1260,   428,
     420,   850,  1075,  1181,  1181,   130,     0,  1188,  1188,     0,
       0,     0,     0,   208,     0,     0,     0,   596,     0,   240,
       0,   171,   171,     0,    98,   183,     0,   106,    98,   170,
    1324,    98,     0,    98,    98,     0,   170,     0,   130,   209,
       0,     0,   130,     0,   961,   130,     0,   130,   130,   962,
       0,   963,   964,   965,   966,   967,   968,   969,     0,    33,
     106,     0,     0,     0,   106,     0,    98,   106,     0,   106,
     106,  1309,    78,    79,   182,    80,    81,    82,     0,   591,
     130,     0,     0,     0,     0,     0,  1323,     0,     0,     0,
       0,     0,   591,   970,   971,     0,   972,     0,     0,   558,
       0,    93,   106,     0,   596,     0,     0,     0,     0,   596,
       0,     0,   210,     0,     0,     0,   596,     0,     0,    11,
       0,  1076,     0,     0,     0,     0,     0,     0,    98,   144,
     182,   182,    75,     0,   211,     0,    78,    79,     0,    80,
      81,    82,   130,    98,     0,    31,    32,     0,   170,     0,
       0,     0,     0,     0,   212,    37,     0,   130,     0,     0,
     213,    33,     0,    35,   106,     0,   171,     0,     0,     0,
       0,     0,   171,     0,     0,     0,     0,   961,   171,   106,
       0,     0,   962,     0,   963,   964,   965,   966,   967,   968,
     969,     0,     0,     0,     0,     0,   170,     0,     0,     0,
     596,    66,    67,    68,    69,    70,     0,    33,    11,   621,
     622,     0,   588,     0,   596,     0,   596,     0,    73,    74,
       0,     0,     0,     0,    33,   171,   970,   971,   170,   972,
     170,     0,    84,   171,   171,   171,     0,     0,    78,    79,
     171,    80,    81,    82,     0,     0,   171,    89,   170,   591,
       0,     0,     0,     0,  1077,     0,     0,     0,     0,     0,
     182,   182,   591,   591,   591,   781,   961,    93,     0,     0,
       0,   962,     0,   963,   964,   965,   966,   967,   968,   969,
       0,     0,     0,     0,    78,    79,   170,    80,    81,    82,
       0,   215,     0,   182,   144,   170,   170,    75,     0,    77,
       0,    78,    79,     0,    80,    81,    82,     0,   596,     0,
     182,     0,   596,   183,   596,   970,   971,     0,   972,     0,
       0,     0,     0,   182,     0,     0,     0,     0,     0,   182,
       0,     0,     0,     0,     0,   591,     0,     0,   182,     0,
     291,   292,   293,  1209,     0,     0,     0,     0,   171,     0,
       0,     0,     0,     0,   182,   171,   294,     0,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
     316,     0,     0,     0,   596,   310,   311,   312,   313,   314,
     315,     0,   316,     0,     0,   170,     0,     0,   595,     0,
       0,     0,     0,     0,     0,     0,     0,   182,     0,   182,
       0,   595,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   591,     0,     0,     0,     0,   591,     0,     0,
       0,     0,     0,   596,   591,     0,     0,   182,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     215,   215,   596,   596,     0,   215,     0,   596,   291,   292,
     293,  1224,     0,   170,     0,     0,     0,   171,     0,     0,
     182,     0,     0,     0,   294,   938,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,     0,   316,     0,
       0,     0,     0,     0,   170,     0,     0,     0,     0,     0,
     879,     0,     0,     0,     0,   171,   170,     0,   591,     0,
     215,     0,     0,   215,     0,     0,     0,     0,     0,   182,
       0,     0,   591,     0,   591,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   182,     0,   171,     0,   171,
       0,   182,     0,     0,   170,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   596,     0,   171,   595,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   595,   595,   595,     0,     0,   596,     0,     0,     0,
       0,     0,     0,     0,     0,    27,    28,   596,     0,     0,
       0,     0,   596,     0,    33,   171,    35,     0,     0,     0,
     596,     0,   763,     0,   171,   171,     0,     0,     0,     0,
       0,     0,   182,   939,     0,  1297,   591,     0,     0,   763,
     591,    33,   591,    35,     0,     0,     0,   182,   215,   182,
     182,     0,     0,   593,   168,     0,     0,   596,   182,    33,
       0,    35,     0,     0,   595,   182,   593,   794,     0,   596,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   182,
       0,   168,   182,   183,   144,     0,     0,    75,     0,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,   168,
       0,     0,     0,    85,   215,   215,     0,     0,     0,     0,
       0,   144,   591,     0,    75,   358,    77,     0,    78,    79,
      93,    80,    81,    82,   171,   596,     0,     0,     0,   144,
       0,   596,    75,     0,    77,   596,    78,    79,   596,    80,
      81,    82,   169,   208,   182,   410,     0,    93,     0,     0,
       0,   595,     0,     0,     0,     0,   595,     0,     0,     0,
     169,   591,     0,   595,     0,    93,     0,     0,     0,   209,
       0,     0,     0,     0,     0,   291,   292,   293,     0,     0,
     591,   591,     0,     0,     0,   591,   182,     0,     0,    33,
     182,   294,   171,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,   316,  -257,     0,     0,     0,
       0,     0,     0,   593,   963,   964,   965,   966,   967,   968,
       0,     0,     0,   171,   215,   215,   593,   593,   593,     0,
       0,     0,   210,     0,     0,   171,     0,   595,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   144,
       0,   595,    75,   595,   211,     0,    78,    79,     0,    80,
      81,    82,     0,     0,   956,     0,     0,     0,     0,     0,
     960,     0,     0,   171,   212,     0,     0,     0,     0,     0,
     213,     0,     0,   591,     0,     0,     0,   215,     0,     0,
       0,     0,     0,   215,     0,     0,     0,     0,     0,   593,
     182,     0,     0,     0,   591,     0,     0,     0,     0,     0,
       0,     0,    33,     0,    35,   591,     0,     0,     0,     0,
     591,     0,     0,     0,     0,     0,     0,     0,   591,     0,
       0,     0,   650,   326,   327,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   595,     0,     0,     0,   595,
       0,   595,   168,     0,     0,     0,     0,     0,     0,  1035,
       0,     0,     0,     0,   425,   591,     0,  1043,     0,     0,
       0,   215,     0,   215,     0,     0,     0,   591,     0,   338,
     339,     0,   144,     0,     0,    75,   593,    77,     0,    78,
      79,   593,    80,    81,    82,     0,     0,     0,   593,     0,
     182,   215,     0,     0,     0,     0,     0,     0,     0,   182,
       0,     0,   182,   169,     0,     0,     0,     0,    93,     0,
       0,   595,     0,     0,     0,   182,     0,     0,     0,   291,
     292,   293,     0,   591,   215,     0,   172,   172,     0,   591,
     184,   340,     0,   591,     0,   294,   591,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,   316,
     595,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   593,     0,     0,     0,     0,     0,     0,   595,
     595,     0,     0,   215,   595,  1221,   593,     0,   593,  1043,
       0,     0,     0,     0,     0,     0,     0,   147,   149,     0,
     151,   152,   153,     0,   156,   157,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,     0,     0,   176,   179,
       0,     0,     0,   592,     0,    33,     0,   291,   292,   293,
     196,     0,   363,     0,     0,     0,   592,   204,     0,   207,
       0,     0,   220,   294,   222,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,  1036,   316,   257,     0,
       0,   172,     0,     0,   597,     0,   215,   172,  1037,     0,
     593,     0,   265,   172,   593,     0,   593,   605,     0,     0,
       0,   215,   595,   215,     0,   144,     0,     0,    75,     0,
    1038,     0,    78,    79,     0,    80,  1039,    82,     0,   215,
       0,     0,     0,   595,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   215,   595,     0,   215,     0,     0,   595,
     172,     0,     0,     0,     0,     0,     0,   595,   172,   172,
     172,     0,   354,     0,     0,   172,     0,     0,     0,     0,
       0,   172,     0,     0,     0,     0,   593,   353,   327,   328,
     329,   330,   331,   332,   333,   334,   335,   336,   337,     0,
       0,     0,     0,     0,   595,     0,     0,     0,     0,     0,
       0,     0,   378,     0,     0,   378,   595,     0,   215,     0,
       0,   196,   387,   592,   682,     0,     0,     0,     0,     0,
       0,     0,     0,   338,   339,   593,   592,   592,   592,  1035,
       0,     0,     0,     0,     0,     0,     0,     0,   184,     0,
       0,  1342,     0,     0,   593,   593,     0,     0,     0,   593,
       0,     0,     0,     0,     0,     0,   176,     0,     0,     0,
     434,     0,   595,     0,   720,     0,     0,     0,   595,     0,
       0,     0,   595,   172,     0,   595,     0,   739,   740,     0,
     172,   464,     0,     0,     0,   340,     0,     0,     0,     0,
       0,     0,   473,     0,     0,     0,     0,     0,     0,   592,
       0,     0,     0,   478,   479,   480,   482,   483,   484,   485,
     486,   487,   488,   489,   490,   491,   492,   493,   494,   495,
     496,   497,   498,   499,   500,   501,   502,   503,     0,     0,
     505,   505,   508,     0,     0,     0,     0,     0,   521,   522,
     523,   524,   525,   526,   527,   528,   529,   530,   531,   532,
     787,     0,     0,     0,     0,   505,   537,   593,   473,   505,
     540,     0,     0,     0,     0,   521,     0,     0,     0,     0,
       0,     0,     0,     0,  1261,   551,     0,   553,   593,     0,
      33,     0,    35,   473,     0,     0,   592,     0,     0,   593,
       0,   592,   172,   564,   593,     0,     0,     0,   592,     0,
       0,     0,   593,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     168,   316,   603,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   766,     0,     0,     0,     0,     0,     0,   593,
     172,     0,   864,     0,     0,     0,     0,     0,     0,   870,
     144,   593,     0,    75,     0,    77,   631,    78,    79,     0,
      80,    81,    82,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   172,     0,   172,     0,     0,     0,     0,     0,
       0,   169,   592,   215,     0,    33,    93,    35,     0,     0,
       0,     0,   172,   662,     0,     0,   592,     0,   592,   215,
       0,     0,     0,     0,     0,     0,     0,   593,     0,     0,
       0,     0,     0,   593,     0,     0,     0,   593,     0,     0,
     593,     0,     0,     0,     0,   180,     0,     0,     0,     0,
     172,   257,     0,   933,     0,     0,     0,     0,     0,   172,
     172,     0,     0,     0,     0,   678,     0,   945,     0,   946,
       0,     0,     0,     0,     0,   144,     0,     0,    75,     0,
      77,     0,    78,    79,     0,    80,    81,    82,     0,     0,
     709,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   196,     0,   291,   292,   293,   181,     0,     0,     0,
     592,    93,     0,     0,   592,     0,   592,     0,   184,   294,
       0,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,     0,   316,  -706,  -706,  -706,  -706,   308,   309,
     310,   311,   312,   313,   314,   315,     0,   316,   770,   172,
       0,  1025,     0,     0,     0,  1027,     0,  1028,     0,     0,
       0,   777,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   592,   786,     0,     0,
       0,     0,     0,     0,     0,   796,     0,     0,   797,     0,
     798,     0,     0,   473,     0,     0,     0,     0,     0,     0,
       0,     0,   473,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   172,     0,     0,
       0,     0,     0,     0,     0,   592,   827,  1091,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   592,   592,     0,     0,     0,   592,
       0,     0,     0,     0,     0,     0,     0,     0,   172,     0,
     685,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     172,     0,     0,     0,     0,     0,  1206,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     878,     0,     0,     0,     0,  1216,  1217,     0,     0,     0,
    1220,     0,     0,   892,     0,     0,     0,     0,   172,     0,
       0,     0,     0,     0,   291,   292,   293,     0,     0,     0,
       0,   473,     0,     0,     0,     0,     0,     0,     0,   473,
     294,   878,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,     0,   316,     0,     0,   592,   196,     0,
       0,     0,     0,     0,     0,     0,   937,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   592,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   592,
       0,     0,     0,     0,   592,     0,     0,     0,     0,     0,
       0,     0,   592,     0,     0,     0,   985,     0,  1251,     0,
     986,     0,   987,     0,     0,     0,   473,     0,     0,     0,
       0,     0,     0,     0,     0,   999,     0,     0,     0,  1267,
       0,     0,     0,   473,     0,     0,   291,   292,   293,   592,
    1277,     0,     0,     0,     0,  1281,     0,     0,     0,     0,
       0,   592,   294,  1288,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,   316,     0,     0,     0,
       0,   754,     0,     0,     5,     6,     7,     8,     9,     0,
    1305,     0,     0,    10,     0,     0,     0,     0,     0,     0,
       0,     0,  1312,     0,     0,     0,     0,   592,     0,     0,
       0,   473,     0,   592,     0,     0,     0,   592,     0,     0,
     592,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,  1362,     0,
      37,    38,    39,    40,  1366,    41,     0,    42,  1369,    43,
       0,  1371,    44,     0,     0,     0,    45,    46,    47,    48,
      49,    50,    51,     0,    52,    53,    54,   473,     0,     0,
      55,    56,    57,     0,    58,    59,    60,    61,    62,    63,
       0,     0,     0,   806,    64,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,    83,     0,    84,     0,     0,
       0,    85,     5,     6,     7,     8,     9,    86,    87,     0,
      88,    10,    89,    90,     0,    91,    92,   771,    93,    94,
       0,    95,    96,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,    49,    50,
      51,     0,    52,    53,    54,     0,     0,     0,    55,    56,
      57,     0,    58,    59,    60,    61,    62,    63,     0,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,    83,     0,    84,     0,     0,     0,    85,
       5,     6,     7,     8,     9,    86,    87,     0,    88,    10,
      89,    90,     0,    91,    92,   880,    93,    94,     0,    95,
      96,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,    12,
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
      82,    83,     0,    84,     0,     0,     0,    85,     5,     6,
       7,     8,     9,    86,    87,     0,    88,    10,    89,    90,
       0,    91,    92,     0,    93,    94,     0,    95,    96,     0,
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
       0,    71,     0,     0,     0,     0,   144,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,    83,
       0,    84,     0,     0,     0,    85,     5,     6,     7,     8,
       9,    86,     0,     0,     0,    10,    89,    90,     0,    91,
      92,   458,    93,    94,     0,    95,    96,     0,     0,     0,
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
       0,     0,     0,     0,   144,    73,    74,    75,    76,    77,
       0,    78,    79,     0,    80,    81,    82,    83,     0,    84,
       0,     0,     0,    85,     5,     6,     7,     8,     9,    86,
       0,     0,     0,    10,    89,    90,     0,    91,    92,   599,
      93,    94,     0,    95,    96,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    11,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,     0,     0,     0,
      31,    32,    33,    34,    35,     0,    36,     0,     0,     0,
      37,    38,    39,    40,   835,    41,     0,    42,     0,    43,
       0,     0,    44,     0,     0,     0,    45,    46,    47,    48,
       0,    50,    51,     0,    52,     0,    54,     0,     0,     0,
      55,    56,    57,     0,    58,    59,    60,     0,    62,    63,
       0,     0,     0,     0,    64,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   144,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,    83,     0,    84,     0,     0,
       0,    85,     5,     6,     7,     8,     9,    86,     0,     0,
       0,    10,    89,    90,     0,    91,    92,     0,    93,    94,
       0,    95,    96,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,   934,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,     0,     0,     0,    55,    56,
      57,     0,    58,    59,    60,     0,    62,    63,     0,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     144,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,    83,     0,    84,     0,     0,     0,    85,
       5,     6,     7,     8,     9,    86,     0,     0,     0,    10,
      89,    90,     0,    91,    92,     0,    93,    94,     0,    95,
      96,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    71,     0,     0,     0,     0,   144,    73,
      74,    75,    76,    77,     0,    78,    79,     0,    80,    81,
      82,    83,     0,    84,     0,     0,     0,    85,     5,     6,
       7,     8,     9,    86,     0,     0,     0,    10,    89,    90,
       0,    91,    92,  1095,    93,    94,     0,    95,    96,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    11,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,  1256,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,     0,     0,     0,    55,    56,    57,     0,    58,    59,
      60,     0,    62,    63,     0,     0,     0,     0,    64,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   144,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,    83,
       0,    84,     0,     0,     0,    85,     5,     6,     7,     8,
       9,    86,     0,     0,     0,    10,    89,    90,     0,    91,
      92,     0,    93,    94,     0,    95,    96,     0,     0,     0,
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
       0,     0,     0,     0,   144,    73,    74,    75,    76,    77,
       0,    78,    79,     0,    80,    81,    82,    83,     0,    84,
       0,     0,     0,    85,     5,     6,     7,     8,     9,    86,
       0,     0,     0,    10,    89,    90,     0,    91,    92,  1286,
      93,    94,     0,    95,    96,     0,     0,     0,     0,     0,
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
       0,     0,   144,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,    83,     0,    84,     0,     0,
       0,    85,     5,     6,     7,     8,     9,    86,     0,     0,
       0,    10,    89,    90,     0,    91,    92,  1289,    93,    94,
       0,    95,    96,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      11,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,  1291,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,     0,     0,     0,    55,    56,
      57,     0,    58,    59,    60,     0,    62,    63,     0,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     144,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,    83,     0,    84,     0,     0,     0,    85,
       5,     6,     7,     8,     9,    86,     0,     0,     0,    10,
      89,    90,     0,    91,    92,     0,    93,    94,     0,    95,
      96,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    71,     0,     0,     0,     0,   144,    73,
      74,    75,    76,    77,     0,    78,    79,     0,    80,    81,
      82,    83,     0,    84,     0,     0,     0,    85,     5,     6,
       7,     8,     9,    86,     0,     0,     0,    10,    89,    90,
       0,    91,    92,  1293,    93,    94,     0,    95,    96,     0,
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
       0,    71,     0,     0,     0,     0,   144,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,    83,
       0,    84,     0,     0,     0,    85,     5,     6,     7,     8,
       9,    86,     0,     0,     0,    10,    89,    90,     0,    91,
      92,  1294,    93,    94,     0,    95,    96,     0,     0,     0,
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
       0,     0,     0,     0,   144,    73,    74,    75,    76,    77,
       0,    78,    79,     0,    80,    81,    82,    83,     0,    84,
       0,     0,     0,    85,     5,     6,     7,     8,     9,    86,
       0,     0,     0,    10,    89,    90,     0,    91,    92,  1306,
      93,    94,     0,    95,    96,     0,     0,     0,     0,     0,
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
       0,     0,   144,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,    83,     0,    84,     0,     0,
       0,    85,     5,     6,     7,     8,     9,    86,     0,     0,
       0,    10,    89,    90,     0,    91,    92,  1358,    93,    94,
       0,    95,    96,     0,     0,     0,     0,     0,     0,     0,
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
     144,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,    83,     0,    84,     0,     0,     0,    85,
       5,     6,     7,     8,     9,    86,     0,     0,     0,    10,
      89,    90,     0,    91,    92,  1370,    93,    94,     0,    95,
      96,     0,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    71,     0,     0,     0,     0,   144,    73,
      74,    75,    76,    77,     0,    78,    79,     0,    80,    81,
      82,    83,     0,    84,     0,     0,     0,    85,     5,     6,
       7,     8,     9,    86,     0,     0,     0,    10,    89,    90,
       0,    91,    92,     0,    93,    94,     0,    95,    96,     0,
       0,   379,     0,     0,     0,     0,     0,     0,     0,     0,
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
       0,    71,     0,     0,     0,     0,   144,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,     0,     0,    85,     5,     6,     7,     8,
       9,    86,     0,     0,     0,    10,    89,    90,     0,    91,
      92,     0,    93,    94,     0,    95,    96,     0,     0,   566,
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
       0,     0,     0,     0,   144,    73,    74,    75,    76,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,     0,     0,    85,     5,     6,     7,     8,     9,    86,
       0,     0,     0,    10,    89,    90,     0,    91,    92,     0,
      93,    94,     0,    95,    96,     0,     0,   722,     0,     0,
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
       0,     0,   144,    73,    74,    75,    76,    77,     0,    78,
      79,     0,    80,    81,    82,     0,     0,    84,     0,     0,
       0,    85,     5,     6,     7,     8,     9,    86,     0,     0,
       0,    10,    89,    90,     0,    91,    92,     0,    93,    94,
       0,    95,    96,     0,     0,  1086,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,     0,     0,     0,    31,    32,
      33,    34,    35,     0,    36,     0,     0,     0,    37,    38,
      39,    40,     0,    41,     0,    42,     0,    43,     0,     0,
      44,     0,     0,     0,    45,    46,    47,    48,     0,    50,
      51,     0,    52,     0,    54,     0,     0,     0,     0,     0,
      57,     0,    58,    59,    60,     0,     0,     0,     0,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     144,    73,    74,    75,    76,    77,     0,    78,    79,     0,
      80,    81,    82,     0,     0,    84,     0,     0,     0,    85,
       5,     6,     7,     8,     9,    86,     0,     0,     0,    10,
      89,    90,     0,    91,    92,     0,    93,    94,     0,    95,
      96,     0,     0,  1212,     0,     0,     0,     0,     0,     0,
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
       0,     0,     0,    71,     0,     0,     0,     0,   144,    73,
      74,    75,    76,    77,     0,    78,    79,     0,    80,    81,
      82,     0,     0,    84,     0,     0,     0,    85,     5,     6,
       7,     8,     9,    86,     0,     0,     0,    10,    89,    90,
       0,    91,    92,     0,    93,    94,     0,    95,    96,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,   316,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,     0,     0,     0,    31,    32,    33,    34,    35,     0,
      36,     0,     0,     0,    37,    38,    39,    40,     0,    41,
       0,    42,     0,    43,     0,     0,    44,     0,     0,     0,
      45,    46,    47,    48,     0,    50,    51,     0,    52,     0,
      54,     0,     0,     0,     0,     0,    57,     0,    58,    59,
      60,     0,     0,     0,     0,     0,     0,     0,    64,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   144,    73,    74,    75,
      76,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,     0,     0,    85,     5,     6,     7,     8,
       9,    86,     0,     0,     0,    10,    89,    90,     0,    91,
      92,     0,    93,    94,     0,    95,    96,     0,     0,     0,
       0,     0,     0,     0,   518,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,    48,   316,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   142,     0,     0,    59,    60,     0,
       0,     0,     0,     0,     0,     0,   143,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   144,    73,    74,    75,   519,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,     0,     0,    85,     5,     6,     7,     8,     9,    86,
       0,     0,     0,    10,    89,    90,     0,     0,     0,     0,
      93,    94,     0,    95,    96,     0,     0,     0,     0,     0,
       0,     0,   175,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,  -706,  -706,  -706,  -706,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,     0,   316,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,     0,     0,    59,    60,     0,     0,     0,
       0,     0,     0,     0,   143,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   144,    73,    74,    75,     0,    77,     0,    78,
      79,     0,    80,    81,    82,     0,     0,    84,     0,     0,
       0,    85,     5,     6,     7,     8,     9,    86,     0,     0,
       0,    10,    89,    90,     0,     0,     0,     0,    93,    94,
       0,    95,    96,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     142,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,   143,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     144,    73,    74,    75,     0,    77,     0,    78,    79,     0,
      80,    81,    82,     0,     0,    84,     0,     0,     0,    85,
       5,     6,     7,     8,     9,    86,     0,     0,     0,    10,
      89,    90,     0,   203,     0,     0,    93,    94,     0,    95,
      96,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   142,     0,
       0,    59,    60,     0,     0,     0,     0,     0,     0,     0,
     143,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,   144,    73,
      74,    75,     0,    77,     0,    78,    79,     0,    80,    81,
      82,     0,     0,    84,     0,     0,     0,    85,     5,     6,
       7,     8,     9,    86,     0,     0,     0,    10,    89,    90,
       0,   206,     0,     0,    93,    94,     0,    95,    96,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   142,     0,     0,    59,
      60,     0,     0,     0,     0,     0,     0,     0,   143,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   144,    73,    74,    75,
       0,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,     0,     0,    85,     5,     6,     7,     8,
       9,    86,     0,     0,     0,    10,    89,    90,     0,   219,
       0,     0,    93,    94,     0,    95,    96,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   256,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   142,     0,     0,    59,    60,     0,
       0,     0,     0,     0,     0,     0,   143,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   144,    73,    74,    75,     0,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,     0,     0,    85,     5,     6,     7,     8,     9,    86,
       0,     0,     0,    10,    89,    90,     0,     0,     0,     0,
      93,    94,     0,    95,    96,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,     0,     0,    59,    60,     0,     0,     0,
       0,     0,     0,     0,   143,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   144,    73,    74,    75,     0,    77,     0,    78,
      79,     0,    80,    81,    82,     0,     0,    84,     0,     0,
       0,    85,     5,     6,     7,     8,     9,    86,     0,     0,
       0,    10,    89,    90,   377,     0,     0,     0,    93,    94,
       0,    95,    96,     0,     0,     0,     0,     0,     0,     0,
     470,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     142,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,   143,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     144,    73,    74,    75,     0,    77,     0,    78,    79,     0,
      80,    81,    82,     0,     0,    84,     0,     0,     0,    85,
       5,     6,     7,     8,     9,    86,     0,     0,     0,    10,
      89,    90,     0,     0,     0,     0,    93,    94,     0,    95,
      96,     0,     0,   481,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   142,     0,
       0,    59,    60,     0,     0,     0,     0,     0,     0,     0,
     143,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,   144,    73,
      74,    75,     0,    77,     0,    78,    79,     0,    80,    81,
      82,     0,     0,    84,     0,     0,     0,    85,     5,     6,
       7,     8,     9,    86,     0,     0,     0,    10,    89,    90,
       0,     0,     0,     0,    93,    94,     0,    95,    96,     0,
       0,     0,     0,     0,     0,     0,   518,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   142,     0,     0,    59,
      60,     0,     0,     0,     0,     0,     0,     0,   143,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   144,    73,    74,    75,
       0,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,     0,     0,    85,     5,     6,     7,     8,
       9,    86,     0,     0,     0,    10,    89,    90,     0,     0,
       0,     0,    93,    94,     0,    95,    96,     0,     0,     0,
       0,     0,     0,     0,   550,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   142,     0,     0,    59,    60,     0,
       0,     0,     0,     0,     0,     0,   143,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   144,    73,    74,    75,     0,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,     0,     0,    85,     5,     6,     7,     8,     9,    86,
       0,     0,     0,    10,    89,    90,     0,     0,     0,     0,
      93,    94,     0,    95,    96,     0,     0,     0,     0,     0,
       0,     0,   552,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,    34,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,     0,     0,    59,    60,     0,     0,     0,
       0,     0,     0,     0,   143,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   144,    73,    74,    75,     0,    77,     0,    78,
      79,     0,    80,    81,    82,     0,     0,    84,     0,     0,
       0,    85,     5,     6,     7,     8,     9,    86,     0,     0,
       0,    10,    89,    90,     0,     0,     0,     0,    93,    94,
       0,    95,    96,     0,     0,     0,     0,     0,     0,     0,
     785,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    13,     0,     0,     0,     0,    14,     0,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,     0,     0,     0,     0,    31,    32,
      33,    34,    35,     0,     0,     0,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    48,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     142,     0,     0,    59,    60,     0,     0,     0,     0,     0,
       0,     0,   143,    65,    66,    67,    68,    69,    70,     0,
       0,     0,     0,     0,     0,    71,     0,     0,     0,     0,
     144,    73,    74,    75,     0,    77,     0,    78,    79,     0,
      80,    81,    82,     0,     0,    84,     0,     0,     0,    85,
       5,     6,     7,     8,     9,    86,     0,     0,     0,    10,
      89,    90,     0,     0,     0,     0,    93,    94,     0,    95,
      96,     0,     0,     0,     0,     0,     0,     0,   826,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    12,
      13,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,     0,     0,     0,     0,    31,    32,    33,    34,
      35,     0,     0,     0,     0,     0,    37,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    48,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   142,     0,
       0,    59,    60,     0,     0,     0,     0,     0,     0,     0,
     143,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,     0,    71,     0,     0,     0,     0,   144,    73,
      74,    75,     0,    77,     0,    78,    79,     0,    80,    81,
      82,     0,     0,    84,     0,     0,     0,    85,     5,     6,
       7,     8,     9,    86,     0,     0,     0,    10,    89,    90,
       0,     0,     0,     0,    93,    94,     0,    95,    96,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,     0,
       0,     0,     0,    14,     0,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
       0,     0,     0,     0,    31,    32,    33,    34,    35,     0,
       0,     0,     0,     0,    37,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    48,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   142,     0,     0,    59,
      60,     0,     0,     0,     0,     0,     0,     0,   143,    65,
      66,    67,    68,    69,    70,     0,     0,     0,     0,     0,
       0,    71,     0,     0,     0,     0,   144,    73,    74,    75,
     519,    77,     0,    78,    79,     0,    80,    81,    82,     0,
       0,    84,     0,     0,     0,    85,     5,     6,     7,     8,
       9,    86,     0,     0,     0,    10,    89,    90,     0,     0,
       0,     0,    93,    94,     0,    95,    96,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    12,    13,     0,     0,     0,
       0,    14,     0,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,     0,     0,
       0,     0,    31,    32,    33,    34,    35,     0,     0,     0,
       0,     0,    37,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    48,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   142,     0,     0,    59,    60,     0,
       0,     0,     0,     0,     0,     0,   143,    65,    66,    67,
      68,    69,    70,     0,     0,     0,     0,     0,     0,    71,
       0,     0,     0,     0,   144,    73,    74,    75,     0,    77,
       0,    78,    79,     0,    80,    81,    82,     0,     0,    84,
       0,     0,     0,    85,     5,     6,     7,     8,     9,    86,
       0,     0,     0,    10,    89,    90,     0,     0,     0,     0,
      93,    94,     0,    95,    96,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,     0,     0,     0,     0,    14,
       0,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,     0,     0,     0,     0,
      31,    32,    33,   433,    35,     0,     0,     0,     0,     0,
      37,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   142,     0,     0,    59,    60,     0,     0,     0,
       0,     0,     0,     0,   143,    65,    66,    67,    68,    69,
      70,     0,     0,     0,     0,     0,     0,    71,     0,     0,
       0,     0,   144,    73,    74,    75,     0,    77,     0,    78,
      79,     0,    80,    81,    82,     0,     0,    84,     0,     0,
       0,    85,  1104,  1105,  1106,  1107,  1108,    86,  1109,  1110,
    1111,  1112,    89,    90,     0,     0,     0,     0,    93,    94,
       0,    95,    96,   294,     0,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   315,     0,   316,  1113,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,  1114,  1115,  1116,  1117,  1118,  1119,  1120,     0,     0,
      33,     0,     0,     0,     0,     0,     0,     0,     0,  1121,
    1122,  1123,  1124,  1125,  1126,  1127,  1128,  1129,  1130,  1131,
    1132,  1133,  1134,  1135,  1136,  1137,  1138,  1139,  1140,  1141,
    1142,  1143,  1144,  1145,  1146,  1147,  1148,  1149,  1150,  1151,
    1152,  1153,  1154,  1155,  1156,  1157,  1158,  1159,  1160,  1161,
       0,     0,  1162,  1163,  1164,  1165,  1166,  1167,  1168,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    1169,  1170,  1171,     0,  1172,     0,     0,    78,    79,     0,
      80,    81,    82,  1173,  1174,  1175,     0,     0,  1176,   291,
     292,   293,     0,     0,     0,  1177,  1178,     0,  1179,     0,
    1180,     0,     0,     0,     0,   294,     0,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,   316,
     291,   292,   293,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   294,     0,   295,   296,
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
     315,     0,   316,   291,   292,   293,   828,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   294,
       0,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,     0,   316,   291,   292,   293,   988,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     294,     0,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   304,   305,   306,   307,   308,   309,   310,   311,   312,
     313,   314,   315,     0,   316,   291,   292,   293,  1063,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   294,     0,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,   316,   291,   292,   293,  1064,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   294,     0,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,   316,     0,   317,     0,
     291,   292,   293,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   294,     0,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   391,
     316,   291,   292,   293,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   294,     0,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     393,   316,   291,   292,   293,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   294,     0,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   403,   316,     0,   584,   585,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   586,     0,     0,     0,     0,   291,   292,
     293,    31,    32,    33,     0,     0,     0,     0,     0,     0,
       0,    37,     0,     0,   294,   405,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   305,   306,   307,   308,
     309,   310,   311,   312,   313,   314,   315,     0,   316,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   542,   587,    66,    67,    68,
      69,    70,     0,     0,     0,     0,     0,     0,   588,     0,
       0,     0,     0,   144,    73,    74,    75,     0,   589,     0,
      78,    79,     0,    80,    81,    82,     0,     0,    84,     0,
       0,     0,     0,     0,     0,     0,   561,     0,   590,   291,
     292,   293,     0,    89,     0,     0,     0,     0,     0,     0,
       0,   444,     0,     0,     0,   294,     0,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,   316,
     291,   292,   293,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   294,     0,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
     316,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   291,   292,   293,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   294,   666,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,     0,   316,   291,   292,   293,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   294,   704,   295,   296,   297,   298,   299,   300,
     301,   302,   303,   304,   305,   306,   307,   308,   309,   310,
     311,   312,   313,   314,   315,     0,   316,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,     0,   316,
       0,     0,   291,   292,   293,     0,     0,     0,   903,     0,
       0,     0,     0,     0,     0,     0,     0,   570,   294,   663,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,     0,   316,   291,   292,   293,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   294,
       0,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,     0,   316,   292,   293,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   294,
       0,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   293,   316,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   294,     0,   295,   296,
     297,   298,   299,   300,   301,   302,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,     0,
     316,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,     0,   316,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,     0,   316
};

static const yytype_int16 yycheck[] =
{
       4,    83,   281,   897,     4,    87,    88,   221,   146,    30,
       4,     4,     4,   346,     4,   182,   357,   326,   543,    40,
      28,   790,   351,    44,   131,   958,   316,   188,   110,    53,
     977,   640,   980,   658,    24,    25,     4,   278,   371,   980,
     569,    90,    46,   440,   994,    49,     8,   188,    72,   774,
       8,    75,     8,     8,     8,     8,     8,     8,     8,    63,
       8,     8,    26,     8,   997,     8,   208,   209,     8,   138,
       8,   213,     8,    29,     8,     8,     8,     8,    61,    83,
       8,    61,   189,    87,    88,     8,     8,    61,    61,    57,
       8,   755,     8,    90,    61,   319,   234,    93,    94,    78,
      90,    61,    61,    90,   121,   140,   110,    73,   575,   450,
      73,   140,  1059,    26,   147,   147,   162,    78,     0,   343,
     169,   102,   661,   347,   144,   172,   172,   162,   160,   793,
     755,   172,   181,   162,   673,    98,   278,   172,   747,   175,
     175,   121,   175,   175,   121,  1078,   175,   144,   121,    61,
      41,   155,    41,    41,   174,   175,    61,   144,   137,   176,
     128,    46,   121,    73,   146,   379,   140,    61,   793,   510,
      73,    73,   138,   139,   142,    61,   137,   902,   159,   169,
     176,   906,   907,   172,    61,   175,   324,    46,    61,   172,
     194,   181,   175,   197,   172,   175,   173,   201,    61,   223,
     194,   175,   284,   197,   194,   172,    61,   197,   175,   163,
     174,   173,   821,    61,   218,   175,   175,   173,   176,   174,
     175,   174,   174,   174,   174,    61,   174,   174,   252,   174,
     163,   174,   163,   630,   174,    61,   174,   173,   228,   580,
     174,   323,   174,   325,   908,   173,   236,   237,   238,   874,
     173,   173,   321,   243,   396,   173,   881,   173,   725,   249,
     727,   146,   175,   175,   268,   804,   157,   177,   157,   157,
     175,   340,   897,   277,   177,   177,   161,   281,   268,  1004,
     284,   175,   173,   908,   173,   173,    31,   146,   617,   175,
     121,   121,   361,   172,   455,    93,    94,   822,   175,   117,
     268,   370,   175,   172,   373,   117,    73,   836,   316,   358,
     452,   453,   175,   321,   455,    66,    67,   321,   322,   323,
     175,   325,    66,    67,    31,   174,   175,   175,    73,   172,
      73,   172,   340,    31,   172,    78,   340,    31,   671,   175,
     949,   582,   173,   173,   511,    73,    31,   680,  1296,   175,
      78,    71,   566,   361,   175,  1296,    31,   361,   699,  1231,
    1232,   351,   370,   172,  1311,   373,    73,   534,   358,   373,
     137,   138,   139,   998,    26,    73,  1001,  1002,   176,    73,
     384,  1331,   549,   407,  1227,  1228,   676,   548,    73,   914,
     384,   916,  1325,   554,   384,   138,   139,  1347,    73,    73,
     404,   172,   743,  1012,    78,   409,  1300,   548,  1341,   172,
     138,   139,   172,   554,   404,   144,   106,   107,   108,   423,
      92,    93,    94,  1317,   591,   429,   430,    31,   148,   149,
     172,   151,   152,   153,   516,   174,   404,    71,   181,    73,
     582,   583,    42,    43,   174,  1339,    61,   554,   174,    45,
      46,    47,   180,    49,  1079,    61,    61,    73,   175,   768,
     140,    61,    78,   137,   138,   139,   799,   157,   172,    69,
      70,    71,    73,   172,   807,    61,   817,    78,  1247,    79,
     470,    98,   144,   173,    25,  1010,    13,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    26,
      40,    42,   172,   645,    45,    92,    93,    94,   722,   651,
      49,   162,   516,   140,   148,   149,    43,   151,   152,   153,
     861,    71,   138,   139,   124,   866,   121,    71,   518,    73,
     179,    31,     8,    59,    60,   172,   136,   138,   139,   543,
     121,   545,   140,   177,   565,   174,  1315,   876,   148,   149,
      25,   151,   152,   153,    73,   140,  1081,   140,   562,    78,
     550,   894,   552,   162,   172,   115,   166,    42,   572,   573,
      45,    71,   562,    73,   106,   107,   108,   646,   911,   573,
     570,   922,    13,   573,    13,    71,    13,   729,   162,   731,
      94,    95,    96,   173,   562,   121,   600,   174,   148,   149,
      79,   151,   152,   153,   148,   149,    13,   151,   152,   153,
      90,   111,   173,   173,   178,   782,   172,   759,   608,   138,
     139,   172,   172,   172,     8,   629,   173,   617,   618,   629,
      41,   175,    82,   177,    25,   629,   629,   629,   646,   629,
      73,   141,   646,   173,   144,    78,   146,   173,   148,   149,
     792,   151,   152,   153,     8,    13,   989,   174,   144,    98,
      51,   629,   148,   149,    79,   151,   152,   153,   676,   172,
     109,   110,   172,     8,   172,  1300,    73,   177,   117,   118,
      71,   174,   961,   687,   688,   689,  1211,   119,    99,   175,
     711,   172,  1317,   104,    61,   106,   107,   108,   109,   110,
     111,   112,   173,   120,   137,   138,   139,    98,   712,   851,
      71,   715,   926,   161,  1339,   154,   122,     8,   712,   173,
      13,    61,   712,     8,   173,   715,   940,   717,   179,    13,
     119,     8,  1065,   124,   179,   176,   172,   148,   149,   179,
     151,    63,   746,   957,   748,   173,   172,   715,   122,    71,
     141,    73,   173,   144,   179,   146,   760,   148,   149,     8,
     151,   152,   153,   172,   174,   176,   174,   140,   772,   173,
     172,   140,   772,   102,   175,   166,    13,  1056,   772,   772,
     772,   172,   772,    69,    70,    71,   174,   148,   149,   111,
     151,   152,   153,    90,   175,   785,   179,    13,   175,    71,
    1014,    13,   944,   174,   772,    13,   172,  1021,   829,   172,
     172,   172,   172,   172,   818,   122,     8,   959,   822,   141,
     173,     8,   144,   174,   146,   174,   148,   149,   832,   151,
     152,   153,    25,    13,   173,   977,   826,   841,   832,   173,
     122,   179,   832,     8,   172,   137,   159,    71,   838,   991,
     172,   841,   994,     8,   172,   177,   122,   174,    51,   173,
     172,     8,   148,   149,   175,   151,   152,   153,   173,     9,
      10,    11,  1086,   841,   146,    41,   148,   149,    71,   151,
     152,   153,   172,   172,   122,    25,   876,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,   175,    49,
     914,   175,   916,   173,   179,  1053,     8,   141,   137,    71,
     144,    26,    24,    25,   148,   149,    28,   151,   152,   153,
      68,   124,   174,    99,   173,   163,    26,   173,   104,   174,
     106,   107,   108,   109,   110,   111,   112,    49,   141,   122,
       8,   144,   176,   146,   122,   148,   149,   961,   151,   152,
     153,   176,   173,     8,   157,   175,   173,    90,   972,   172,
     176,    26,   173,   166,   173,   175,   173,    25,   173,   172,
     122,   401,   148,   149,   173,   151,     8,    26,  1202,   174,
     173,   122,   144,  1207,   414,   174,   148,   149,  1212,   151,
     152,   153,   174,    51,  1218,  1219,  1010,    73,   976,   150,
     176,  1015,   146,   172,  1018,   173,   104,    73,  1022,    13,
     173,   122,   173,    71,  1018,  1015,  1030,   173,  1018,   246,
     173,   122,  1022,   250,   173,   122,   176,    13,  1042,  1253,
    1030,    73,   175,   175,  1048,   174,   172,  1015,    13,   122,
     175,   175,  1056,   270,  1022,   272,   273,   274,   275,   175,
    1198,    13,  1030,    13,   173,    73,    13,   169,   175,    51,
      73,    73,   172,   175,     8,    13,   124,  1081,   174,   181,
    1084,  1085,   153,    90,    90,  1089,   140,    41,    29,   172,
    1058,  1085,    71,   141,  1084,  1085,   144,    13,   146,  1089,
     148,   149,   174,   151,   152,   153,   208,   209,   173,   157,
      73,   213,   155,  1195,   173,     8,  1084,   174,   166,   172,
      41,  1089,   173,    13,   172,    73,   228,    73,    13,   173,
      73,  1345,    13,   175,   236,   237,   238,    13,   407,  1308,
     128,   243,   323,   325,   322,    99,   675,   249,   672,  1363,
     104,   571,   106,   107,   108,   109,   110,   111,   112,   632,
    1322,  1243,   838,   937,   584,   585,   586,   146,  1317,   148,
     149,   150,   151,   152,   153,  1102,   278,  1184,   412,   281,
     972,  1326,   106,   107,   108,   109,   110,   111,    38,  1331,
    1350,  1195,    90,   172,   148,   149,  1031,   151,   106,   107,
     108,   109,   110,   111,   775,  1347,    41,  1211,  1196,   744,
     688,  1215,   805,   701,   316,    71,   818,    73,  1222,   244,
     237,   732,   176,  1227,  1228,  1215,    -1,  1231,  1232,    -1,
      -1,    -1,    -1,    25,    -1,    -1,    -1,   657,    -1,  1243,
      -1,    24,    25,    -1,  1248,    28,    -1,  1215,  1252,   351,
     174,  1255,    -1,  1257,  1258,    -1,   358,    -1,  1248,    51,
      -1,    -1,  1252,    -1,    99,  1255,    -1,  1257,  1258,   104,
      -1,   106,   107,   108,   109,   110,   111,   112,    -1,    71,
    1248,    -1,    -1,    -1,  1252,    -1,  1290,  1255,    -1,  1257,
    1258,  1295,   148,   149,   396,   151,   152,   153,    -1,   401,
    1290,    -1,    -1,    -1,    -1,    -1,  1310,    -1,    -1,    -1,
      -1,    -1,   414,   148,   149,    -1,   151,    -1,    -1,   175,
      -1,   177,  1290,    -1,   744,    -1,    -1,    -1,    -1,   749,
      -1,    -1,   124,    -1,    -1,    -1,   756,    -1,    -1,    41,
      -1,   176,    -1,    -1,    -1,    -1,    -1,    -1,  1352,   141,
     452,   453,   144,    -1,   146,    -1,   148,   149,    -1,   151,
     152,   153,  1352,  1367,    -1,    69,    70,    -1,   470,    -1,
      -1,    -1,    -1,    -1,   166,    79,    -1,  1367,    -1,    -1,
     172,    71,    -1,    73,  1352,    -1,   169,    -1,    -1,    -1,
      -1,    -1,   175,    -1,    -1,    -1,    -1,    99,   181,  1367,
      -1,    -1,   104,    -1,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,    -1,   518,    -1,    -1,    -1,
     840,   125,   126,   127,   128,   129,    -1,    71,    41,    73,
      74,    -1,   136,    -1,   854,    -1,   856,    -1,   142,   143,
      -1,    -1,    -1,    -1,    71,   228,   148,   149,   550,   151,
     552,    -1,   156,   236,   237,   238,    -1,    -1,   148,   149,
     243,   151,   152,   153,    -1,    -1,   249,   171,   570,   571,
      -1,    -1,    -1,    -1,   176,    -1,    -1,    -1,    -1,    -1,
     582,   583,   584,   585,   586,   175,    99,   177,    -1,    -1,
      -1,   104,    -1,   106,   107,   108,   109,   110,   111,   112,
      -1,    -1,    -1,    -1,   148,   149,   608,   151,   152,   153,
      -1,    49,    -1,   615,   141,   617,   618,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,   948,    -1,
     632,    -1,   952,   316,   954,   148,   149,    -1,   151,    -1,
      -1,    -1,    -1,   645,    -1,    -1,    -1,    -1,    -1,   651,
      -1,    -1,    -1,    -1,    -1,   657,    -1,    -1,   660,    -1,
       9,    10,    11,   176,    -1,    -1,    -1,    -1,   351,    -1,
      -1,    -1,    -1,    -1,   676,   358,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,  1024,    42,    43,    44,    45,    46,
      47,    -1,    49,    -1,    -1,   717,    -1,    -1,   401,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   729,    -1,   731,
      -1,   414,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   744,    -1,    -1,    -1,    -1,   749,    -1,    -1,
      -1,    -1,    -1,  1073,   756,    -1,    -1,   759,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     208,   209,  1092,  1093,    -1,   213,    -1,  1097,     9,    10,
      11,  1101,    -1,   785,    -1,    -1,    -1,   470,    -1,    -1,
     792,    -1,    -1,    -1,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,   826,    -1,    -1,    -1,    -1,    -1,
     179,    -1,    -1,    -1,    -1,   518,   838,    -1,   840,    -1,
     278,    -1,    -1,   281,    -1,    -1,    -1,    -1,    -1,   851,
      -1,    -1,   854,    -1,   856,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   867,    -1,   550,    -1,   552,
      -1,   873,    -1,    -1,   876,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1205,    -1,   570,   571,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   584,   585,   586,    -1,    -1,  1226,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    62,    63,  1237,    -1,    -1,
      -1,    -1,  1242,    -1,    71,   608,    73,    -1,    -1,    -1,
    1250,    -1,   615,    -1,   617,   618,    -1,    -1,    -1,    -1,
      -1,    -1,   944,   174,    -1,  1265,   948,    -1,    -1,   632,
     952,    71,   954,    73,    -1,    -1,    -1,   959,   396,   961,
     962,    -1,    -1,   401,   111,    -1,    -1,  1287,   970,    71,
      -1,    73,    -1,    -1,   657,   977,   414,   660,    -1,  1299,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   991,
      -1,   111,   994,   676,   141,    -1,    -1,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   111,
      -1,    -1,    -1,   160,   452,   453,    -1,    -1,    -1,    -1,
      -1,   141,  1024,    -1,   144,   172,   146,    -1,   148,   149,
     177,   151,   152,   153,   717,  1355,    -1,    -1,    -1,   141,
      -1,  1361,   144,    -1,   146,  1365,   148,   149,  1368,   151,
     152,   153,   172,    25,  1056,   175,    -1,   177,    -1,    -1,
      -1,   744,    -1,    -1,    -1,    -1,   749,    -1,    -1,    -1,
     172,  1073,    -1,   756,    -1,   177,    -1,    -1,    -1,    51,
      -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,
    1092,  1093,    -1,    -1,    -1,  1097,  1098,    -1,    -1,    71,
    1102,    25,   785,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,    98,    -1,    -1,    -1,
      -1,    -1,    -1,   571,   106,   107,   108,   109,   110,   111,
      -1,    -1,    -1,   826,   582,   583,   584,   585,   586,    -1,
      -1,    -1,   124,    -1,    -1,   838,    -1,   840,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   141,
      -1,   854,   144,   856,   146,    -1,   148,   149,    -1,   151,
     152,   153,    -1,    -1,   867,    -1,    -1,    -1,    -1,    -1,
     873,    -1,    -1,   876,   166,    -1,    -1,    -1,    -1,    -1,
     172,    -1,    -1,  1205,    -1,    -1,    -1,   645,    -1,    -1,
      -1,    -1,    -1,   651,    -1,    -1,    -1,    -1,    -1,   657,
    1222,    -1,    -1,    -1,  1226,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    71,    -1,    73,  1237,    -1,    -1,    -1,    -1,
    1242,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1250,    -1,
      -1,    -1,   176,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   948,    -1,    -1,    -1,   952,
      -1,   954,   111,    -1,    -1,    -1,    -1,    -1,    -1,   962,
      -1,    -1,    -1,    -1,   123,  1287,    -1,   970,    -1,    -1,
      -1,   729,    -1,   731,    -1,    -1,    -1,  1299,    -1,    59,
      60,    -1,   141,    -1,    -1,   144,   744,   146,    -1,   148,
     149,   749,   151,   152,   153,    -1,    -1,    -1,   756,    -1,
    1322,   759,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1331,
      -1,    -1,  1334,   172,    -1,    -1,    -1,    -1,   177,    -1,
      -1,  1024,    -1,    -1,    -1,  1347,    -1,    -1,    -1,     9,
      10,    11,    -1,  1355,   792,    -1,    24,    25,    -1,  1361,
      28,   121,    -1,  1365,    -1,    25,  1368,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
    1073,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   840,    -1,    -1,    -1,    -1,    -1,    -1,  1092,
    1093,    -1,    -1,   851,  1097,  1098,   854,    -1,   856,  1102,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     5,     6,    -1,
       8,     9,    10,    -1,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    -1,    -1,    26,    27,
      -1,    -1,    -1,   401,    -1,    71,    -1,     9,    10,    11,
      38,    -1,   122,    -1,    -1,    -1,   414,    45,    -1,    47,
      -1,    -1,    50,    25,    52,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,   112,    49,    76,    -1,
      -1,   169,    -1,    -1,   401,    -1,   944,   175,   124,    -1,
     948,    -1,    90,   181,   952,    -1,   954,   414,    -1,    -1,
      -1,   959,  1205,   961,    -1,   141,    -1,    -1,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,   977,
      -1,    -1,    -1,  1226,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   991,  1237,    -1,   994,    -1,    -1,  1242,
     228,    -1,    -1,    -1,    -1,    -1,    -1,  1250,   236,   237,
     238,    -1,   150,    -1,    -1,   243,    -1,    -1,    -1,    -1,
      -1,   249,    -1,    -1,    -1,    -1,  1024,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      -1,    -1,    -1,    -1,  1287,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   190,    -1,    -1,   193,  1299,    -1,  1056,    -1,
      -1,   199,   200,   571,   176,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,  1073,   584,   585,   586,  1322,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   316,    -1,
      -1,  1334,    -1,    -1,  1092,  1093,    -1,    -1,    -1,  1097,
      -1,    -1,    -1,    -1,    -1,    -1,   244,    -1,    -1,    -1,
     248,    -1,  1355,    -1,   571,    -1,    -1,    -1,  1361,    -1,
      -1,    -1,  1365,   351,    -1,  1368,    -1,   584,   585,    -1,
     358,   269,    -1,    -1,    -1,   121,    -1,    -1,    -1,    -1,
      -1,    -1,   280,    -1,    -1,    -1,    -1,    -1,    -1,   657,
      -1,    -1,    -1,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,    -1,    -1,
     318,   319,   320,    -1,    -1,    -1,    -1,    -1,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     657,    -1,    -1,    -1,    -1,   343,   344,  1205,   346,   347,
     348,    -1,    -1,    -1,    -1,   353,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1222,   363,    -1,   365,  1226,    -1,
      71,    -1,    73,   371,    -1,    -1,   744,    -1,    -1,  1237,
      -1,   749,   470,   381,  1242,    -1,    -1,    -1,   756,    -1,
      -1,    -1,  1250,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
     111,    49,   410,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,  1287,
     518,    -1,   749,    -1,    -1,    -1,    -1,    -1,    -1,   756,
     141,  1299,    -1,   144,    -1,   146,   444,   148,   149,    -1,
     151,   152,   153,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   550,    -1,   552,    -1,    -1,    -1,    -1,    -1,
      -1,   172,   840,  1331,    -1,    71,   177,    73,    -1,    -1,
      -1,    -1,   570,   481,    -1,    -1,   854,    -1,   856,  1347,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,  1355,    -1,    -1,
      -1,    -1,    -1,  1361,    -1,    -1,    -1,  1365,    -1,    -1,
    1368,    -1,    -1,    -1,    -1,   111,    -1,    -1,    -1,    -1,
     608,   519,    -1,   840,    -1,    -1,    -1,    -1,    -1,   617,
     618,    -1,    -1,    -1,    -1,   533,    -1,   854,    -1,   856,
      -1,    -1,    -1,    -1,    -1,   141,    -1,    -1,   144,    -1,
     146,    -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,
     558,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   569,    -1,     9,    10,    11,   172,    -1,    -1,    -1,
     948,   177,    -1,    -1,   952,    -1,   954,    -1,   676,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,   626,   717,
      -1,   948,    -1,    -1,    -1,   952,    -1,   954,    -1,    -1,
      -1,   639,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1024,   655,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   663,    -1,    -1,   666,    -1,
     668,    -1,    -1,   671,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   680,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   785,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,  1073,   704,  1024,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1092,  1093,    -1,    -1,    -1,  1097,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   826,    -1,
     176,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     838,    -1,    -1,    -1,    -1,    -1,  1073,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     768,    -1,    -1,    -1,    -1,  1092,  1093,    -1,    -1,    -1,
    1097,    -1,    -1,   781,    -1,    -1,    -1,    -1,   876,    -1,
      -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,    -1,
      -1,   799,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   807,
      25,   809,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    -1,    49,    -1,    -1,  1205,   836,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   844,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1226,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1237,
      -1,    -1,    -1,    -1,  1242,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1250,    -1,    -1,    -1,   884,    -1,  1205,    -1,
     888,    -1,   890,    -1,    -1,    -1,   894,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   903,    -1,    -1,    -1,  1226,
      -1,    -1,    -1,   911,    -1,    -1,     9,    10,    11,  1287,
    1237,    -1,    -1,    -1,    -1,  1242,    -1,    -1,    -1,    -1,
      -1,  1299,    25,  1250,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    -1,    -1,    -1,
      -1,   176,    -1,    -1,     3,     4,     5,     6,     7,    -1,
    1287,    -1,    -1,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,  1299,    -1,    -1,    -1,    -1,  1355,    -1,    -1,
      -1,   989,    -1,  1361,    -1,    -1,    -1,  1365,    -1,    -1,
    1368,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,  1355,    -1,
      79,    80,    81,    82,  1361,    84,    -1,    86,  1365,    88,
      -1,  1368,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      99,   100,   101,    -1,   103,   104,   105,  1065,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,   176,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,   167,    -1,
     169,    12,   171,   172,    -1,   174,   175,   176,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    99,   100,
     101,    -1,   103,   104,   105,    -1,    -1,    -1,   109,   110,
     111,    -1,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,   167,    -1,   169,    12,
     171,   172,    -1,   174,   175,   176,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    41,    42,
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
     153,   154,    -1,   156,    -1,    -1,    -1,   160,     3,     4,
       5,     6,     7,   166,   167,    -1,   169,    12,   171,   172,
      -1,   174,   175,    -1,   177,   178,    -1,   180,   181,    -1,
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
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,   176,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,   176,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    75,    -1,    -1,    -1,
      79,    80,    81,    82,    83,    84,    -1,    86,    -1,    88,
      -1,    -1,    91,    -1,    -1,    -1,    95,    96,    97,    98,
      -1,   100,   101,    -1,   103,    -1,   105,    -1,    -1,    -1,
     109,   110,   111,    -1,   113,   114,   115,    -1,   117,   118,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,   145,   146,    -1,   148,
     149,    -1,   151,   152,   153,   154,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,   174,   175,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    -1,    86,    -1,    88,    89,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,   176,   177,   178,    -1,   180,   181,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    41,    42,    43,    -1,
      -1,    -1,    -1,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    -1,    -1,    69,    70,    71,    72,    73,    -1,
      75,    -1,    -1,    -1,    79,    80,    81,    82,    -1,    84,
      -1,    86,    87,    88,    -1,    -1,    91,    -1,    -1,    -1,
      95,    96,    97,    98,    -1,   100,   101,    -1,   103,    -1,
     105,    -1,    -1,    -1,   109,   110,   111,    -1,   113,   114,
     115,    -1,   117,   118,    -1,    -1,    -1,    -1,   123,   124,
     125,   126,   127,   128,   129,    -1,    -1,    -1,    -1,    -1,
      -1,   136,    -1,    -1,    -1,    -1,   141,   142,   143,   144,
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,   154,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,   176,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
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
      -1,    12,   171,   172,    -1,   174,   175,   176,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      41,    42,    43,    -1,    -1,    -1,    -1,    48,    -1,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    -1,    -1,    69,    70,
      71,    72,    73,    -1,    75,    -1,    -1,    -1,    79,    80,
      81,    82,    -1,    84,    85,    86,    -1,    88,    -1,    -1,
      91,    -1,    -1,    -1,    95,    96,    97,    98,    -1,   100,
     101,    -1,   103,    -1,   105,    -1,    -1,    -1,   109,   110,
     111,    -1,   113,   114,   115,    -1,   117,   118,    -1,    -1,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,    -1,    -1,
     141,   142,   143,   144,   145,   146,    -1,   148,   149,    -1,
     151,   152,   153,   154,    -1,   156,    -1,    -1,    -1,   160,
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,   176,   177,   178,    -1,   180,   181,    -1,
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
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,   176,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,   176,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
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
      -1,    12,   171,   172,    -1,   174,   175,   176,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,   176,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,    -1,   177,   178,    -1,   180,   181,    -1,
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
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    26,
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
      -1,    -1,    -1,    12,   171,   172,    -1,   174,   175,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    26,    -1,    -1,
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
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,   174,   175,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    26,    -1,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,   175,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
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
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,   175,    -1,   177,   178,    -1,   180,   181,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    42,    43,    -1,
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
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
     175,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    98,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,   145,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    42,    43,    -1,    -1,    -1,    -1,    48,
      -1,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    -1,    -1,    -1,    -1,
      69,    70,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      79,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    98,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   111,    -1,    -1,   114,   115,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,   124,   125,   126,   127,   128,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,   148,
     149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,    -1,
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,   174,    -1,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,   174,    -1,    -1,   177,   178,    -1,   180,   181,    -1,
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
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,   174,
      -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    42,    43,    -1,    -1,    -1,
      -1,    48,    -1,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    -1,    -1,
      -1,    -1,    69,    70,    71,    72,    73,    -1,    -1,    -1,
      -1,    -1,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    95,    -1,
      -1,    98,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   111,    -1,    -1,   114,   115,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   123,   124,   125,   126,
     127,   128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,
      -1,   148,   149,    -1,   151,   152,   153,    -1,    -1,   156,
      -1,    -1,    -1,   160,     3,     4,     5,     6,     7,   166,
      -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
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
      -1,   160,     3,     4,     5,     6,     7,   166,    -1,    -1,
      -1,    12,   171,   172,   173,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
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
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,    -1,
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
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
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
      -1,    12,   171,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
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
       3,     4,     5,     6,     7,   166,    -1,    -1,    -1,    12,
     171,   172,    -1,    -1,    -1,    -1,   177,   178,    -1,   180,
     181,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,
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
       5,     6,     7,   166,    -1,    -1,    -1,    12,   171,   172,
      -1,    -1,    -1,    -1,   177,   178,    -1,   180,   181,    -1,
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
     145,   146,    -1,   148,   149,    -1,   151,   152,   153,    -1,
      -1,   156,    -1,    -1,    -1,   160,     3,     4,     5,     6,
       7,   166,    -1,    -1,    -1,    12,   171,   172,    -1,    -1,
      -1,    -1,   177,   178,    -1,   180,   181,    -1,    -1,    -1,
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
      -1,    -1,    -1,    12,   171,   172,    -1,    -1,    -1,    -1,
     177,   178,    -1,   180,   181,    -1,    -1,    -1,    -1,    -1,
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
      -1,   160,     3,     4,     5,     6,     7,   166,     9,    10,
      11,    12,   171,   172,    -1,    -1,    -1,    -1,   177,   178,
      -1,   180,   181,    25,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    -1,    49,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    62,    63,    64,    65,    66,    67,    68,    -1,    -1,
      71,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      -1,    -1,   123,   124,   125,   126,   127,   128,   129,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     141,   142,   143,    -1,   145,    -1,    -1,   148,   149,    -1,
     151,   152,   153,   154,   155,   156,    -1,    -1,   159,     9,
      10,    11,    -1,    -1,    -1,   166,   167,    -1,   169,    -1,
     171,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
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
      47,    -1,    49,     9,    10,    11,   176,    -1,    -1,    -1,
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
      43,    44,    45,    46,    47,    -1,    49,    -1,   174,    -1,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
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
      47,   174,    49,    -1,    42,    43,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,     9,    10,
      11,    69,    70,    71,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    79,    -1,    -1,    25,   174,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    -1,    49,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   173,   124,   125,   126,   127,
     128,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,
      -1,    -1,    -1,   141,   142,   143,   144,    -1,   146,    -1,
     148,   149,    -1,   151,   152,   153,    -1,    -1,   156,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   173,    -1,   166,     9,
      10,    11,    -1,   171,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   122,    -1,    -1,    -1,    25,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
       9,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     9,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,   122,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    -1,    49,     9,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    25,   122,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    -1,    49,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    -1,    49,
      -1,    -1,     9,    10,    11,    -1,    -1,    -1,   122,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    90,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,     9,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    11,    49,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    -1,
      49,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    -1,    49,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    -1,    49
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
     226,   286,   290,   291,   292,   293,   294,   302,   303,   304,
     306,   307,   310,   320,   321,   322,   327,   330,   348,   353,
     355,   356,   357,   358,   359,   360,   361,   362,   364,   377,
     379,   381,   111,   123,   141,   189,   211,   293,   355,   293,
     172,   293,   293,   293,   346,   347,   293,   293,   293,   293,
     293,   293,   293,   293,   293,   293,   293,   293,   111,   172,
     193,   321,   322,   355,   355,    31,   293,   368,   369,   293,
     111,   172,   193,   321,   322,   323,   354,   360,   365,   366,
     172,   287,   324,   172,   287,   288,   293,   202,   287,   172,
     172,   172,   287,   174,   293,   189,   174,   293,    25,    51,
     124,   146,   166,   172,   189,   196,   382,   392,   393,   174,
     293,   175,   293,   144,   190,   191,   192,    73,   177,   252,
     253,   117,   117,    73,   211,   254,   172,   172,   172,   172,
     189,   224,   383,   172,   172,    73,    78,   137,   138,   139,
     374,   375,   144,   175,   192,   192,    95,   293,   225,   383,
     146,   172,   383,   383,   286,   293,   294,   355,   198,   175,
      78,   325,   374,    78,   374,   374,    26,   144,   162,   384,
     172,     8,   174,    31,   210,   146,   223,   383,   174,   174,
     174,     9,    10,    11,    25,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    49,   174,    61,    61,
     175,   140,   118,   154,   211,   226,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    59,    60,
     121,   350,   351,    61,   175,   352,   172,    61,   175,   177,
     361,   172,   210,    13,   293,    40,   189,   345,   172,   286,
     355,   140,   355,   122,   179,     8,   332,   286,   355,   384,
     140,   172,   326,   121,   350,   351,   352,   173,   293,    26,
     200,     8,   174,   200,   201,   288,   289,   293,   189,   238,
     204,   174,   174,   174,   393,   393,   162,   172,    98,   385,
     393,    13,   189,   174,   198,   174,   192,     8,   174,    90,
     175,   355,     8,   174,    13,   210,     8,   174,   355,   378,
     378,   355,   173,   162,   218,   123,   355,   367,   368,    61,
     121,   137,   375,    72,   293,   355,    78,   137,   375,   192,
     188,   174,   175,   174,   122,   221,   311,   313,    79,   297,
     298,   300,    13,    90,   380,   173,   173,   173,   176,   199,
     200,   212,   215,   220,   293,   178,   180,   181,   189,   385,
      31,   250,   251,   293,   382,   172,   383,   216,   293,   293,
     293,    26,   293,   293,   293,   293,   293,   293,   293,   293,
     293,   293,   293,   293,   293,   293,   293,   293,   293,   293,
     293,   293,   293,   293,   323,   293,   363,   363,   293,   370,
     371,   189,   360,   361,   224,   225,   210,   223,    31,   145,
     290,   293,   293,   293,   293,   293,   293,   293,   293,   293,
     293,   293,   293,   175,   189,   360,   363,   293,   250,   363,
     293,   367,   173,   172,   344,     8,   332,   286,   173,   189,
      31,   293,    31,   293,   173,   173,   360,   250,   175,   189,
     360,   173,   198,   242,   293,    82,    26,   200,   236,   174,
      90,    13,     8,   173,    26,   175,   239,   393,    79,   389,
     390,   391,   172,     8,    42,    43,    61,   124,   136,   146,
     166,   193,   194,   196,   305,   321,   327,   328,   329,   176,
      90,   191,   189,   293,   253,   328,   172,    73,     8,   173,
     173,   173,   174,   189,   388,   119,   229,   172,     8,   173,
     173,    73,    74,   189,   376,   189,    61,   176,   176,   185,
     187,   293,   120,   228,   161,    46,   146,   161,   315,   122,
       8,   332,   173,   393,   393,    13,   121,   350,   351,   352,
     176,     8,   163,   355,   173,     8,   333,    13,   295,   213,
     119,   227,   293,    26,   179,   179,   122,   176,     8,   332,
     384,   172,   219,   222,   383,   217,    63,   355,   293,   384,
     172,   179,   176,   173,   179,   176,   173,    42,    43,    61,
      69,    70,    79,   124,   136,   166,   189,   335,   337,   340,
     343,   189,   355,   355,   122,   350,   351,   352,   173,   293,
     243,    66,    67,   244,   287,   198,   289,    31,   233,   355,
     328,   189,    26,   200,   237,   174,   240,   174,   240,     8,
     163,   122,     8,   332,   173,   157,   385,   386,   393,   328,
     328,   328,   331,   334,   172,    78,   140,   172,   140,   175,
     102,   207,   208,   189,   176,   296,    13,   355,   174,    90,
       8,   163,   230,   321,   175,   367,   123,   355,    13,   179,
     293,   176,   185,   230,   175,   314,    13,   293,   297,   174,
     393,   175,   189,   360,   393,    31,   293,   328,   157,   248,
     249,   348,   349,   172,   321,   228,   293,   293,   293,   172,
     250,   229,   228,   214,   227,   323,   176,   172,   250,    13,
      69,    70,   189,   336,   336,   337,   338,   339,   172,    78,
     137,   172,     8,   332,   173,   344,    31,   293,   176,    66,
      67,   245,   287,   200,   174,    83,   174,   355,   122,   232,
      13,   198,   240,    92,    93,    94,   240,   176,   393,   393,
     389,     8,   173,   173,   122,   179,     8,   332,   331,   189,
     297,   299,   301,   189,   328,   372,   373,   172,   159,   248,
     328,   393,   189,     8,   255,   173,   172,   290,   293,   179,
     176,   255,   147,   160,   175,   310,   317,   147,   175,   316,
     122,   174,   293,   384,   172,   355,   173,     8,   333,   393,
     394,   248,   175,   122,   250,   173,   175,   175,   172,   228,
     326,   172,   250,   173,   122,   179,     8,   332,   338,   137,
     297,   341,   342,   337,   355,   287,    26,    68,   200,   174,
     289,   233,   173,   328,    89,    92,   174,   293,    26,   174,
     241,   176,   163,   157,    26,   328,   328,   173,   122,     8,
     332,   173,   122,   176,     8,   332,   321,   175,   173,    90,
     321,    99,   104,   106,   107,   108,   109,   110,   111,   112,
     148,   149,   151,   176,   256,   278,   279,   280,   281,   285,
     348,   367,   176,   176,    46,   293,   293,   293,   176,   172,
     250,    26,   387,   157,   349,    31,    73,   173,   255,   293,
     173,   255,   255,   248,   175,   250,   173,   337,   337,   173,
     122,   173,     8,   332,    26,   198,   174,   173,   205,   174,
     174,   241,   198,   393,   122,   328,   297,   328,   328,    73,
     198,   387,   393,   382,   231,   321,   112,   124,   146,   152,
     265,   266,   267,   321,   150,   271,   272,   115,   172,   189,
     273,   274,   257,   211,   281,   393,     8,   174,   279,   280,
     173,   146,   312,   176,   176,   172,   250,   173,   393,   104,
     308,   394,    73,    13,   387,   176,   176,   176,   173,   255,
     173,   122,   337,   297,   198,   203,    26,   200,   235,   198,
     173,   328,   122,   122,   173,   176,   308,    13,     8,   174,
     175,   175,     8,   174,     3,     4,     5,     6,     7,     9,
      10,    11,    12,    49,    62,    63,    64,    65,    66,    67,
      68,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   123,   124,   125,   126,   127,   128,   129,   141,
     142,   143,   145,   154,   155,   156,   159,   166,   167,   169,
     171,   189,   318,   319,     8,   174,   146,   150,   189,   274,
     275,   276,   174,    73,   284,   210,   258,   382,   211,   250,
     173,   172,   175,    31,    73,    13,   328,   175,   387,   176,
     337,   122,    26,   200,   234,   198,   328,   328,   175,   175,
     328,   321,   261,   268,   327,   266,    13,    26,    43,   269,
     272,     8,    29,   173,    25,    42,    45,    13,     8,   174,
     383,   284,    13,   210,   173,    31,    73,   309,   198,    73,
      13,   328,   198,   175,   337,   198,    87,   198,   198,   176,
     189,   196,   262,   263,   264,     8,   176,   328,   319,   319,
      51,   270,   275,   275,    25,    42,    45,   328,    73,   172,
     174,   328,   383,    73,     8,   333,   176,    13,   328,   176,
     198,    85,   174,   176,   176,   140,    90,   327,   153,    13,
     259,   172,    31,    73,   173,   328,   176,   174,   206,   189,
     279,   280,   328,   157,   246,   247,   349,   260,    73,   102,
     207,   209,   155,   189,   174,   173,     8,   333,   106,   107,
     108,   282,   283,   246,   172,   231,   174,   387,   157,   282,
     394,   173,   321,   174,   174,   175,   277,   349,    31,    73,
     387,    73,   198,   394,    73,    13,   277,   173,   176,    31,
      73,    13,   328,   175,    73,    13,   328,   198,    13,   328,
     176,   328
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
#line 814 "hphp.y"
    { _p->initParseTree(); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 814 "hphp.y"
    { _p->popLabelInfo();
                                                  _p->finiParseTree();;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 820 "hphp.y"
    { _p->addTopStatement((yyvsp[(2) - (2)]));;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 821 "hphp.y"
    { ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 824 "hphp.y"
    { _p->nns((yyvsp[(1) - (1)]).num()); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 825 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 826 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 827 "hphp.y"
    { _p->nns(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 828 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 829 "hphp.y"
    { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 832 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text(), true);
                                         (yyval).reset();;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 834 "hphp.y"
    { _p->onNamespaceStart((yyvsp[(2) - (3)]).text());;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 835 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(5) - (6)]);;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 836 "hphp.y"
    { _p->onNamespaceStart("");;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 837 "hphp.y"
    { _p->onNamespaceEnd(); (yyval) = (yyvsp[(4) - (5)]);;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 838 "hphp.y"
    { _p->nns(); (yyval).reset();;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 839 "hphp.y"
    { _p->nns();
                                         _p->finishStatement((yyval), (yyvsp[(1) - (2)])); (yyval) = 1;;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 844 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 845 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 846 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 847 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 848 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 849 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 854 "hphp.y"
    { ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 855 "hphp.y"
    { ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 858 "hphp.y"
    { _p->onUse((yyvsp[(1) - (1)]).text(),"");;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 859 "hphp.y"
    { _p->onUse((yyvsp[(2) - (2)]).text(),"");;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 860 "hphp.y"
    { _p->onUse((yyvsp[(1) - (3)]).text(),(yyvsp[(3) - (3)]).text());;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 862 "hphp.y"
    { _p->onUse((yyvsp[(2) - (4)]).text(),(yyvsp[(4) - (4)]).text());;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 866 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + (yyvsp[(2) - (3)]) + (yyvsp[(3) - (3)]); (yyval) = (yyvsp[(1) - (3)]).num() | 2;;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = (yyval).num() | 1;;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 873 "hphp.y"
    { (yyval).set((yyvsp[(3) - (3)]).num() | 2, _p->nsDecl((yyvsp[(3) - (3)]).text()));;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 874 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = (yyval).num() | 2;;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 877 "hphp.y"
    { if ((yyvsp[(1) - (1)]).num() & 1) {
                                           (yyvsp[(1) - (1)]).setText(_p->resolve((yyvsp[(1) - (1)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 884 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),0));
                                         }
                                         (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 891 "hphp.y"
    { if ((yyvsp[(1) - (2)]).num() & 1) {
                                           (yyvsp[(1) - (2)]).setText(_p->resolve((yyvsp[(1) - (2)]).text(),1));
                                         }
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 899 "hphp.y"
    { (yyvsp[(3) - (5)]).setText(_p->nsDecl((yyvsp[(3) - (5)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 902 "hphp.y"
    { (yyvsp[(2) - (4)]).setText(_p->nsDecl((yyvsp[(2) - (4)]).text()));
                                         on_constant(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 908 "hphp.y"
    { _p->addStatement((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 909 "hphp.y"
    { _p->onStatementListStart((yyval));;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 912 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 913 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 918 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 922 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 927 "hphp.y"
    { _p->onIf((yyval),(yyvsp[(2) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]));;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 928 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 929 "hphp.y"
    { _p->popLabelScope();
                                         _p->onWhile((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 932 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 934 "hphp.y"
    { _p->popLabelScope();
                                         _p->onDo((yyval),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 937 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 938 "hphp.y"
    { _p->popLabelScope();
                                         _p->onFor((yyval),(yyvsp[(3) - (10)]),(yyvsp[(5) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]));;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 940 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 941 "hphp.y"
    { _p->popLabelScope();
                                         _p->onSwitch((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 943 "hphp.y"
    { _p->onBreakContinue((yyval), true, NULL);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 944 "hphp.y"
    { _p->onBreakContinue((yyval), true, &(yyvsp[(2) - (3)]));;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 945 "hphp.y"
    { _p->onBreakContinue((yyval), false, NULL);;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 946 "hphp.y"
    { _p->onBreakContinue((yyval), false, &(yyvsp[(2) - (3)]));;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 947 "hphp.y"
    { _p->onReturn((yyval), NULL);;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 948 "hphp.y"
    { _p->onReturn((yyval), &(yyvsp[(2) - (3)]));;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 949 "hphp.y"
    { _p->onYieldBreak((yyval));;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 950 "hphp.y"
    { _p->onGlobal((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 951 "hphp.y"
    { _p->onStatic((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 952 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 953 "hphp.y"
    { _p->onUnset((yyval), (yyvsp[(3) - (5)]));;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 954 "hphp.y"
    { (yyval).reset(); (yyval) = ';';}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 955 "hphp.y"
    { _p->onEcho((yyval), (yyvsp[(1) - (1)]), 1);;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 958 "hphp.y"
    { _p->pushLabelScope();;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 959 "hphp.y"
    { _p->popLabelScope();
                                         _p->onForEach((yyval),(yyvsp[(3) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(6) - (9)]),(yyvsp[(9) - (9)]));;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 962 "hphp.y"
    { _p->onBlock((yyval), (yyvsp[(5) - (5)])); (yyval) = T_DECLARE;;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 970 "hphp.y"
    { _p->onTry((yyval),(yyvsp[(3) - (14)]),(yyvsp[(7) - (14)]),(yyvsp[(8) - (14)]),(yyvsp[(11) - (14)]),(yyvsp[(13) - (14)]),(yyvsp[(14) - (14)]));;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 973 "hphp.y"
    { _p->onTry((yyval), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 974 "hphp.y"
    { _p->onThrow((yyval), (yyvsp[(2) - (3)]));;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 975 "hphp.y"
    { _p->onGoto((yyval), (yyvsp[(2) - (3)]), true);
                                         _p->addGoto((yyvsp[(2) - (3)]).text(),
                                                     _p->getLocation(),
                                                     &(yyval)); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 979 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 980 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 981 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 982 "hphp.y"
    { _p->onExpStatement((yyval), (yyvsp[(1) - (2)]));;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 983 "hphp.y"
    { _p->onLabel((yyval), (yyvsp[(1) - (2)]));
                                         _p->addLabel((yyvsp[(1) - (2)]).text(),
                                                      _p->getLocation(),
                                                      &(yyval)); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 995 "hphp.y"
    { _p->onCatch((yyval), (yyvsp[(1) - (9)]), (yyvsp[(4) - (9)]), (yyvsp[(5) - (9)]), (yyvsp[(8) - (9)]));;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 996 "hphp.y"
    { (yyval).reset();;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 1000 "hphp.y"
    { finally_statement(_p);;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 1002 "hphp.y"
    { _p->onFinally((yyval), (yyvsp[(4) - (5)]));;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 1007 "hphp.y"
    { (yyval).reset();;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 1011 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 1012 "hphp.y"
    { (yyval).reset();;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1016 "hphp.y"
    { _p->pushFuncLocation();;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1021 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onFunctionStart((yyvsp[(3) - (3)]));
                                         _p->pushLabelInfo();;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1026 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(8) - (11)]),(yyvsp[(2) - (11)]),(yyvsp[(3) - (11)]),(yyvsp[(6) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1030 "hphp.y"
    { (yyvsp[(4) - (4)]).setText(_p->nsDecl((yyvsp[(4) - (4)]).text()));
                                         _p->onFunctionStart((yyvsp[(4) - (4)]));
                                         _p->pushLabelInfo();;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1035 "hphp.y"
    { _p->onFunction((yyval),0,(yyvsp[(9) - (12)]),(yyvsp[(3) - (12)]),(yyvsp[(4) - (12)]),(yyvsp[(7) - (12)]),(yyvsp[(11) - (12)]),&(yyvsp[(1) - (12)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1042 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart((yyvsp[(1) - (2)]).num(),(yyvsp[(2) - (2)]));;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1045 "hphp.y"
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
#line 1060 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart((yyvsp[(2) - (3)]).num(),(yyvsp[(3) - (3)]));;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1063 "hphp.y"
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
#line 1077 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(2) - (2)]));;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1080 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(6) - (7)]),0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1085 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_INTERFACE,(yyvsp[(3) - (3)]));;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1088 "hphp.y"
    { _p->onInterface((yyval),(yyvsp[(3) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),&(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1095 "hphp.y"
    { (yyvsp[(2) - (2)]).setText(_p->nsDecl((yyvsp[(2) - (2)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(2) - (2)]));;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1098 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(2) - (7)]),t_ext,t_imp,
                                                     (yyvsp[(6) - (7)]), 0);
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1106 "hphp.y"
    { (yyvsp[(3) - (3)]).setText(_p->nsDecl((yyvsp[(3) - (3)]).text()));
                                         _p->onClassStart(T_TRAIT, (yyvsp[(3) - (3)]));;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1109 "hphp.y"
    { Token t_ext, t_imp;
                                         t_ext.reset(); t_imp.reset();
                                         _p->onClass((yyval),T_TRAIT,(yyvsp[(3) - (8)]),t_ext,t_imp,
                                                     (yyvsp[(7) - (8)]), &(yyvsp[(1) - (8)]));
                                         _p->popClass();
                                         _p->popTypeScope();;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1117 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1118 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1122 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1125 "hphp.y"
    { _p->pushClass(false); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1128 "hphp.y"
    { (yyval) = T_CLASS;;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1129 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1130 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1134 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1135 "hphp.y"
    { (yyval).reset();;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1138 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1139 "hphp.y"
    { (yyval).reset();;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1142 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1143 "hphp.y"
    { (yyval).reset();;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1146 "hphp.y"
    { _p->onInterfaceName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1148 "hphp.y"
    { _p->onInterfaceName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1151 "hphp.y"
    { _p->onTraitName((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1153 "hphp.y"
    { _p->onTraitName((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1157 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1158 "hphp.y"
    { (yyval).reset();;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1161 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1162 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1166 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1168 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1171 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1173 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1176 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1178 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1181 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1183 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1193 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1194 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1195 "hphp.y"
    { (yyval) = (yyvsp[(2) - (4)]);;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1196 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1201 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1203 "hphp.y"
    { _p->onCase((yyval),(yyvsp[(1) - (4)]),NULL,(yyvsp[(4) - (4)]));;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1204 "hphp.y"
    { (yyval).reset();;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1207 "hphp.y"
    { (yyval).reset();;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1208 "hphp.y"
    { (yyval).reset();;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1213 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1214 "hphp.y"
    { (yyval).reset();;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1219 "hphp.y"
    { _p->onElseIf((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1220 "hphp.y"
    { (yyval).reset();;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1223 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1224 "hphp.y"
    { (yyval).reset();;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1227 "hphp.y"
    { (yyval) = (yyvsp[(3) - (3)]);;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1228 "hphp.y"
    { (yyval).reset();;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1233 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1235 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1236 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset(); ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1237 "hphp.y"
    { (yyval).reset();;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1243 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]),0,
                                                     NULL,&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]));;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1247 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),1,
                                                     NULL,&(yyvsp[(1) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1252 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (7)]),(yyvsp[(5) - (7)]),1,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(1) - (7)]),&(yyvsp[(2) - (7)]));;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1257 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(3) - (6)]),(yyvsp[(4) - (6)]),0,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),&(yyvsp[(2) - (6)]));;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1262 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]),0,
                                                     NULL,&(yyvsp[(4) - (6)]),&(yyvsp[(3) - (6)]));;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1267 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]),1,
                                                     NULL,&(yyvsp[(4) - (7)]),&(yyvsp[(3) - (7)]));;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1273 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(7) - (9)]),1,
                                                     &(yyvsp[(9) - (9)]),&(yyvsp[(4) - (9)]),&(yyvsp[(3) - (9)]));;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1279 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(6) - (8)]),0,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(4) - (8)]),&(yyvsp[(3) - (8)]));;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1285 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1287 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1288 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset(); ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1289 "hphp.y"
    { (yyval).reset();;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1294 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]),0,
                                                     NULL,&(yyvsp[(1) - (3)]),NULL);;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1297 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),1,
                                                     NULL,&(yyvsp[(1) - (4)]),NULL);;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1301 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (6)]),(yyvsp[(4) - (6)]),1,
                                                     &(yyvsp[(6) - (6)]),&(yyvsp[(1) - (6)]),NULL);;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1305 "hphp.y"
    { _p->onParam((yyval),NULL,(yyvsp[(2) - (5)]),(yyvsp[(3) - (5)]),0,
                                                     &(yyvsp[(5) - (5)]),&(yyvsp[(1) - (5)]),NULL);;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1309 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),(yyvsp[(5) - (5)]),0,
                                                     NULL,&(yyvsp[(3) - (5)]),NULL);;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1313 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(4) - (6)]),(yyvsp[(6) - (6)]),1,
                                                     NULL,&(yyvsp[(3) - (6)]),NULL);;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1318 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(6) - (8)]),1,
                                                     &(yyvsp[(8) - (8)]),&(yyvsp[(3) - (8)]),NULL);;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1323 "hphp.y"
    { _p->onParam((yyval),&(yyvsp[(1) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]),0,
                                                     &(yyvsp[(7) - (7)]),&(yyvsp[(3) - (7)]),NULL);;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1329 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1330 "hphp.y"
    { (yyval).reset();;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1333 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(1) - (1)]),0);;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1334 "hphp.y"
    { _p->onCallParam((yyval),NULL,(yyvsp[(2) - (2)]),1);;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1336 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1338 "hphp.y"
    { _p->onCallParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1342 "hphp.y"
    { _p->onGlobalVar((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1343 "hphp.y"
    { _p->onGlobalVar((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1346 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1347 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]); (yyval) = 1;;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1348 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 1;;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1352 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1354 "hphp.y"
    { _p->onStaticVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1355 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1356 "hphp.y"
    { _p->onStaticVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1361 "hphp.y"
    { _p->onClassStatement((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1362 "hphp.y"
    { (yyval).reset();;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1365 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (1)]));;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1366 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1369 "hphp.y"
    { _p->onClassVariableModifer((yyvsp[(1) - (2)]));;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1370 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]),&(yyvsp[(2) - (5)]));;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1372 "hphp.y"
    { _p->onClassVariableStart
                                         ((yyval),NULL,(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1376 "hphp.y"
    { _p->onMethodStart((yyvsp[(4) - (5)]), (yyvsp[(1) - (5)]));
                                         _p->pushLabelInfo();;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1381 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(1) - (10)]),(yyvsp[(9) - (10)]),(yyvsp[(3) - (10)]),(yyvsp[(4) - (10)]),(yyvsp[(7) - (10)]),(yyvsp[(10) - (10)]),0);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1387 "hphp.y"
    { _p->onMethodStart((yyvsp[(5) - (6)]), (yyvsp[(2) - (6)]));
                                         _p->pushLabelInfo();;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1392 "hphp.y"
    { _p->onMethod((yyval),(yyvsp[(2) - (11)]),(yyvsp[(10) - (11)]),(yyvsp[(4) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(11) - (11)]),&(yyvsp[(1) - (11)]));
                                         _p->popLabelInfo();
                                         _p->popTypeScope();;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1396 "hphp.y"
    { _p->xhpSetAttributes((yyvsp[(2) - (3)]));;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1398 "hphp.y"
    { xhp_category_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1400 "hphp.y"
    { xhp_children_stmt(_p,(yyval),(yyvsp[(2) - (3)]));;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1401 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitUse((yyval),(yyvsp[(2) - (3)]),t); ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1404 "hphp.y"
    { _p->onTraitUse((yyval),(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)])); ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1407 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1408 "hphp.y"
    { _p->onTraitRule((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1409 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1415 "hphp.y"
    { _p->onTraitPrecRule((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1419 "hphp.y"
    { _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),
                                                                    (yyvsp[(4) - (5)]));;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1422 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleModify((yyval),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),
                                                                    t);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1429 "hphp.y"
    { _p->onTraitAliasRuleStart((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1430 "hphp.y"
    { Token t; t.reset();
                                         _p->onTraitAliasRuleStart((yyval),t,(yyvsp[(1) - (1)]));;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1435 "hphp.y"
    { xhp_attribute_list(_p,(yyval),
                                         _p->xhpGetAttributes(),(yyvsp[(1) - (1)]));;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1438 "hphp.y"
    { xhp_attribute_list(_p,(yyval), &(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1445 "hphp.y"
    { xhp_attribute(_p,(yyval),(yyvsp[(1) - (4)]),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));
                                         (yyval) = 1;;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1447 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 0;;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1451 "hphp.y"
    { (yyval) = 4;;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1452 "hphp.y"
    { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         (yyval) = 5; (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1458 "hphp.y"
    { (yyval) = 6;;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1460 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]); (yyval) = 7;;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1464 "hphp.y"
    { _p->onArrayPair((yyval),  0,0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1466 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1470 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1471 "hphp.y"
    { scalar_null(_p, (yyval));;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1475 "hphp.y"
    { scalar_num(_p, (yyval), "1");;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1476 "hphp.y"
    { scalar_num(_p, (yyval), "0");;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1480 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),0,&(yyvsp[(1) - (1)]),t,0);;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1483 "hphp.y"
    { Token t; scalar_num(_p, t, "1");
                                         _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]),t,0);;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1488 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1493 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = 2;;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1494 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1;;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1496 "hphp.y"
    { (yyval) = 0;;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1500 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (3)]), 0);;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1501 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 1);;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1502 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 2);;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1503 "hphp.y"
    { xhp_children_paren(_p, (yyval), (yyvsp[(2) - (4)]), 3);;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1507 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1508 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (1)]),0,  0);;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1509 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),1,  0);;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1510 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),2,  0);;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1511 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (2)]),3,  0);;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1513 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),4,&(yyvsp[(3) - (3)]));;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1515 "hphp.y"
    { xhp_children_decl(_p,(yyval),(yyvsp[(1) - (3)]),5,&(yyvsp[(3) - (3)]));;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1519 "hphp.y"
    { (yyval) = -1;
                                         if ((yyvsp[(1) - (1)]).same("any")) (yyval) = 1; else
                                         if ((yyvsp[(1) - (1)]).same("pcdata")) (yyval) = 2;;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1522 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();  (yyval) = (yyvsp[(1) - (1)]); (yyval) = 3;;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1523 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(0); (yyval) = (yyvsp[(1) - (1)]); (yyval) = 4;;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1527 "hphp.y"
    { (yyval).reset();;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1528 "hphp.y"
    { _p->finishStatement((yyval), (yyvsp[(2) - (3)])); (yyval) = 1;;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1531 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1532 "hphp.y"
    { (yyval).reset();;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1535 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1536 "hphp.y"
    { (yyval).reset();;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1539 "hphp.y"
    { _p->onMemberModifier((yyval),NULL,(yyvsp[(1) - (1)]));;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1541 "hphp.y"
    { _p->onMemberModifier((yyval),&(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1544 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1545 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1546 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1547 "hphp.y"
    { (yyval) = T_STATIC;;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1548 "hphp.y"
    { (yyval) = T_ABSTRACT;;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1549 "hphp.y"
    { (yyval) = T_FINAL;;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1552 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1553 "hphp.y"
    { (yyval).reset();;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1556 "hphp.y"
    { (yyval) = T_PUBLIC;;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1557 "hphp.y"
    { (yyval) = T_PROTECTED;;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1558 "hphp.y"
    { (yyval) = T_PRIVATE;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1562 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1564 "hphp.y"
    { _p->onClassVariable((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),&(yyvsp[(5) - (5)]));;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1565 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1566 "hphp.y"
    { _p->onClassVariable((yyval),0,(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1570 "hphp.y"
    { _p->onClassConstant((yyval),&(yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1571 "hphp.y"
    { _p->onClassConstant((yyval),0,(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1575 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1577 "hphp.y"
    { _p->onNewObject((yyval), (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1578 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_CLONE,1);;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1579 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1582 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1586 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1587 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1591 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1592 "hphp.y"
    { (yyval).reset();;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1596 "hphp.y"
    { _p->onYield((yyval), (yyvsp[(2) - (2)]));;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1597 "hphp.y"
    { _p->onYieldPair((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1601 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0, true);;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1606 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]), true);;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1610 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1611 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1612 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1616 "hphp.y"
    { _p->onListAssignment((yyval), (yyvsp[(3) - (6)]), &(yyvsp[(6) - (6)]));;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1617 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1618 "hphp.y"
    { _p->onAssign((yyval), (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)]), 1);;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1621 "hphp.y"
    { _p->onAssignNew((yyval),(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]),(yyvsp[(6) - (6)]));;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1622 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_PLUS_EQUAL);;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1623 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MINUS_EQUAL);;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1624 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MUL_EQUAL);;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1625 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_DIV_EQUAL);;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1626 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_CONCAT_EQUAL);;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1627 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_MOD_EQUAL);;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1628 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_AND_EQUAL);;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1629 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_OR_EQUAL);;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1630 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_XOR_EQUAL);;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1631 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL_EQUAL);;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1632 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR_EQUAL);;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1633 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_INC,0);;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1634 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INC,1);;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1635 "hphp.y"
    { UEXP((yyval),(yyvsp[(1) - (2)]),T_DEC,0);;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1636 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DEC,1);;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1637 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_OR);;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1638 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_BOOLEAN_AND);;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1639 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_OR);;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1640 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_AND);;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1641 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_LOGICAL_XOR);;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1642 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'|');;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1643 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'&');;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1644 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'^');;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1645 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'.');;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1646 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'+');;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1647 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'-');;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1648 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'*');;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1649 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'/');;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1650 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'%');;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1651 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SL);;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1652 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_SR);;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1653 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1654 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1655 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'!',1);;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1656 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'~',1);;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1657 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_IDENTICAL);;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1658 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_IDENTICAL);;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1659 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_EQUAL);;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1660 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_IS_NOT_EQUAL);;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1661 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'<');;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1662 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_SMALLER_OR_EQUAL);;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1664 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),'>');;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1665 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),
                                              T_IS_GREATER_OR_EQUAL);;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1668 "hphp.y"
    { BEXP((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),T_INSTANCEOF);;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1669 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1670 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (5)]), &(yyvsp[(3) - (5)]), (yyvsp[(5) - (5)]));;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1671 "hphp.y"
    { _p->onQOp((yyval), (yyvsp[(1) - (4)]),   0, (yyvsp[(4) - (4)]));;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1672 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1673 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INT_CAST,1);;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1674 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_DOUBLE_CAST,1);;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1675 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_STRING_CAST,1);;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1676 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_ARRAY_CAST,1);;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1677 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_OBJECT_CAST,1);;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1678 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_BOOL_CAST,1);;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1679 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_UNSET_CAST,1);;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1680 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_EXIT,1);;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1681 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'@',1);;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1682 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1683 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1684 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1685 "hphp.y"
    { _p->onEncapsList((yyval),'`',(yyvsp[(2) - (3)]));;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1686 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_PRINT,1);;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1688 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1692 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(2) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)]),(yyvsp[(10) - (11)]),0);
                                         _p->popLabelInfo();;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1696 "hphp.y"
    { Token t; _p->onClosureStart(t);
                                         _p->pushLabelInfo();;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1700 "hphp.y"
    { Token u; u.reset();
                                         _p->onClosure((yyval),u,(yyvsp[(3) - (12)]),(yyvsp[(6) - (12)]),(yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),1);
                                         _p->popLabelInfo();;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1703 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1704 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1708 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (1)]), _p);
                                        _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)])); ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1715 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1718 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1725 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1728 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1733 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1734 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1739 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1740 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1744 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onArray((yyval), (yyvsp[(3) - (4)]), T_ARRAY); ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1749 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1750 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1755 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1762 "hphp.y"
    { Token t;
                                         _p->onName(t,(yyvsp[(1) - (4)]),Parser::StringName);
                                         BEXP((yyval),t,(yyvsp[(3) - (4)]),T_COLLECTION);;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1769 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1771 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1775 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1776 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1777 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1784 "hphp.y"
    { (yyval) = (yyvsp[(3) - (5)]);;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1785 "hphp.y"
    { (yyval).reset();;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1789 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1790 "hphp.y"
    { _p->onClosureParam((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1791 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1792 "hphp.y"
    { _p->onClosureParam((yyval),  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1799 "hphp.y"
    { xhp_tag(_p,(yyval),(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1802 "hphp.y"
    { Token t1; _p->onArray(t1,(yyvsp[(1) - (2)]));
                                         Token t2; _p->onArray(t2,(yyvsp[(2) - (2)]));
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam((yyvsp[(1) - (2)]),NULL,t1,0);
                                         _p->onCallParam((yyval), &(yyvsp[(1) - (2)]),t2,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),file,0);
                                         _p->onCallParam((yyvsp[(1) - (2)]), &(yyvsp[(1) - (2)]),line,0);
                                         (yyval).setText("");;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1813 "hphp.y"
    { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray((yyvsp[(4) - (6)]),(yyvsp[(1) - (6)]));
                                         _p->onArray((yyvsp[(5) - (6)]),(yyvsp[(3) - (6)]));
                                         _p->onCallParam((yyvsp[(2) - (6)]),NULL,(yyvsp[(4) - (6)]),0);
                                         _p->onCallParam((yyval), &(yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),file,0);
                                         _p->onCallParam((yyvsp[(2) - (6)]), &(yyvsp[(2) - (6)]),line,0);
                                         (yyval).setText((yyvsp[(6) - (6)]).text());;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1824 "hphp.y"
    { (yyval).reset(); (yyval).setText("");;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1825 "hphp.y"
    { (yyval).reset(); (yyval).setText((yyvsp[(1) - (1)]));;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1830 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),&(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]),0);;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1831 "hphp.y"
    { (yyval).reset();;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1834 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (2)]),0,(yyvsp[(2) - (2)]),0);;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1835 "hphp.y"
    { (yyval).reset();;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1838 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1842 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpDecode();
                                         _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1845 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1848 "hphp.y"
    { (yyval).reset();
                                         if ((yyvsp[(1) - (1)]).htmlTrim()) {
                                           (yyvsp[(1) - (1)]).xhpDecode();
                                           _p->onScalar((yyval),
                                           T_CONSTANT_ENCAPSED_STRING, (yyvsp[(1) - (1)]));
                                         }
                                       ;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1855 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1856 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1860 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1862 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + ":" + (yyvsp[(3) - (3)]);;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1864 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]) + "-" + (yyvsp[(3) - (3)]);;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1867 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1868 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1869 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1870 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1871 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1872 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1873 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1874 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1875 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1876 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1877 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1878 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1879 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1880 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1881 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1882 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1883 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1884 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1885 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1886 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1887 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1888 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1889 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1890 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1891 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1892 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1893 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1894 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1895 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1896 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 1897 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 1898 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1899 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1900 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1901 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 1902 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1903 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 1904 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1905 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1906 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1907 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1908 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1909 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1910 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1911 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1912 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1913 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1914 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1915 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1916 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1917 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1918 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1919 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 1920 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1921 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 1922 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 1923 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1924 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1925 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1926 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 1927 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1928 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1929 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 1930 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 1931 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1932 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1933 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1934 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1935 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1936 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1937 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1938 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1939 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 1940 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 1941 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1942 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1943 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1944 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 1949 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1953 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1954 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel(); (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1957 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1958 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1959 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),
                                         Parser::StaticClassExprName);;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 1963 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StringName);;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 1964 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::StaticName);;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1965 "hphp.y"
    { _p->onName((yyval),(yyvsp[(1) - (1)]),Parser::ExprName);;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 1969 "hphp.y"
    { (yyval).reset();;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 1970 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 1971 "hphp.y"
    { (yyval).reset();;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 1975 "hphp.y"
    { (yyval).reset();;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 1976 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), 0);;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 1977 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 1981 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 1982 "hphp.y"
    { (yyval).reset();;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 1986 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 1987 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 1988 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 1990 "hphp.y"
    { _p->onScalar((yyval), T_LINE,     (yyvsp[(1) - (1)]));;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 1991 "hphp.y"
    { _p->onScalar((yyval), T_FILE,     (yyvsp[(1) - (1)]));;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 1992 "hphp.y"
    { _p->onScalar((yyval), T_DIR,      (yyvsp[(1) - (1)]));;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 1993 "hphp.y"
    { _p->onScalar((yyval), T_CLASS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 1994 "hphp.y"
    { _p->onScalar((yyval), T_TRAIT_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 1995 "hphp.y"
    { _p->onScalar((yyval), T_METHOD_C, (yyvsp[(1) - (1)]));;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 1996 "hphp.y"
    { _p->onScalar((yyval), T_FUNC_C,   (yyvsp[(1) - (1)]));;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 1997 "hphp.y"
    { _p->onScalar((yyval), T_NS_C,  (yyvsp[(1) - (1)]));;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 1998 "hphp.y"
    { _p->onScalar((yyval), T_COMPILER_HALT_OFFSET, (yyvsp[(1) - (1)]));;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2001 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2003 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2007 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2008 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2009 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2010 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2012 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2013 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY); ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2015 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2017 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2018 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2024 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2026 "hphp.y"
    { (yyvsp[(1) - (3)]).xhpLabel();
                                         _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 1);;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2031 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2032 "hphp.y"
    { _p->onConstantValue((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2033 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2034 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2035 "hphp.y"
    { _p->onEncapsList((yyval),'"',(yyvsp[(2) - (3)]));;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2036 "hphp.y"
    { _p->onEncapsList((yyval),'\'',(yyvsp[(2) - (3)]));;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2038 "hphp.y"
    { _p->onEncapsList((yyval),T_START_HEREDOC,
                                                          (yyvsp[(2) - (3)]));;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2043 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2044 "hphp.y"
    { (yyval).reset();;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2048 "hphp.y"
    { (yyval).reset();;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2049 "hphp.y"
    { (yyval).reset();;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2052 "hphp.y"
    { only_in_hh_syntax(_p); (yyval).reset();;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2053 "hphp.y"
    { (yyval).reset();;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2059 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2061 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2063 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2064 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2068 "hphp.y"
    { _p->onScalar((yyval), T_LNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2069 "hphp.y"
    { _p->onScalar((yyval), T_DNUMBER,  (yyvsp[(1) - (1)]));;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2070 "hphp.y"
    { _p->onScalar((yyval),
                                         T_CONSTANT_ENCAPSED_STRING,  (yyvsp[(1) - (1)]));;}
    break;

  case 549:

/* Line 1455 of yacc.c  */
#line 2074 "hphp.y"
    { _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyvsp[(2) - (3)]));;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2076 "hphp.y"
    { (yyval).setText(""); _p->onScalar((yyval), T_CONSTANT_ENCAPSED_STRING, (yyval));;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2079 "hphp.y"
    { _p->onScalar((yyval),T_LNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2080 "hphp.y"
    { _p->onScalar((yyval),T_DNUMBER,(yyvsp[(1) - (1)]));;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2081 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2084 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2085 "hphp.y"
    { constant_ae(_p,(yyval),(yyvsp[(1) - (1)]));;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2086 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'+',1);;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2087 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),'-',1);;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2089 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY);;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2090 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2092 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onArray((yyval),(yyvsp[(3) - (4)]),T_ARRAY); ;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2098 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2099 "hphp.y"
    { (yyval).reset();;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2104 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2106 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2108 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2109 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2113 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2114 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2119 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2120 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2125 "hphp.y"
    {  _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0); ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2128 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0); ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2133 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2134 "hphp.y"
    { (yyval).reset();;}
    break;

  case 575:

/* Line 1455 of yacc.c  */
#line 2137 "hphp.y"
    { _p->onArray((yyval),(yyvsp[(2) - (3)]),T_ARRAY);;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2138 "hphp.y"
    { Token t; t.reset();
                                         _p->onArray((yyval),t,T_ARRAY);;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2145 "hphp.y"
    { _p->onUserAttribute((yyval),&(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2147 "hphp.y"
    { _p->onUserAttribute((yyval),  0,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2150 "hphp.y"
    { user_attribute_check(_p);;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2152 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2155 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2158 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2159 "hphp.y"
    { (yyval).reset();;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2163 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2165 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2169 "hphp.y"
    { (yyval) = (yyvsp[(2) - (2)]);;}
    break;

  case 587:

/* Line 1455 of yacc.c  */
#line 2170 "hphp.y"
    { (yyval) = (yyvsp[(3) - (4)]);;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2174 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2175 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2179 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2181 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2186 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2188 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]));;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2192 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2193 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2194 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2195 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2196 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2197 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2199 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2202 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2204 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2205 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2209 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2210 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2211 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2212 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 2214 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 2216 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2218 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),NULL);;}
    break;

  case 611:

/* Line 1455 of yacc.c  */
#line 2219 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2223 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 2224 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 2225 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 2231 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]));;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2234 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2237 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(1) - (8)]),(yyvsp[(4) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2241 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (9)]),(yyvsp[(5) - (9)]),(yyvsp[(8) - (9)]));;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2245 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (8)]),(yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]));;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 2249 "hphp.y"
    { _p->onObjectMethodCall((yyval),(yyvsp[(2) - (10)]),(yyvsp[(6) - (10)]),(yyvsp[(9) - (10)]));;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2256 "hphp.y"
    { _p->onCall((yyval),0,(yyvsp[(3) - (7)]),(yyvsp[(6) - (7)]),&(yyvsp[(1) - (7)]));;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 2260 "hphp.y"
    { _p->onCall((yyval),1,(yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),&(yyvsp[(1) - (6)]));;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2264 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2266 "hphp.y"
    { _p->onIndirectRef((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2271 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2272 "hphp.y"
    { _p->onRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2273 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2276 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2277 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(3) - (4)]), 0);;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2280 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2281 "hphp.y"
    { (yyval).reset();;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2285 "hphp.y"
    { (yyval) = 1;;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2286 "hphp.y"
    { (yyval)++;;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2290 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2291 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2292 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2294 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 2297 "hphp.y"
    { _p->onStaticMember((yyval),(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2298 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2302 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]);;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2304 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2306 "hphp.y"
    { _p->onObjectProperty((yyval),(yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]));;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2307 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 2311 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (2)]),NULL);;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2312 "hphp.y"
    { _p->onAListVar((yyval),&(yyvsp[(1) - (3)]),&(yyvsp[(3) - (3)]));;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2314 "hphp.y"
    { _p->onAListSub((yyval),&(yyvsp[(1) - (6)]),(yyvsp[(5) - (6)]));;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2315 "hphp.y"
    { _p->onAListVar((yyval),NULL,NULL);;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2316 "hphp.y"
    { _p->onAListVar((yyval),NULL,&(yyvsp[(1) - (1)]));;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2317 "hphp.y"
    { _p->onAListSub((yyval),NULL,(yyvsp[(3) - (4)]));;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2322 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2323 "hphp.y"
    { (yyval).reset();;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 2327 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]),0);;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 2328 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]),0);;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2329 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),0);;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2330 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(1) - (1)]),0);;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2333 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (6)]),&(yyvsp[(3) - (6)]),(yyvsp[(6) - (6)]),1);;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2335 "hphp.y"
    { _p->onArrayPair((yyval),&(yyvsp[(1) - (4)]),  0,(yyvsp[(4) - (4)]),1);;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2336 "hphp.y"
    { _p->onArrayPair((yyval),  0,&(yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]),1);;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2337 "hphp.y"
    { _p->onArrayPair((yyval),  0,  0,(yyvsp[(2) - (2)]),1);;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2342 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2343 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2347 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2348 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2349 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2350 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2355 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]);;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2356 "hphp.y"
    { _p->onEmptyCollection((yyval));;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2361 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (5)]),&(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)]));;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 2363 "hphp.y"
    { _p->onCollectionPair((yyval),&(yyvsp[(1) - (3)]),  0,(yyvsp[(3) - (3)]));;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2365 "hphp.y"
    { _p->onCollectionPair((yyval),  0,&(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]));;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2366 "hphp.y"
    { _p->onCollectionPair((yyval),  0,  0,(yyvsp[(1) - (1)]));;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2370 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), -1);;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 2372 "hphp.y"
    { _p->addEncap((yyval), &(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), 0);;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2373 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (1)]), -1);;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 2375 "hphp.y"
    { _p->addEncap((yyval), NULL, (yyvsp[(1) - (2)]), 0);
                                         _p->addEncap((yyval), &(yyval), (yyvsp[(2) - (2)]), -1); ;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 2380 "hphp.y"
    { _p->onSimpleVariable((yyval), (yyvsp[(1) - (1)]));;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2382 "hphp.y"
    { _p->encapRefDim((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]));;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2384 "hphp.y"
    { _p->encapObjProp((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2386 "hphp.y"
    { _p->onDynamicVariable((yyval), (yyvsp[(2) - (3)]), 1);;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2388 "hphp.y"
    { _p->encapArray((yyval), (yyvsp[(2) - (6)]), (yyvsp[(4) - (6)]));;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 2389 "hphp.y"
    { (yyval) = (yyvsp[(2) - (3)]);;}
    break;

  case 683:

/* Line 1455 of yacc.c  */
#line 2392 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_STRING;;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2393 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_NUM_STRING;;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2394 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); (yyval) = T_VARIABLE;;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2398 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_ISSET,1);;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2399 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EMPTY,1);;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2400 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE,1);;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 2401 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_INCLUDE_ONCE,1);;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2402 "hphp.y"
    { UEXP((yyval),(yyvsp[(3) - (4)]),T_EVAL,1);;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2403 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE,1);;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2404 "hphp.y"
    { UEXP((yyval),(yyvsp[(2) - (2)]),T_REQUIRE_ONCE,1);;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2408 "hphp.y"
    { _p->onExprListElem((yyval), NULL, (yyvsp[(1) - (1)]));;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2409 "hphp.y"
    { _p->onExprListElem((yyval), &(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 2414 "hphp.y"
    { _p->onClassConst((yyval), (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), 0);;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2428 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypedef((yyval), (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                         _p->popTypeScope(); ;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2433 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypedef((yyval), (yyvsp[(2) - (6)]), (yyvsp[(5) - (6)]));
                                         _p->popTypeScope(); ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2439 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2440 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2446 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2450 "hphp.y"
    { _p->pushTypeScope(); (yyval) = (yyvsp[(1) - (4)]);
                                         only_in_hh_syntax(_p); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2457 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 2458 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2462 "hphp.y"
    { Token t; t.reset();
                                         _p->onTypeList((yyvsp[(1) - (1)]), t);
                                         (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2465 "hphp.y"
    { _p->onTypeList((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
                                         (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2470 "hphp.y"
    { (yyval) = (yyvsp[(1) - (3)]); ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2471 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 2472 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 2473 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2477 "hphp.y"
    { (yyval).reset(); ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2478 "hphp.y"
    { only_in_hh_syntax(_p); (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2483 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (3)]).text()); ;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 2484 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (1)]).text()); ;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 2486 "hphp.y"
    { _p->addTypeVar((yyvsp[(3) - (5)]).text()); ;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2487 "hphp.y"
    { _p->addTypeVar((yyvsp[(1) - (3)]).text()); ;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2493 "hphp.y"
    { validate_shape_keyname((yyvsp[(1) - (3)]), _p); ;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2504 "hphp.y"
    { (yyval) = (yyvsp[(1) - (2)]); ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2506 "hphp.y"
    {;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2510 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyval).setText("array"); ;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 2518 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '?');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2521 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization((yyvsp[(2) - (2)]), '@');
                                         (yyval) = (yyvsp[(2) - (2)]); ;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2524 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 727:

/* Line 1455 of yacc.c  */
#line 2525 "hphp.y"
    { Token t; t.reset();
                                         (yyvsp[(1) - (1)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t); ;}
    break;

  case 728:

/* Line 1455 of yacc.c  */
#line 2528 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2530 "hphp.y"
    { only_in_hh_syntax(_p);
                                         (yyvsp[(1) - (4)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2534 "hphp.y"
    { only_in_hh_syntax(_p);
                                         _p->onTypeList((yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
                                         (yyvsp[(1) - (6)]).setText("array");
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (6)]), (yyvsp[(3) - (6)])); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2538 "hphp.y"
    { (yyvsp[(1) - (1)]).xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation((yyval), (yyvsp[(1) - (1)]), t);
                                         _p->onTypeSpecialization((yyval), 'x'); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 2544 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(7) - (8)]), (yyvsp[(4) - (8)]));
                                        _p->onTypeAnnotation((yyval), (yyvsp[(2) - (8)]), (yyvsp[(7) - (8)]));
                                        _p->onTypeSpecialization((yyval), 'f'); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2548 "hphp.y"
    { only_in_hh_syntax(_p);
                                        _p->onTypeList((yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation((yyval), t, (yyvsp[(2) - (5)]));
                                        _p->onTypeSpecialization((yyval), 't'); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 2556 "hphp.y"
    { (yyval) = (yyvsp[(1) - (1)]); ;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 2557 "hphp.y"
    { (yyval).reset(); ;}
    break;



/* Line 1455 of yacc.c  */
#line 10768 "new_hphp.tab.cpp"
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
#line 2560 "hphp.y"

bool Parser::parseImpl() {
  return yyparse(this) == 0;
}

