%{
// macros for bison
#define YYSTYPE HPHP::HPHP_PARSER_NS::Token
#define YYSTYPE_IS_TRIVIAL false
#define YYLTYPE HPHP::Location
#define YYLTYPE_IS_TRIVIAL true
#define YYERROR_VERBOSE
#define YYINITDEPTH 500
#define YYLEX_PARAM _p

#include "hphp/compiler/parser/parser.h"
#include <folly/Conv.h>
#include "hphp/util/text-util.h"
#include "hphp/util/logger.h"

#define line0 r.line0
#define char0 r.char0
#define line1 r.line1
#define char1 r.char1

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
    if (N) {                                                            \
      (Current).first(YYRHSLOC (Rhs, 1));                               \
      (Current).last (YYRHSLOC (Rhs, N));                               \
    } else {                                                            \
      (Current).line0 = (Current).line1 = YYRHSLOC (Rhs, 0).line1;\
      (Current).char0 = (Current).char1 = YYRHSLOC (Rhs, 0).char1;\
    }                                                                   \
  while (0);                                                            \
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
  while (0)

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
  while (0)

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
  while (0)

# define YYSTACK_RELOCATE_RESET(Stack_alloc, Stack)                     \
  do                                                                    \
    {                                                                   \
      YYSIZE_T yynewbytes;                                              \
      YYCOPY_RESET (&yyptr->Stack_alloc, Stack, yysize);                \
      Stack = &yyptr->Stack_alloc;                                      \
      yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
      yyptr += yynewbytes / sizeof (*yyptr);                            \
    }                                                                   \
  while (0)

#define YYSTACK_CLEANUP                         \
  YYTOKEN_RESET (yyvs, yystacksize);            \
  if (yyvs != yyvsa) {                          \
    YYSTACK_FREE (yyvs);                        \
  }                                             \
  if (yyls != yylsa) {                          \
    YYSTACK_FREE (yyls);                        \
  }                                             \


// macros for rules
#define BEXP(...) _p->onBinaryOpExp(__VA_ARGS__);
#define UEXP(...) _p->onUnaryOpExp(__VA_ARGS__);

using namespace HPHP::HPHP_PARSER_NS;

typedef HPHP::ClosureType ClosureType;

///////////////////////////////////////////////////////////////////////////////
// helpers

static void scalar_num(Parser *_p, Token &out, const char *num) {
  Token t;
  t.setText(num);
  _p->onScalar(out, T_LNUMBER, t);
}

static void scalar_num(Parser *_p, Token &out, int num) {
  Token t;
  t.setText(folly::to<std::string>(num));
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
   * The bool, int, float, and string typenames are not given any special
   * treatment by the parser and are treated the same as regular class names
   * (which initially gets marked as type code 5). However, XHP wants to use
   * different type codes for bool, int, float, and string, so we need to fix
   * up the type code here to make XHP happy.
   */
  if (type.num() == 5) {
    auto* str = type.text().c_str();
    if (_p->scanner().isHHSyntaxEnabled()) {
      switch (type.text().size()) {
        case 6:
          if (!strcasecmp(str, "HH\\int")) {
            type.reset(); type.setNum(3);
          }
          break;
        case 7:
          if (!strcasecmp(str, "HH\\bool")) {
            type.reset(); type.setNum(2);
          }
          break;
        case 8:
          if (!strcasecmp(str, "HH\\float")) {
            type.reset(); type.setNum(8);
          } else if (!strcasecmp(str, "HH\\mixed")) {
            type.reset(); type.setNum(6);
          }
          break;
        case 9:
          if (!strcasecmp(str, "HH\\string")) {
            type.reset(); type.setNum(1);
          }
          break;
        default:
          break;
      }
    } else {
      switch (type.text().size()) {
        case 3:
          if (!strcasecmp(str, "int")) {
            type.reset(); type.setNum(3);
          }
          break;
        case 4:
          if (!strcasecmp(str, "bool")) {
            type.reset(); type.setNum(2);
          } else if (!strcasecmp(str, "real")) {
            type.reset(); type.setNum(8);
          }
          break;
        case 5:
          if (!strcasecmp(str, "float")) {
            type.reset(); type.setNum(8);
          } else if (!strcasecmp(str, "mixed")) {
            type.reset(); type.setNum(6);
          }
          break;
        case 6:
          if (!strcasecmp(str, "string")) {
            type.reset(); type.setNum(1);
          } else if (!strcasecmp(str, "double")) {
            type.reset(); type.setNum(8);
          }
          break;
        case 7:
          if (!strcasecmp(str, "integer")) {
            type.reset(); type.setNum(3);
          } else if (!strcasecmp(str, "boolean")) {
            type.reset(); type.setNum(2);
          }
          break;
        default:
          break;
      }
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
  HPHP::split(':', attributes.text().c_str(), classes, true);
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
    Token params1; _p->onCallParam(params1, NULL, param1, false, false);

    for (unsigned int i = 0; i < classes.size(); i++) {
      Token parent;  parent.set(T_STRING, classes[i]);
      Token cls;     _p->onName(cls, parent, Parser::StringName);
      Token fname;   fname.setText("__xhpAttributeDeclaration");
      Token param;   _p->onCall(param, 0, fname, dummy, &cls);

      Token params; _p->onCallParam(params, &params1, param, false, false);
      params1 = params;
    }

    Token params2; _p->onCallParam(params2, &params1, arrAttributes,
                                   false, false);

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
      "Syntax only allowed in Hack files (<?hh) or with -v "
        "Eval.EnableHipHopSyntax=true",
      _p);
  }
}

static void validate_hh_variadic_variant(Parser* _p,
                                         Token& userAttrs, Token& typehint,
                                         Token* mod) {
  if (!userAttrs.text().empty() || !typehint.text().empty() ||
     (mod && !mod->text().empty())) {
    HPHP_PARSER_ERROR("Variadic '...' should be followed by a '$variable'", _p);
  }
  only_in_hh_syntax(_p);
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
%}

%expect 2
%define api.pure
%lex-param {HPHP::HPHP_PARSER_NS::Parser *_p}
%parse-param {HPHP::HPHP_PARSER_NS::Parser *_p}

%left T_INCLUDE T_INCLUDE_ONCE T_EVAL T_REQUIRE T_REQUIRE_ONCE
%right T_LAMBDA_ARROW
%left ','
%nonassoc "..."
%left T_LOGICAL_OR
%left T_LOGICAL_XOR
%left T_LOGICAL_AND
%right T_PRINT
%left '=' T_PLUS_EQUAL T_MINUS_EQUAL T_MUL_EQUAL T_DIV_EQUAL T_CONCAT_EQUAL T_MOD_EQUAL T_AND_EQUAL T_OR_EQUAL T_XOR_EQUAL T_SL_EQUAL T_SR_EQUAL T_POW_EQUAL
%right T_AWAIT T_YIELD
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
%right T_POW
%right '['

%nonassoc T_NEW T_CLONE
%token T_EXIT
%token T_IF
%left T_ELSEIF
%left T_ELSE
%left T_ENDIF
%token T_LNUMBER  /* long */
%token T_DNUMBER  /* double */
%token T_ONUMBER  /* overflowed decimal, might get parsed as long or double */
%token T_STRING
%token T_STRING_VARNAME
%token T_VARIABLE
%token T_NUM_STRING
%token T_INLINE_HTML
%token T_HASHBANG
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
%token T_SUPER
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
%token T_NULLSAFE_OBJECT_OPERATOR
%token T_DOUBLE_ARROW
%token T_LIST
%token T_ARRAY
%token T_CALLABLE
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
%token T_DOUBLE_COLON
%token T_NAMESPACE
%token T_NS_C
%token T_DIR
%token T_NS_SEPARATOR

%token T_XHP_LABEL
%token T_XHP_TEXT
%token T_XHP_ATTRIBUTE
%token T_XHP_CATEGORY
%token T_XHP_CATEGORY_LABEL
%token T_XHP_CHILDREN
%token T_ENUM
%token T_XHP_REQUIRED

%token T_TRAIT
%token T_ELLIPSIS "..."
%token T_INSTEADOF
%token T_TRAIT_C

%token T_HH_ERROR
%token T_FINALLY

%token T_XHP_TAG_LT
%token T_XHP_TAG_GT
%token T_TYPELIST_LT
%token T_TYPELIST_GT
%token T_UNRESOLVED_LT

%token T_COLLECTION
%token T_SHAPE
%token T_TYPE
%token T_UNRESOLVED_TYPE
%token T_NEWTYPE
%token T_UNRESOLVED_NEWTYPE

%token T_COMPILER_HALT_OFFSET
%token T_ASYNC

%right T_FROM
%token T_WHERE
%token T_JOIN
%token T_IN
%token T_ON
%token T_EQUALS
%token T_INTO
%token T_LET
%token T_ORDERBY
%token T_ASCENDING
%token T_DESCENDING
%token T_SELECT
%token T_GROUP
%token T_BY

%token T_LAMBDA_OP
%token T_LAMBDA_CP
%token T_UNRESOLVED_OP

%%

start:
                                       { _p->onNewLabelScope(true);
                                         _p->initParseTree();}
    top_statement_list
                                       { _p->popLabelInfo();
                                         _p->finiParseTree();
                                         _p->onCompleteLabelScope(true);}
;

top_statement_list:
    top_statement_list
    top_statement                      { _p->addTopStatement($2);}
  |                                    { }
;
top_statement:
    statement                          { _p->nns($1.num(), $1.text()); $$ = $1;}
  | function_declaration_statement     { _p->nns(); $$ = $1;}
  | class_declaration_statement        { _p->nns(); $$ = $1;}
  | enum_declaration_statement         { _p->nns(); $$ = $1;}
  | trait_declaration_statement        { _p->nns(); $$ = $1;}
  | hh_type_alias_statement            { $$ = $1; }
  | T_HALT_COMPILER '(' ')' ';'        { _p->onHaltCompiler();
                                         _p->finiParseTree();
                                         YYACCEPT;}
  | T_NAMESPACE namespace_name ';'     { _p->onNamespaceStart($2.text(), true);
                                         $$.reset();}
  | T_NAMESPACE namespace_name '{'     { _p->onNamespaceStart($2.text());}
    top_statement_list '}'             { _p->onNamespaceEnd(); $$ = $5;}
  | T_NAMESPACE '{'                    { _p->onNamespaceStart("");}
    top_statement_list '}'             { _p->onNamespaceEnd(); $$ = $4;}
  | T_USE use_declarations ';'         { _p->nns(); $$.reset();}
  | T_USE T_FUNCTION
    use_fn_declarations ';'            { _p->nns(); $$.reset();}
  | T_USE T_CONST
    use_const_declarations ';'         { _p->nns(); $$.reset();}
  | constant_declaration ';'           { _p->nns();
                                         _p->finishStatement($$, $1); $$ = 1;}
;

ident:
    T_STRING                           { $$ = $1;}
  | T_SUPER                            { $$ = $1;}
  | T_XHP_ATTRIBUTE                    { $$ = $1;}
  | T_XHP_CATEGORY                     { $$ = $1;}
  | T_XHP_CHILDREN                     { $$ = $1;}
  | T_XHP_REQUIRED                     { $$ = $1;}
  | T_ENUM                             { $$ = $1;}
  | T_WHERE                            { $$ = $1;}
  | T_JOIN                             { $$ = $1;}
  | T_ON                               { $$ = $1;}
  | T_IN                               { $$ = $1;}
  | T_EQUALS                           { $$ = $1;}
  | T_INTO                             { $$ = $1;}
  | T_LET                              { $$ = $1;}
  | T_ORDERBY                          { $$ = $1;}
  | T_ASCENDING                        { $$ = $1;}
  | T_DESCENDING                       { $$ = $1;}
  | T_SELECT                           { $$ = $1;}
  | T_GROUP                            { $$ = $1;}
  | T_BY                               { $$ = $1;}
;

use_declarations:
    use_declarations ','
    use_declaration                    { }
  | use_declaration                    { }
;

use_fn_declarations:
    use_fn_declaration ','
    use_fn_declaration                 { }
  | use_fn_declaration                 { }
;

use_const_declarations:
    use_const_declaration ','
    use_const_declaration              { }
  | use_const_declaration              { }
;

use_declaration:
    namespace_name                     { _p->onUse($1.text(),"");}
  | T_NS_SEPARATOR namespace_name      { _p->onUse($2.text(),"");}
  | namespace_name T_AS ident          { _p->onUse($1.text(),$3.text());}
  | T_NS_SEPARATOR namespace_name
    T_AS ident                         { _p->onUse($2.text(),$4.text());}
;

use_fn_declaration:
    namespace_name                     { _p->onUseFunction($1.text(),"");}
  | T_NS_SEPARATOR namespace_name      { _p->onUseFunction($2.text(),"");}
  | namespace_name T_AS ident          { _p->onUseFunction($1.text(),$3.text());}
  | T_NS_SEPARATOR namespace_name
    T_AS ident                         { _p->onUseFunction($2.text(),$4.text());}
;

use_const_declaration:
    namespace_name                     { _p->onUseConst($1.text(),"");}
  | T_NS_SEPARATOR namespace_name      { _p->onUseConst($2.text(),"");}
  | namespace_name T_AS ident          { _p->onUseConst($1.text(),$3.text());}
  | T_NS_SEPARATOR namespace_name
    T_AS ident                         { _p->onUseConst($2.text(),$4.text());}
;

namespace_name:
    ident                              { $$ = $1;}
  | namespace_name T_NS_SEPARATOR
    ident                              { $$ = $1 + $2 + $3; $$ = $1.num() | 2;}
;
namespace_string_base:
    namespace_name                     { $$ = $1; $$ = $$.num() | 1;}
  | T_NAMESPACE T_NS_SEPARATOR
    namespace_name                     { $$.set($3.num() | 2, _p->nsDecl($3.text()));}
  | T_NS_SEPARATOR namespace_name      { $$ = $2; $$ = $$.num() | 2;}
;
namespace_string:
    namespace_string_base              { if ($1.num() & 1) {
                                           $1.setText(_p->resolve($1.text(),0));
                                         }
                                         $$ = $1;}
;
namespace_string_typeargs:
    namespace_string_base
    hh_typeargs_opt                    { if ($1.num() & 1) {
                                           $1.setText(_p->resolve($1.text(),0));
                                         }
                                         $$ = $1;}
;
class_namespace_string_typeargs:
    namespace_string_base
    hh_typeargs_opt                    { if ($1.num() & 1) {
                                           $1.setText(_p->resolve($1.text(),1));
                                         }
                                         _p->onTypeAnnotation($$, $1, $2);}
;
constant_declaration:
    constant_declaration ','
    hh_name_with_type
    '=' static_expr                    { $3.setText(_p->nsDecl($3.text()));
                                         _p->onConst($$,$3,$5);}
  | T_CONST hh_name_with_type '='
    static_expr                        { $2.setText(_p->nsDecl($2.text()));
                                         _p->onConst($$,$2,$4);}
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
  | T_IF parenthesis_expr
    statement
    elseif_list
    else_single                        { _p->onIf($$,$2,$3,$4,$5);}
  | T_IF parenthesis_expr ':'
    inner_statement_list
    new_elseif_list
    new_else_single
    T_ENDIF ';'                        { _p->onIf($$,$2,$4,$5,$6);}
  | T_WHILE parenthesis_expr           { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    while_statement                    { _p->popLabelScope();
                                         _p->onWhile($$,$2,$4);
                                         _p->onCompleteLabelScope(false);}

  | T_DO                               { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    statement T_WHILE parenthesis_expr
    ';'                                { _p->popLabelScope();
                                         _p->onDo($$,$3,$5);
                                         _p->onCompleteLabelScope(false);}
  | T_FOR '(' for_expr ';'
    for_expr ';' for_expr ')'          { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    for_statement                      { _p->popLabelScope();
                                         _p->onFor($$,$3,$5,$7,$10);
                                         _p->onCompleteLabelScope(false);}
  | T_SWITCH parenthesis_expr          { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    switch_case_list                   { _p->popLabelScope();
                                         _p->onSwitch($$,$2,$4);
                                         _p->onCompleteLabelScope(false);}
  | T_BREAK ';'                        { _p->onBreakContinue($$, true, NULL);}
  | T_BREAK expr ';'                   { _p->onBreakContinue($$, true, &$2);}
  | T_CONTINUE ';'                     { _p->onBreakContinue($$, false, NULL);}
  | T_CONTINUE expr ';'                { _p->onBreakContinue($$, false, &$2);}
  | T_RETURN ';'                       { _p->onReturn($$, NULL);}
  | T_RETURN expr ';'                  { _p->onReturn($$, &$2);}
  | T_YIELD T_BREAK ';'                { _p->onYieldBreak($$);}
  | T_GLOBAL global_var_list ';'       { _p->onGlobal($$, $2);}
  | T_STATIC static_var_list ';'       { _p->onStatic($$, $2);}
  | T_ECHO expr_list ';'               { _p->onEcho($$, $2, 0);}
  | T_UNSET '(' variable_list ')' ';'  { _p->onUnset($$, $3);}
  | ';'                                { $$.reset(); $$ = ';';}
  | T_INLINE_HTML                      { _p->onEcho($$, $1, 1);}
  | T_HASHBANG                         { _p->onHashBang($$, $1);
                                         $$ = T_HASHBANG;}
  | T_FOREACH '(' expr
    T_AS foreach_variable
    foreach_optional_arg ')'           { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    foreach_statement                  { _p->popLabelScope();
                                         _p->onForEach($$,$3,$5,$6,$9, false);
                                         _p->onCompleteLabelScope(false);}
  | T_FOREACH '(' expr
    T_AWAIT T_AS foreach_variable
    foreach_optional_arg ')'           { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    foreach_statement                  { _p->popLabelScope();
                                         _p->onForEach($$,$3,$6,$7,$10, true);
                                         _p->onCompleteLabelScope(false);}
  | T_DECLARE '(' declare_list ')'
    declare_statement                  { _p->onBlock($$, $5); $$ = T_DECLARE;}
  | T_TRY
    try_statement_list
    T_CATCH '('
    fully_qualified_class_name
    T_VARIABLE ')' '{'
    inner_statement_list '}'
    additional_catches                 { _p->onCompleteLabelScope(false);}
    optional_finally                   { _p->onTry($$,$2,$5,$6,$9,$11,$13);}
  | T_TRY
    try_statement_list
    T_FINALLY                          { _p->onCompleteLabelScope(false);}
    finally_statement_list             { _p->onTry($$, $2, $5);}
  | T_THROW expr ';'                   { _p->onThrow($$, $2);}
  | T_GOTO ident ';'                   { _p->onGoto($$, $2, true);
                                         _p->addGoto($2.text(),
                                                     _p->getRange(),
                                                     &$$);}
  | expr ';'                           { _p->onExpStatement($$, $1);}
  | yield_expr ';'                     { _p->onExpStatement($$, $1);}
  | yield_assign_expr ';'              { _p->onExpStatement($$, $1);}
  | yield_list_assign_expr ';'         { _p->onExpStatement($$, $1);}
  | await_expr ';'                     { _p->onExpStatement($$, $1);}
  | await_assign_expr ';'              { _p->onExpStatement($$, $1);}
  | T_RETURN await_expr ';'            { _p->onReturn($$, &$2); }
  | await_list_assign_expr ';'         { _p->onExpStatement($$, $1);}
  | query_assign_expr ';'              { _p->onExpStatement($$, $1);}
  | T_RETURN query_expr ';'            { _p->onReturn($$, &$2); }
  | ident ':'                          { _p->onLabel($$, $1);
                                         _p->addLabel($1.text(),
                                                      _p->getRange(),
                                                      &$$);
                                         _p->onScopeLabel($$, $1);}
;

try_statement_list:
   '{'                                 { _p->onNewLabelScope(false);}
   inner_statement_list '}'            { $$ = $3;}
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

finally_statement_list:
    '{'                                { _p->onNewLabelScope(false);
                                         _p->pushLabelScope();}
    inner_statement_list '}'           { _p->popLabelScope();
                                         _p->onFinally($$, $3);
                                         _p->onCompleteLabelScope(false);}
;

optional_finally:
    T_FINALLY finally_statement_list   { $$ = $2;}
  |                                    { $$.reset();}
;

is_reference:
    '&'                                { $$ = 1;}
  |                                    { $$.reset();}
;

function_loc:
    T_FUNCTION                         { _p->pushFuncLocation(); }
;

function_declaration_statement:
    function_loc
    is_reference hh_name_with_typevar  { $3.setText(_p->nsDecl($3.text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart($3);
                                         _p->pushLabelInfo();}
    '(' parameter_list ')'
    hh_opt_return_type
    function_body                      { _p->onFunction($$,nullptr,$8,$2,$3,$6,$9,nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
  | non_empty_member_modifiers
    function_loc
    is_reference hh_name_with_typevar  { $4.setText(_p->nsDecl($4.text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart($4);
                                         _p->pushLabelInfo();}
    '(' parameter_list ')'
    hh_opt_return_type
    function_body                      { _p->onFunction($$,&$1,$9,$3,$4,$7,$10,nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
  | non_empty_user_attributes
    method_modifiers function_loc
    is_reference hh_name_with_typevar  { $5.setText(_p->nsDecl($5.text()));
                                         _p->onNewLabelScope(true);
                                         _p->onFunctionStart($5);
                                         _p->pushLabelInfo();}
    '(' parameter_list ')'
    hh_opt_return_type
    function_body                      { _p->onFunction($$,&$2,$10,$4,$5,$8,$11,&$1);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
;

enum_declaration_statement:
    T_ENUM
    ident                              { $2.setText(_p->nsClassDecl($2.text()));
                                         _p->onClassStart(T_ENUM,$2);}
    ':' hh_type
    hh_opt_constraint
    '{' enum_statement_list '}'        { _p->onEnum($$,$2,$5,$8,0); }

  | non_empty_user_attributes
    T_ENUM
    ident                              { $3.setText(_p->nsClassDecl($3.text()));
                                         _p->onClassStart(T_ENUM,$3);}
    ':' hh_type
    hh_opt_constraint
    '{' enum_statement_list '}'        { _p->onEnum($$,$3,$6,$9,&$1); }

;

class_declaration_statement:
    class_entry_type
    class_decl_name                    { $2.setText(_p->nsClassDecl($2.text()));
                                         _p->onClassStart($1.num(),$2);}
    extends_from implements_list '{'
    class_statement_list '}'           { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,$7);
                                         } else {
                                           stmts = $7;
                                         }
                                         _p->onClass($$,$1.num(),$2,$4,$5,
                                                     stmts,0,nullptr);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();}
  | non_empty_user_attributes
    class_entry_type
    class_decl_name                    { $3.setText(_p->nsClassDecl($3.text()));
                                         _p->onClassStart($2.num(),$3);}
    extends_from implements_list '{'
    class_statement_list '}'           { Token stmts;
                                         if (_p->peekClass()) {
                                           xhp_collect_attributes(_p,stmts,$8);
                                         } else {
                                           stmts = $8;
                                         }
                                         _p->onClass($$,$2.num(),$3,$5,$6,
                                                     stmts,&$1,nullptr);
                                         if (_p->peekClass()) {
                                           _p->xhpResetAttributes();
                                         }
                                         _p->popClass();
                                         _p->popTypeScope();}
  | T_INTERFACE
    interface_decl_name                { $2.setText(_p->nsClassDecl($2.text()));
                                         _p->onClassStart(T_INTERFACE,$2);}
    interface_extends_list '{'
    class_statement_list '}'           { _p->onInterface($$,$2,$4,$6,0);
                                         _p->popClass();
                                         _p->popTypeScope();}
  | non_empty_user_attributes
    T_INTERFACE
    interface_decl_name                { $3.setText(_p->nsClassDecl($3.text()));
                                         _p->onClassStart(T_INTERFACE,$3);}
    interface_extends_list '{'
    class_statement_list '}'           { _p->onInterface($$,$3,$5,$7,&$1);
                                         _p->popClass();
                                         _p->popTypeScope();}
;

trait_declaration_statement:
    T_TRAIT
    trait_decl_name                    { $2.setText(_p->nsClassDecl($2.text()));
                                         _p->onClassStart(T_TRAIT, $2);}
    implements_list
    '{' class_statement_list '}'       { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass($$,T_TRAIT,$2,t_ext,$4,
                                                     $6, 0, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
  | non_empty_user_attributes
    T_TRAIT
    trait_decl_name                    { $3.setText(_p->nsClassDecl($3.text()));
                                         _p->onClassStart(T_TRAIT, $3);}
    implements_list
    '{' class_statement_list '}'       { Token t_ext;
                                         t_ext.reset();
                                         _p->onClass($$,T_TRAIT,$3,t_ext,$5,
                                                     $7, &$1, nullptr);
                                         _p->popClass();
                                         _p->popTypeScope();}
;
class_decl_name:
    hh_name_with_typevar               { _p->pushClass(false); $$ = $1;}
  | T_XHP_LABEL                        { $1.xhpLabel(); _p->pushTypeScope();
                                         _p->pushClass(true); $$ = $1;}
;
interface_decl_name:
    hh_name_with_typevar               { _p->pushClass(false); $$ = $1;}
;
trait_decl_name:
    hh_name_with_typevar               { _p->pushClass(false); $$ = $1;}
;
class_entry_type:
    T_CLASS                            { $$ = T_CLASS;}
  | T_ABSTRACT T_CLASS                 { $$ = T_ABSTRACT; }
  | T_ABSTRACT T_FINAL T_CLASS         { only_in_hh_syntax(_p);
      /* hacky, but transforming to a single token is quite convenient */
      $$ = T_STATIC; }
  | T_FINAL T_ABSTRACT T_CLASS         { only_in_hh_syntax(_p); $$ = T_STATIC; }
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
    variable                           { $$ = $1; $$ = 0;}
  | '&' variable                       { $$ = $2; $$ = 1;}
  | T_LIST '(' assignment_list ')'     { _p->onListAssignment($$, $3, NULL);}
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
    ident '=' static_expr
  | declare_list ','
    ident '=' static_expr
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
    elseif_list T_ELSEIF parenthesis_expr
    statement                          { _p->onElseIf($$,$1,$3,$4);}
  |                                    { $$.reset();}
;
new_elseif_list:
    new_elseif_list T_ELSEIF
    parenthesis_expr ':'
    inner_statement_list               { _p->onElseIf($$,$1,$3,$5);}
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

method_parameter_list:
    non_empty_method_parameter_list ','
    optional_user_attributes
    parameter_modifiers
    hh_type_opt "..." T_VARIABLE
                                      { _p->onVariadicParam($$,&$1,$5,$7,false,
                                                            &$3,&$4); }
  | non_empty_method_parameter_list ','
    optional_user_attributes
    parameter_modifiers
    hh_type_opt '&' "..." T_VARIABLE
                                      { _p->onVariadicParam($$,&$1,$5,$8,true,
                                                            &$3,&$4); }
  | non_empty_method_parameter_list ','
    optional_user_attributes
    parameter_modifiers
    hh_type_opt "..."
                                      { validate_hh_variadic_variant(
                                          _p, $3, $5, &$4);
                                        $$ = $1; }
  | non_empty_method_parameter_list
    hh_possible_comma                 { $$ = $1;}
  | optional_user_attributes
    parameter_modifiers
    hh_type_opt "..." T_VARIABLE
                                      { _p->onVariadicParam($$,NULL,$3,$5,false,
                                                            &$1,&$2); }
  | optional_user_attributes
    parameter_modifiers
    hh_type_opt '&' "..." T_VARIABLE
                                      { _p->onVariadicParam($$,NULL,$3,$6,true,
                                                            &$1,&$2); }
  | optional_user_attributes
    parameter_modifiers
    hh_type_opt "..."
                                      { validate_hh_variadic_variant(
                                          _p, $1, $3, &$2);
                                        $$.reset(); }
  |                                   { $$.reset(); }
;

non_empty_method_parameter_list:
    optional_user_attributes
    parameter_modifiers
    hh_type_opt T_VARIABLE             { _p->onParam($$,NULL,$3,$4,0,
                                                     NULL,&$1,&$2);}
  | optional_user_attributes
    parameter_modifiers
    hh_type_opt '&' T_VARIABLE         { _p->onParam($$,NULL,$3,$5,1,
                                                     NULL,&$1,&$2);}
  | optional_user_attributes
    parameter_modifiers
    hh_type_opt '&' T_VARIABLE
    '=' expr                           { _p->onParam($$,NULL,$3,$5,1,
                                                     &$7,&$1,&$2);}
  | optional_user_attributes
    parameter_modifiers
    hh_type_opt T_VARIABLE
    '=' expr                           { _p->onParam($$,NULL,$3,$4,0,
                                                     &$6,&$1,&$2);}
  | non_empty_method_parameter_list ','
    optional_user_attributes
    parameter_modifiers
    hh_type_opt T_VARIABLE             { _p->onParam($$,&$1,$5,$6,0,
                                                     NULL,&$3,&$4);}
  | non_empty_method_parameter_list ','
    optional_user_attributes
    parameter_modifiers
    hh_type_opt '&' T_VARIABLE         { _p->onParam($$,&$1,$5,$7,1,
                                                     NULL,&$3,&$4);}
  | non_empty_method_parameter_list ','
    optional_user_attributes
    parameter_modifiers
    hh_type_opt '&' T_VARIABLE
    '=' expr                           { _p->onParam($$,&$1,$5,$7,1,
                                                     &$9,&$3,&$4);}
  | non_empty_method_parameter_list ','
    optional_user_attributes
    parameter_modifiers
    hh_type_opt T_VARIABLE
    '=' expr                           { _p->onParam($$,&$1,$5,$6,0,
                                                     &$8,&$3,&$4);}
;

parameter_list:
    non_empty_parameter_list ','
    optional_user_attributes
    hh_type_opt "..." T_VARIABLE
                                      { _p->onVariadicParam($$,&$1,$4,$6,
                                        false,&$3,NULL); }
  | non_empty_parameter_list ','
    optional_user_attributes
    hh_type_opt '&' "..." T_VARIABLE
                                      { _p->onVariadicParam($$,&$1,$4,$7,
                                        true,&$3,NULL); }
  | non_empty_parameter_list ','
    optional_user_attributes
    hh_type_opt "..."
                                      { validate_hh_variadic_variant(
                                          _p, $3, $4, NULL);
                                        $$ = $1; }
  | non_empty_parameter_list
    hh_possible_comma                 { $$ = $1;}
  | optional_user_attributes
    hh_type_opt "..." T_VARIABLE
                                      { _p->onVariadicParam($$,NULL,$2,$4,
                                                            false,&$1,NULL); }
  | optional_user_attributes
    hh_type_opt '&' "..." T_VARIABLE
                                      { _p->onVariadicParam($$,NULL,$2,$5,
                                                            true,&$1,NULL); }
  | optional_user_attributes
    hh_type_opt "..."
                                      { validate_hh_variadic_variant(
                                          _p, $1, $2, NULL);
                                        $$.reset(); }
  |                                   { $$.reset();}
;

non_empty_parameter_list:
    optional_user_attributes
    hh_type_opt T_VARIABLE             { _p->onParam($$,NULL,$2,$3,false,
                                                     NULL,&$1,NULL); }
  | optional_user_attributes
    hh_type_opt '&' T_VARIABLE         { _p->onParam($$,NULL,$2,$4,true,
                                                     NULL,&$1,NULL); }
  | optional_user_attributes
    hh_type_opt '&' T_VARIABLE
    '=' expr                           { _p->onParam($$,NULL,$2,$4,true,
                                                     &$6,&$1,NULL); }
  | optional_user_attributes
    hh_type_opt T_VARIABLE
    '=' expr                           { _p->onParam($$,NULL,$2,$3,false,
                                                     &$5,&$1,NULL); }
  | non_empty_parameter_list ','
    optional_user_attributes
    hh_type_opt T_VARIABLE             { _p->onParam($$,&$1,$4,$5,false,
                                                     NULL,&$3,NULL); }
  | non_empty_parameter_list ','
    optional_user_attributes
    hh_type_opt '&' T_VARIABLE         { _p->onParam($$,&$1,$4,$6,true,
                                                     NULL,&$3,NULL); }
  | non_empty_parameter_list ','
    optional_user_attributes
    hh_type_opt '&' T_VARIABLE
    '=' expr                           { _p->onParam($$,&$1,$4,$6,true,
                                                     &$8,&$3,NULL); }
  | non_empty_parameter_list ','
    optional_user_attributes
    hh_type_opt T_VARIABLE
    '=' expr                           { _p->onParam($$,&$1,$4,$5,false,
                                                     &$7,&$3,NULL); }
;

function_call_parameter_list:
    non_empty_fcall_parameter_list
    hh_possible_comma                  { $$ = $1;}
  |                                    { $$.reset();}
;
non_empty_fcall_parameter_list:
  expr                               { _p->onCallParam($$,NULL,$1,false,false);}
| '&' variable                       { _p->onCallParam($$,NULL,$2,true,false);}
| "..." expr                         { _p->onCallParam($$,NULL,$2,false,true);}
| non_empty_fcall_parameter_list
',' expr                           { _p->onCallParam($$,&$1,$3,false, false);}
| non_empty_fcall_parameter_list
',' "..." expr                     { _p->onCallParam($$,&$1,$4,false,true);}
| non_empty_fcall_parameter_list
',' '&' variable                   { _p->onCallParam($$,&$1,$4,true, false);}
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
    '=' static_expr                    { _p->onStaticVariable($$,&$1,$3,&$5);}
  | T_VARIABLE                         { _p->onStaticVariable($$,0,$1,0);}
  | T_VARIABLE '=' static_expr         { _p->onStaticVariable($$,0,$1,&$3);}
;

enum_statement_list:
    enum_statement_list
    enum_statement                     { _p->onClassStatement($$, $1, $2);}
  |                                    { $$.reset();}
;
enum_statement:
    enum_constant_declaration ';'      { _p->onClassVariableStart
                                         ($$,NULL,$1,NULL);}
;
enum_constant_declaration:
    hh_name_with_type '='
    static_expr                         { _p->onClassConstant($$,0,$1,$3);}
;


class_statement_list:
    class_statement_list
    class_statement                    { _p->onClassStatement($$, $1, $2);}
  |                                    { $$.reset();}
;
class_statement:
    variable_modifiers                 { _p->onClassVariableModifer($1);}
    class_variable_declaration ';'     { _p->onClassVariableStart
                                         ($$,&$1,$3,NULL);}
  | non_empty_member_modifiers
    hh_type                            { _p->onClassVariableModifer($1);}
    class_variable_declaration ';'     { _p->onClassVariableStart
                                         ($$,&$1,$4,&$2);}
  | class_constant_declaration ';'     { _p->onClassVariableStart
                                         ($$,NULL,$1,NULL);}
  | class_abstract_constant_declaration ';'
                                       { _p->onClassVariableStart
                                         ($$,NULL,$1,NULL, true);}
  | class_type_constant_declaration ';' { $$ = $1; }
  | method_modifiers function_loc
    is_reference hh_name_with_typevar '('
                                       { _p->onNewLabelScope(true);
                                         _p->onMethodStart($4, $1);
                                         _p->pushLabelInfo();}
    method_parameter_list ')'
    hh_opt_return_type
    method_body
                                       { _p->onMethod($$,$1,$9,$3,$4,$7,$10,nullptr);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
  | non_empty_user_attributes
    method_modifiers function_loc
    is_reference hh_name_with_typevar '('
                                       { _p->onNewLabelScope(true);
                                         _p->onMethodStart($5, $2);
                                         _p->pushLabelInfo();}
    method_parameter_list ')'
    hh_opt_return_type
    method_body
                                       { _p->onMethod($$,$2,$10,$4,$5,$8,$11,&$1);
                                         _p->popLabelInfo();
                                         _p->popTypeScope();
                                         _p->onCompleteLabelScope(true);}
  | T_XHP_ATTRIBUTE
    xhp_attribute_stmt ';'             { _p->xhpSetAttributes($2);}
  | T_XHP_CATEGORY
    xhp_category_stmt ';'              { xhp_category_stmt(_p,$$,$2);}
  | T_XHP_CHILDREN
    xhp_children_stmt ';'              { xhp_children_stmt(_p,$$,$2);}
  | T_REQUIRE T_EXTENDS fully_qualified_class_name  ';'
                                       { _p->onClassRequire($$, $3, true); }
  | T_REQUIRE T_IMPLEMENTS fully_qualified_class_name ';'
                                       { _p->onClassRequire($$, $3, false); }
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
    class_namespace_string_typeargs
    T_DOUBLE_COLON
    ident
    T_INSTEADOF trait_list ';'         { _p->onTraitPrecRule($$,$1,$3,$5);}
;
trait_alias_rule:
    trait_alias_rule_method T_AS
    method_modifiers ident ';'         { _p->onTraitAliasRuleModify($$,$1,$3,
                                                                    $4);}
  | trait_alias_rule_method T_AS
    non_empty_member_modifiers ';'     { Token t; t.reset();
                                         _p->onTraitAliasRuleModify($$,$1,$3,
                                                                    t);}
;
trait_alias_rule_method:
    class_namespace_string_typeargs
    T_DOUBLE_COLON
    ident                              { _p->onTraitAliasRuleStart($$,$1,$3);}
  | ident                              { Token t; t.reset();
                                         _p->onTraitAliasRuleStart($$,t,$1);}
;

xhp_attribute_stmt:
    xhp_attribute_decl                 { xhp_attribute_list(_p,$$,
                                         _p->xhpGetAttributes(),$1);}
  | xhp_attribute_stmt ','
    xhp_attribute_decl                 { xhp_attribute_list(_p,$$, &$1,$3);}
;

xhp_attribute_decl:
    xhp_nullable_attribute_decl_type
    xhp_label_ws
    xhp_attribute_default
    xhp_attribute_is_required          { xhp_attribute(_p,$$,$1,$2,$3,$4);
                                         $$ = 1;}
  | T_XHP_LABEL                        { $$ = $1; $$ = 0;}
;

xhp_nullable_attribute_decl_type:
  '?' xhp_attribute_decl_type          { $$ = $2;}
  | xhp_attribute_decl_type
;

xhp_attribute_decl_type:
    T_ARRAY                            { $$ = 4;}
  | T_ARRAY T_TYPELIST_LT hh_type
    T_TYPELIST_GT                      { $$ = 4;}
  | T_ARRAY T_TYPELIST_LT hh_type ','
    hh_type T_TYPELIST_GT              { $$ = 4;}
  | fully_qualified_class_name         { /* This case handles all types other
                                            than "array", "var" and "enum".
                                            For now we just use type code 5;
                                            later xhp_attribute() will fix up
                                            the type code as appropriate. */
                                         $$ = 5; $$.setText($1);}
  | T_VAR                              { $$ = 6;}
  | T_ENUM '{'
    xhp_attribute_enum '}'             { $$ = $3; $$ = 7;}
  | T_CALLABLE                         { $$ = 9; }
;

xhp_attribute_enum:
    common_scalar                      { _p->onArrayPair($$,  0,0,$1,0);}
  | xhp_attribute_enum ','
    common_scalar                      { _p->onArrayPair($$,&$1,0,$3,0);}
;

xhp_attribute_default:
    '=' static_expr                    { $$ = $2;}
  |                                    { scalar_null(_p, $$);}
;

xhp_attribute_is_required:
    '@' T_XHP_REQUIRED                 { scalar_num(_p, $$, "1");}
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
  | ident                              { $$ = -1;
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
    ident                              { $$ = -1;
                                         if ($1.same("any")) $$ = 1; else
                                         if ($1.same("pcdata")) $$ = 2;}
  | T_XHP_LABEL                        { $1.xhpLabel();  $$ = $1; $$ = 3;}
  | T_XHP_CATEGORY_LABEL               { $1.xhpLabel(0); $$ = $1; $$ = 4;}
;

function_body:
    ';'                                { $$.reset();}
  | '{' inner_statement_list '}'       { _p->finishStatement($$, $2); $$ = 1;}
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
  | T_ASYNC                            { $$ = T_ASYNC;}
;

parameter_modifiers:
    parameter_modifier                 { $$ = $1;}
  |                                    { $$.reset();}
;
parameter_modifier:
    T_PUBLIC                           { $$ = T_PUBLIC;}
  | T_PROTECTED                        { $$ = T_PROTECTED;}
  | T_PRIVATE                          { $$ = T_PRIVATE;}
;
class_variable_declaration:
    class_variable_declaration ','
    T_VARIABLE                         { _p->onClassVariable($$,&$1,$3,0);}
  | class_variable_declaration ','
    T_VARIABLE '=' static_expr         { _p->onClassVariable($$,&$1,$3,&$5);}
  | T_VARIABLE                         { _p->onClassVariable($$,0,$1,0);}
  | T_VARIABLE '=' static_expr         { _p->onClassVariable($$,0,$1,&$3);}
;
class_constant_declaration:
    class_constant_declaration ','
    hh_name_with_type '=' static_expr   { _p->onClassConstant($$,&$1,$3,$5);}
  | T_CONST hh_name_with_type '='
    static_expr                         { _p->onClassConstant($$,0,$2,$4);}
;
class_abstract_constant_declaration:
    class_abstract_constant_declaration ','
    hh_name_with_type                   { _p->onClassAbstractConstant($$,&$1,$3);}
  | T_ABSTRACT T_CONST hh_name_with_type
                                        { _p->onClassAbstractConstant($$,NULL,$3);}
;
class_type_constant_declaration:
    T_ABSTRACT class_type_constant
      hh_opt_constraint                 { Token t;
                                          _p->onClassTypeConstant($$, $2, t);
                                          _p->popTypeScope(); }
  | class_type_constant
      hh_opt_constraint '=' hh_type     { _p->onClassTypeConstant($$, $1, $4);
                                          _p->popTypeScope(); }
class_type_constant:
    T_CONST T_TYPE hh_name_with_typevar { $$ = $3; }
;

expr_with_parens:
    '(' expr_with_parens ')'           { $$ = $2;}
  | T_NEW class_name_reference
    ctor_arguments                     { _p->onNewObject($$, $2, $3);}
  | T_CLONE expr                       { UEXP($$,$2,T_CLONE,1);}
  | xhp_tag                            { $$ = $1;}
  | collection_literal                 { $$ = $1;}

parenthesis_expr:
  '(' expr ')'                         { $$ = $2;}
;

expr_list:
    expr_list ',' expr                 { _p->onExprListElem($$, &$1, $3);}
  | expr                               { _p->onExprListElem($$, NULL, $1);}
;

for_expr:
    expr_list                          { $$ = $1;}
  |                                    { $$.reset();}
;

yield_expr:
    T_YIELD                            { _p->onYield($$, NULL);}
  | T_YIELD expr                       { _p->onYield($$, &$2);}
  | T_YIELD expr T_DOUBLE_ARROW expr   { _p->onYieldPair($$, &$2, &$4);}
;

yield_assign_expr:
    variable '=' yield_expr            { _p->onAssign($$, $1, $3, 0, true);}
;

yield_list_assign_expr:
    T_LIST '(' assignment_list ')'
    '=' yield_expr                     { _p->onListAssignment($$, $3, &$6, true);}
;

await_expr:
    T_AWAIT expr                       { _p->onAwait($$, $2); }
;

await_assign_expr:
    variable '=' await_expr            { _p->onAssign($$, $1, $3, 0, true);}
;

await_list_assign_expr:
    T_LIST '(' assignment_list ')'
    '=' await_expr                     { _p->onListAssignment($$, $3, &$6, true);}
;

expr:
    expr_no_variable                   { $$ = $1;}
  | variable                           { $$ = $1;}
  | expr_with_parens                   { $$ = $1;}
  | lambda_or_closure                  { $$ = $1;}
  | lambda_or_closure_with_parens      { $$ = $1;}
;

expr_no_variable:
    T_LIST '(' assignment_list ')'
    '=' expr                           { _p->onListAssignment($$, $3, &$6);}
  | variable '=' expr                  { _p->onAssign($$, $1, $3, 0);}
  | variable '=' '&' variable          { _p->onAssign($$, $1, $4, 1);}
  | variable '=' '&' T_NEW
    class_name_reference
    ctor_arguments                     { _p->onAssignNew($$,$1,$5,$6);}
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
  | variable T_POW_EQUAL expr          { BEXP($$,$1,$3,T_POW_EQUAL);}
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
  | expr T_POW expr                    { BEXP($$,$1,$3,T_POW);}
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
  | '(' expr_no_variable ')'           { $$ = $2;}
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
  | scalar                             { $$ = $1; }
  | array_literal                      { $$ = $1; }
  | shape_literal                      { $$ = $1; }
  | '`' backticks_expr '`'             { _p->onEncapsList($$,'`',$2);}
  | T_PRINT expr                       { UEXP($$,$2,T_PRINT,1);}
  | dim_expr                           { $$ = $1;}
;

lambda_use_vars:
    T_USE '('
    lexical_var_list
    hh_possible_comma
    ')'                                { $$ = $3;}
  |                                    { $$.reset();}
;

closure_expression:
    function_loc
    is_reference '('                   { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    parameter_list ')'
    hh_opt_return_type lambda_use_vars
    '{' inner_statement_list '}'       { _p->finishStatement($10, $10); $10 = 1;
                                         $$ = _p->onClosure(ClosureType::Long,
                                                            nullptr,
                                                            $2,$5,$8,$10,$7);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
  | non_empty_member_modifiers
    function_loc
    is_reference '('                   { Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo(); }
    parameter_list ')'
    hh_opt_return_type lambda_use_vars
    '{' inner_statement_list '}'       { _p->finishStatement($11, $11); $11 = 1;
                                         $$ = _p->onClosure(ClosureType::Long,
                                                            &$1,
                                                            $3,$6,$9,$11,$8);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
;

lambda_expression:
    T_VARIABLE                         { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam($1,NULL,u,$1,0,
                                                     NULL,NULL,NULL);}
    lambda_body                        { Token v; Token w; Token x;
                                         _p->finishStatement($3, $3); $3 = 1;
                                         $$ = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            v,$1,w,$3,x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
  | T_ASYNC
    T_VARIABLE                         { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();
                                         Token u;
                                         _p->onParam($2,NULL,u,$2,0,
                                                     NULL,NULL,NULL);}
    lambda_body                        { Token v; Token w; Token x;
                                         $1 = T_ASYNC;
                                         _p->onMemberModifier($1, nullptr, $1);
                                         _p->finishStatement($4, $4); $4 = 1;
                                         $$ = _p->onClosure(ClosureType::Short,
                                                            &$1,
                                                            v,$2,w,$4,x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
  | T_LAMBDA_OP                        { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    parameter_list
    T_LAMBDA_CP
    hh_opt_return_type
    lambda_body                        { Token u; Token v;
                                         _p->finishStatement($6, $6); $6 = 1;
                                         $$ = _p->onClosure(ClosureType::Short,
                                                            nullptr,
                                                            u,$3,v,$6,$5);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
  | T_ASYNC
    T_LAMBDA_OP                        { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    parameter_list
    T_LAMBDA_CP
    hh_opt_return_type
    lambda_body                        { Token u; Token v;
                                         $1 = T_ASYNC;
                                         _p->onMemberModifier($1, nullptr, $1);
                                         _p->finishStatement($7, $7); $7 = 1;
                                         $$ = _p->onClosure(ClosureType::Short,
                                                            &$1,
                                                            u,$4,v,$7,$6);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);}
  | T_ASYNC
    '{'                                { _p->pushFuncLocation();
                                         Token t;
                                         _p->onNewLabelScope(true);
                                         _p->onClosureStart(t);
                                         _p->pushLabelInfo();}
    inner_statement_list
    '}'                                { Token u; Token v; Token w; Token x;
                                         Token y;
                                         $1 = T_ASYNC;
                                         _p->onMemberModifier($1, nullptr, $1);
                                         _p->finishStatement($4, $4); $4 = 1;
                                         $$ = _p->onClosure(ClosureType::Short,
                                                            &$1,
                                                            u,v,w,$4,x);
                                         _p->popLabelInfo();
                                         _p->onCompleteLabelScope(true);
                                         _p->onCall($$,1,$$,y,NULL);}
;

lambda_body:
    T_LAMBDA_ARROW expr               { $$ = _p->onExprForLambda($2);}
  | T_LAMBDA_ARROW await_expr         { $$ = _p->onExprForLambda($2);}
  | T_LAMBDA_ARROW
    '{' inner_statement_list '}'      { $$ = $3; }
;

shape_keyname:
    T_CONSTANT_ENCAPSED_STRING        { validate_shape_keyname($1, _p);
                                        _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $1); }
  | class_constant                    { $$ = $1; }
;

non_empty_shape_pair_list:
    non_empty_shape_pair_list ','
      shape_keyname
      T_DOUBLE_ARROW
      expr                            { _p->onArrayPair($$,&$1,&$3,$5,0); }
  | shape_keyname
      T_DOUBLE_ARROW
      expr                            { _p->onArrayPair($$,  0,&$1,$3,0); }
;

non_empty_static_shape_pair_list:
    non_empty_static_shape_pair_list ','
      shape_keyname
      T_DOUBLE_ARROW
      static_expr                     { _p->onArrayPair($$,&$1,&$3,$5,0); }
  | shape_keyname
      T_DOUBLE_ARROW
      static_expr                     { _p->onArrayPair($$,  0,&$1,$3,0); }
;

shape_pair_list:
    non_empty_shape_pair_list
    possible_comma                    { $$ = $1; }
  |                                   { $$.reset(); }
;

static_shape_pair_list:
    non_empty_static_shape_pair_list
    possible_comma                    { $$ = $1; }
  |                                   { $$.reset(); }
;

shape_literal:
    T_SHAPE '(' shape_pair_list ')'   { _p->onArray($$, $3, T_ARRAY);}
;

array_literal:
    T_ARRAY '(' array_pair_list ')'   { _p->onArray($$,$3,T_ARRAY);}
  | '[' array_pair_list ']'           { _p->onArray($$,$2,T_ARRAY);}
;

collection_literal:
    fully_qualified_class_name
    '{' collection_init '}'            { Token t;
                                         _p->onName(t,$1,Parser::StringName);
                                         BEXP($$,t,$3,T_COLLECTION);}
;

static_collection_literal:
    fully_qualified_class_name
    '{' static_collection_init '}'     { Token t;
                                         _p->onName(t,$1,Parser::StringName);
                                         BEXP($$,t,$3,T_COLLECTION);}
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
  | lambda_or_closure_with_parens      { $$ = $1;}
  | T_CONSTANT_ENCAPSED_STRING         { _p->onScalar($$,
                                         T_CONSTANT_ENCAPSED_STRING, $1); }
  | '(' expr_no_variable ')'           { $$ = $2;}
;

query_expr:
    query_head query_body            { _p->onQuery($$, $1, $2); }
;

query_assign_expr:
    variable '=' query_expr          { _p->onAssign($$, $1, $3, 0, true);}
;

query_head:
  T_FROM T_VARIABLE T_IN expr        { _p->onFromClause($$, $2, $4); }
;

query_body:
    query_body_clauses select_or_group_clause
                                     { _p->onQueryBody($$, &$1, $2, NULL); }
  | query_body_clauses select_or_group_clause query_continuation
                                     { _p->onQueryBody($$, &$1, $2, &$3); }
  | select_or_group_clause
                                     { _p->onQueryBody($$, NULL, $1, NULL); }
  | select_or_group_clause query_continuation
                                     { _p->onQueryBody($$, NULL, $1, &$2); }
;

query_body_clauses:
    query_body_clause                { _p->onQueryBodyClause($$, NULL, $1); }
  | query_body_clauses query_body_clause
                                     { _p->onQueryBodyClause($$, &$1, $2); }
;

query_body_clause:
    from_clause                      { $$ = $1;}
  | let_clause                       { $$ = $1;}
  | where_clause                     { $$ = $1;}
  | join_clause                      { $$ = $1;}
  | join_into_clause                 { $$ = $1;}
  | orderby_clause                   { $$ = $1;}
;

from_clause:
    T_FROM T_VARIABLE T_IN expr      { _p->onFromClause($$, $2, $4); }
;

let_clause:
    T_LET T_VARIABLE '=' expr        { _p->onLetClause($$, $2, $4); }
;

where_clause:
    T_WHERE expr                     { _p->onWhereClause($$, $2); }
;

join_clause:
    T_JOIN T_VARIABLE T_IN expr T_ON expr T_EQUALS expr
                                     { _p->onJoinClause($$, $2, $4, $6, $8); }
;

join_into_clause:
    T_JOIN T_VARIABLE T_IN expr T_ON expr T_EQUALS expr T_INTO T_VARIABLE
                              { _p->onJoinIntoClause($$, $2, $4, $6, $8, $10); }
;

orderby_clause:
    T_ORDERBY orderings              { _p->onOrderbyClause($$, $2); }
;

orderings:
    ordering                         { _p->onOrdering($$, NULL, $1); }
  | orderings ',' ordering           { _p->onOrdering($$, &$1, $3); }
;

ordering:
    expr                             { _p->onOrderingExpr($$, $1, NULL); }
  | expr ordering_direction          { _p->onOrderingExpr($$, $1, &$2); }
;

ordering_direction:
    T_ASCENDING                      { $$ = $1;}
  | T_DESCENDING                     { $$ = $1;}
;

select_or_group_clause:
    select_clause                    { $$ = $1;}
  | group_clause                     { $$ = $1;}
;

select_clause:
    T_SELECT expr                    { _p->onSelectClause($$, $2); }
;

group_clause:
    T_GROUP expr T_BY expr           { _p->onGroupClause($$, $2, $4); }
;

query_continuation:
    T_INTO T_VARIABLE query_body     { _p->onIntoClause($$, $2, $3); }
;

lexical_var_list:
    lexical_var_list ',' T_VARIABLE    { _p->onClosureParam($$,&$1,$3,0);}
  | lexical_var_list ',' '&'T_VARIABLE { _p->onClosureParam($$,&$1,$4,1);}
  | T_VARIABLE                         { _p->onClosureParam($$,  0,$1,0);}
  | '&' T_VARIABLE                     { _p->onClosureParam($$,  0,$2,1);}
;

xhp_tag:
    T_XHP_TAG_LT
    T_XHP_LABEL
    xhp_tag_body
    T_XHP_TAG_GT                       { xhp_tag(_p,$$,$2,$3);}
;
xhp_tag_body:
    xhp_attributes '/'                 { Token t1; _p->onArray(t1,$1);
                                         Token t2; _p->onArray(t2,$2);
                                         Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onCallParam($1,NULL,t1,0,0);
                                         _p->onCallParam($$, &$1,t2,0,0);
                                         _p->onCallParam($1, &$1,file,0,0);
                                         _p->onCallParam($1, &$1,line,0,0);
                                         $$.setText("");}
  | xhp_attributes T_XHP_TAG_GT
    xhp_children T_XHP_TAG_LT '/'
    xhp_opt_end_label                  { Token file; scalar_file(_p, file);
                                         Token line; scalar_line(_p, line);
                                         _p->onArray($4,$1);
                                         _p->onArray($5,$3);
                                         _p->onCallParam($2,NULL,$4,0,0);
                                         _p->onCallParam($$, &$2,$5,0,0);
                                         _p->onCallParam($2, &$2,file,0,0);
                                         _p->onCallParam($2, &$2,line,0,0);
                                         $$.setText($6.text());}
;
xhp_opt_end_label:
                                       { $$.reset(); $$.setText("");}
  | T_XHP_LABEL                        { $$.reset(); $$.setText($1);}
;
xhp_attributes:
    xhp_attributes
    xhp_attribute_name '='
    xhp_attribute_value                { _p->onArrayPair($$,&$1,&$2,$4,0);}
  |                                    { $$.reset();}
;
xhp_children:
    xhp_children xhp_child             { _p->onArrayPair($$,&$1,0,$2,0);}
  |                                    { $$.reset();}
;
xhp_attribute_name:
    T_XHP_LABEL                        { _p->onScalar($$,
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
  | '{' expr '}'                       { $$ = $2; }
  | xhp_tag                            { $$ = $1; }
;

xhp_label_ws:
    xhp_bareword                       { $$ = $1;}
  | xhp_label_ws ':'
    xhp_bareword                       { $$ = $1 + ":" + $3;}
  | xhp_label_ws '-'
    xhp_bareword                       { $$ = $1 + "-" + $3;}
;
xhp_bareword:
    ident                              { $$ = $1;}
  | T_EXIT                             { $$ = $1;}
  | T_FUNCTION                         { $$ = $1;}
  | T_CONST                            { $$ = $1;}
  | T_RETURN                           { $$ = $1;}
  | T_YIELD                            { $$ = $1;}
  | T_AWAIT                            { $$ = $1;}
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
  | T_ASYNC                            { $$ = $1;}
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
  | T_COMPILER_HALT_OFFSET             { $$ = $1;}
  | T_TRAIT                            { $$ = $1;}
  | T_TRAIT_C                          { $$ = $1;}
  | T_INSTEADOF                        { $$ = $1;}
  | T_TYPE                             { $$ = $1;}
  | T_NEWTYPE                          { $$ = $1;}
  | T_SHAPE                            { $$ = $1;}
;

simple_function_call:
    namespace_string_typeargs '('
    function_call_parameter_list ')'   { _p->onCall($$,0,$1,$3,NULL);}
;

fully_qualified_class_name:
    class_namespace_string_typeargs    { $$ = $1;}
  | T_XHP_LABEL                        { $1.xhpLabel(); $$ = $1;}
;
static_class_name:
    fully_qualified_class_name         { _p->onName($$,$1,Parser::StringName);}
  | T_STATIC                           { _p->onName($$,$1,Parser::StaticName);}
  | reference_variable                 { _p->onName($$,$1,
                                         Parser::StaticClassExprName);}
;
class_name_reference:
    fully_qualified_class_name         { _p->onName($$,$1,Parser::StringName);}
  | T_STATIC                           { _p->onName($$,$1,Parser::StaticName);}
  | variable_no_calls                  { _p->onName($$,$1,Parser::ExprName);}
;

exit_expr:
    '(' ')'                            { $$.reset();}
  | parenthesis_expr                   { $$ = $1;}
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
  | T_ONUMBER                          { _p->onScalar($$, T_ONUMBER,  $1);}
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
  | T_COMPILER_HALT_OFFSET             { _p->onScalar($$, T_COMPILER_HALT_OFFSET, $1);}
  | T_START_HEREDOC
    T_ENCAPSED_AND_WHITESPACE
    T_END_HEREDOC                      { _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $2);}
  | T_START_HEREDOC
    T_END_HEREDOC                      { $$.setText(""); _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $$);}
;

static_expr:
    common_scalar                      { $$ = $1;}
  | namespace_string                   { _p->onConstantValue($$, $1);}
  | T_ARRAY '('
    static_array_pair_list ')'         { _p->onArray($$,$3,T_ARRAY); }
  | '[' static_array_pair_list ']'     { _p->onArray($$,$2,T_ARRAY); }
  | T_SHAPE '('
    static_shape_pair_list ')'         { _p->onArray($$,$3,T_ARRAY); }
  | static_class_constant              { $$ = $1;}
  | static_collection_literal          { $$ = $1;}
  | '(' static_expr ')'                { $$ = $2;}
  | static_expr T_BOOLEAN_OR
    static_expr                        { BEXP($$,$1,$3,T_BOOLEAN_OR);}
  | static_expr T_BOOLEAN_AND
    static_expr                        { BEXP($$,$1,$3,T_BOOLEAN_AND);}
  | static_expr T_LOGICAL_OR
    static_expr                        { BEXP($$,$1,$3,T_LOGICAL_OR);}
  | static_expr T_LOGICAL_AND
    static_expr                        { BEXP($$,$1,$3,T_LOGICAL_AND);}
  | static_expr T_LOGICAL_XOR
    static_expr                        { BEXP($$,$1,$3,T_LOGICAL_XOR);}
  | static_expr '|' static_expr        { BEXP($$,$1,$3,'|');}
  | static_expr '&' static_expr        { BEXP($$,$1,$3,'&');}
  | static_expr '^' static_expr        { BEXP($$,$1,$3,'^');}
  | static_expr '.' static_expr        { BEXP($$,$1,$3,'.');}
  | static_expr '+' static_expr        { BEXP($$,$1,$3,'+');}
  | static_expr '-' static_expr        { BEXP($$,$1,$3,'-');}
  | static_expr '*' static_expr        { BEXP($$,$1,$3,'*');}
  | static_expr '/' static_expr        { BEXP($$,$1,$3,'/');}
  | static_expr '%' static_expr        { BEXP($$,$1,$3,'%');}
  | static_expr T_SL static_expr       { BEXP($$,$1,$3,T_SL);}
  | static_expr T_SR static_expr       { BEXP($$,$1,$3,T_SR);}
  | static_expr T_POW static_expr      { BEXP($$,$1,$3,T_POW);}
  | '!' static_expr                    { UEXP($$,$2,'!',1);}
  | '~' static_expr                    { UEXP($$,$2,'~',1);}
  | '+' static_expr                    { UEXP($$,$2,'+',1);}
  | '-' static_expr                    { UEXP($$,$2,'-',1);}
  | static_expr T_IS_IDENTICAL
    static_expr                        { BEXP($$,$1,$3,T_IS_IDENTICAL);}
  | static_expr T_IS_NOT_IDENTICAL
    static_expr                        { BEXP($$,$1,$3,T_IS_NOT_IDENTICAL);}
  | static_expr T_IS_EQUAL
    static_expr                        { BEXP($$,$1,$3,T_IS_EQUAL);}
  | static_expr T_IS_NOT_EQUAL
    static_expr                        { BEXP($$,$1,$3,T_IS_NOT_EQUAL);}
  | static_expr '<' static_expr        { BEXP($$,$1,$3,'<');}
  | static_expr T_IS_SMALLER_OR_EQUAL
    static_expr                        { BEXP($$,$1,$3,
                                              T_IS_SMALLER_OR_EQUAL);}
  | static_expr '>' static_expr        { BEXP($$,$1,$3,'>');}
  | static_expr
    T_IS_GREATER_OR_EQUAL
    static_expr                        { BEXP($$,$1,$3,
                                              T_IS_GREATER_OR_EQUAL);}
  | static_expr '?' static_expr ':'
    static_expr                        { _p->onQOp($$, $1, &$3, $5);}
  | static_expr '?' ':' static_expr    { _p->onQOp($$, $1,   0, $4);}
;

static_class_constant:
    class_namespace_string_typeargs
    T_DOUBLE_COLON
    ident                              { _p->onClassConst($$, $1, $3, 1);}
  | T_XHP_LABEL T_DOUBLE_COLON
    ident                              { $1.xhpLabel();
                                         _p->onClassConst($$, $1, $3, 1);}
  | class_namespace_string_typeargs
    T_DOUBLE_COLON
    T_CLASS                            { _p->onClassClass($$, $1, $3, 1);}
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
hh_possible_comma:
    ','                                { only_in_hh_syntax(_p); $$.reset();}
  |                                    { $$.reset();}
;

non_empty_static_array_pair_list:
    non_empty_static_array_pair_list
    ',' static_expr T_DOUBLE_ARROW
    static_expr                        { _p->onArrayPair($$,&$1,&$3,$5,0);}
  | non_empty_static_array_pair_list
    ',' static_expr                    { _p->onArrayPair($$,&$1,  0,$3,0);}
  | static_expr T_DOUBLE_ARROW
    static_expr                        { _p->onArrayPair($$,  0,&$1,$3,0);}
  | static_expr                        { _p->onArrayPair($$,  0,  0,$1,0);}
;

common_scalar_ae:
    T_LNUMBER                          { _p->onScalar($$, T_LNUMBER,  $1);}
  | T_DNUMBER                          { _p->onScalar($$, T_DNUMBER,  $1);}
  | T_ONUMBER                          { _p->onScalar($$, T_ONUMBER,  $1);}
  | T_START_HEREDOC
    T_ENCAPSED_AND_WHITESPACE
    T_END_HEREDOC                      { _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $2);}
  | T_START_HEREDOC
    T_END_HEREDOC                      { $$.setText(""); _p->onScalar($$, T_CONSTANT_ENCAPSED_STRING, $$);}
;
static_numeric_scalar_ae:
    T_LNUMBER                          { _p->onScalar($$,T_LNUMBER,$1);}
  | T_DNUMBER                          { _p->onScalar($$,T_DNUMBER,$1);}
  | T_ONUMBER                          { _p->onScalar($$,T_ONUMBER,$1);}
  | ident                              { constant_ae(_p,$$,$1);}
;

static_string_expr_ae:
    T_CONSTANT_ENCAPSED_STRING         { _p->onScalar($$,
                                         T_CONSTANT_ENCAPSED_STRING,$1);}
  | T_CONSTANT_ENCAPSED_STRING '.' static_string_expr_ae
                                       { _p->onScalar($$,
                                         T_CONSTANT_ENCAPSED_STRING,
                                         $1 + $3);}
;

static_scalar_ae:
    common_scalar_ae
  | static_string_expr_ae              { $$ = $1;}
  | ident                              { constant_ae(_p,$$,$1);}
  | '+' static_numeric_scalar_ae       { UEXP($$,$2,'+',1);}
  | '-' static_numeric_scalar_ae       { UEXP($$,$2,'-',1);}
  | T_ARRAY '('
    static_array_pair_list_ae ')'      { _p->onArray($$,$3,T_ARRAY);}
  | '[' static_array_pair_list_ae ']'  { _p->onArray($$,$2,T_ARRAY);}
  | T_SHAPE '('
    static_shape_pair_list_ae ')'      { _p->onArray($$,$3,T_ARRAY); }
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

static_shape_pair_list_ae:
    non_empty_static_shape_pair_list_ae
    possible_comma                     { $$ = $1; }
  |                                    { $$.reset(); }
;
non_empty_static_shape_pair_list_ae:
    non_empty_static_shape_pair_list_ae
      ',' shape_keyname
      T_DOUBLE_ARROW static_scalar_ae  {  _p->onArrayPair($$,&$1,&$3,$5,0); }
  | shape_keyname
      T_DOUBLE_ARROW
      static_scalar_ae                 { _p->onArrayPair($$,  0,&$1,$3,0); }
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
    ',' ident
    attribute_static_scalar_list       { _p->onUserAttribute($$,&$1,$3,$4);}
  | ident
    attribute_static_scalar_list       { _p->onUserAttribute($$,  0,$1,$2);}
;
user_attribute_list:
                                       { only_in_hh_syntax(_p);}
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

object_operator:
    T_OBJECT_OPERATOR                  { $$ = $1; $$ = 0;}
  | T_NULLSAFE_OBJECT_OPERATOR         { $$ = $1; $$ = 1;}
;

object_property_name_no_variables:
    ident                              { $$ = $1; $$ = HPHP::ObjPropNormal;}
  | T_XHP_LABEL                        { $$ = $1; $$ = HPHP::ObjPropXhpAttr;}
  | '{' expr '}'                       { $$ = $2; $$ = HPHP::ObjPropNormal;}
;

object_property_name:
    object_property_name_no_variables  { $$ = $1;}
  | variable_no_objects                { $$ = $1; $$ = HPHP::ObjPropNormal;}
;

object_method_name_no_variables:
    ident                              { $$ = $1;}
  | '{' expr '}'                       { $$ = $2;}
;

object_method_name:
    object_method_name_no_variables    { $$ = $1;}
  | variable_no_objects                { $$ = $1;}
;

array_access:
    '[' dim_offset ']'                 { $$ = $2;}
  | '{' expr '}'                       { $$ = $2;}
;

dimmable_variable_access:
    dimmable_variable array_access     { _p->onRefDim($$, $1, $2);}
  | '(' expr_with_parens ')'
    array_access                       { _p->onRefDim($$, $2, $4);}
;

dimmable_variable_no_calls_access:
    dimmable_variable_no_calls
    array_access                       { _p->onRefDim($$, $1, $2);}
  | '(' expr_with_parens ')'
    array_access                       { _p->onRefDim($$, $2, $4);}
;

variable:
    variable_no_objects                { $$ = $1;}
  | simple_function_call               { $$ = $1;}
  | object_method_call                 { $$ = $1;}
  | class_method_call                  { $$ = $1;}
  | dimmable_variable_access           { $$ = $1;}
  | variable object_operator
    object_property_name            { _p->onObjectProperty(
                                        $$,
                                        $1,
                                        !$2.num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        $3
                                      );
                                    }
  | '(' expr_with_parens ')'
    object_operator
    object_property_name            { _p->onObjectProperty(
                                        $$,
                                        $2,
                                        !$4.num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        $5
                                      );
                                    }
  | static_class_name
    T_DOUBLE_COLON
    variable_no_objects                { _p->onStaticMember($$,$1,$3);}
  | callable_variable '('
    function_call_parameter_list ')'   { _p->onCall($$,1,$1,$3,NULL);}
  | lambda_or_closure_with_parens '('
    function_call_parameter_list ')'   { _p->onCall($$,1,$1,$3,NULL);}
  | '(' variable ')'                   { $$ = $2;}
;

dimmable_variable:
    simple_function_call               { $$ = $1;}
  | object_method_call                 { $$ = $1;}
  | class_method_call                  { $$ = $1;}
  | dimmable_variable_access           { $$ = $1;}
  | variable object_operator
    object_property_name_no_variables
                                    { _p->onObjectProperty(
                                        $$,
                                        $1,
                                        !$2.num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        $3
                                      );
                                    }
  | '(' expr_with_parens ')'
    object_operator
    object_property_name_no_variables
                                    { _p->onObjectProperty(
                                        $$,
                                        $2,
                                        !$4.num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        $5
                                      );
                                    }
  | callable_variable '('
    function_call_parameter_list ')'   { _p->onCall($$,1,$1,$3,NULL);}
  | '(' variable ')'                   { $$ = $2;}
;

callable_variable:
    variable_no_objects                { $$ = $1;}
  | dimmable_variable_access           { $$ = $1;}
  | '(' variable ')'                   { $$ = $2;}
;

lambda_or_closure_with_parens:
    '(' lambda_or_closure ')'          { $$ = $2;}
;

lambda_or_closure:
    closure_expression                 { $$ = $1;}
  | lambda_expression                  { $$ = $1;}
;

object_method_call:
    variable object_operator
    object_method_name hh_typeargs_opt '('
    function_call_parameter_list ')' { _p->onObjectMethodCall($$,$1,$2.num(),$3,$6);}
  | '(' expr_with_parens ')'
    object_operator
    object_method_name hh_typeargs_opt '('
    function_call_parameter_list ')' { _p->onObjectMethodCall($$,$2,$4.num(),$5,$8);}
;

class_method_call:
    static_class_name
    T_DOUBLE_COLON
    ident hh_typeargs_opt '('
    function_call_parameter_list ')'   { _p->onCall($$,0,$3,$6,&$1);}
  | static_class_name
    T_DOUBLE_COLON
    variable_no_objects '('
    function_call_parameter_list ')'   { _p->onCall($$,1,$3,$5,&$1);}
  | static_class_name
    T_DOUBLE_COLON
    '{' expr '}' '('
    function_call_parameter_list ')'   { _p->onCall($$,1,$4,$7,&$1);}
;

variable_no_objects:
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
    variable_no_objects                { $$ = $1;}
  | dimmable_variable_no_calls_access  { $$ = $1;}
  | variable_no_calls
    object_operator
    object_property_name            { _p->onObjectProperty(
                                        $$,
                                        $1,
                                        !$2.num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        $3
                                      );
                                    }
  | '(' expr_with_parens ')'
    object_operator
    object_property_name            { _p->onObjectProperty(
                                        $$,
                                        $2,
                                        !$4.num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        $5
                                      );
                                    }
  | static_class_name
    T_DOUBLE_COLON
    variable_no_objects                { _p->onStaticMember($$,$1,$3);}
  | '(' variable ')'                   { $$ = $2;}
;

dimmable_variable_no_calls:
  | dimmable_variable_no_calls_access  { $$ = $1;}
  | variable_no_calls object_operator
    object_property_name_no_variables
                                    { _p->onObjectProperty(
                                        $$,
                                        $1,
                                        !$2.num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        $3
                                      );
                                    }
  | '(' expr_with_parens ')'
    object_operator
    object_property_name_no_variables
                                    { _p->onObjectProperty(
                                        $$,
                                        $2,
                                        !$4.num()
                                          ? HPHP::PropAccessType::Normal
                                          : HPHP::PropAccessType::NullSafe,
                                        $5
                                      );
                                    }
  | '(' variable ')'                   { $$ = $2;}
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

collection_init:
    non_empty_collection_init
    possible_comma                     { $$ = $1;}
  |                                    { _p->onEmptyCollection($$);}
;
non_empty_collection_init:
    non_empty_collection_init
    ',' expr T_DOUBLE_ARROW expr       { _p->onCollectionPair($$,&$1,&$3,$5);}
  | non_empty_collection_init ',' expr { _p->onCollectionPair($$,&$1,  0,$3);}
  | expr T_DOUBLE_ARROW expr           { _p->onCollectionPair($$,  0,&$1,$3);}
  | expr                               { _p->onCollectionPair($$,  0,  0,$1);}
;

static_collection_init:
    non_empty_static_collection_init
    possible_comma                     { $$ = $1;}
  |                                    { _p->onEmptyCollection($$);}
;
non_empty_static_collection_init:
    non_empty_static_collection_init
    ',' static_expr T_DOUBLE_ARROW
    static_expr                        { _p->onCollectionPair($$,&$1,&$3,$5);}
  | non_empty_static_collection_init
    ',' static_expr                    { _p->onCollectionPair($$,&$1,  0,$3);}
  | static_expr T_DOUBLE_ARROW
    static_expr                        { _p->onCollectionPair($$,  0,&$1,$3);}
  | static_expr                        { _p->onCollectionPair($$,  0,  0,$1);}
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
  | T_VARIABLE object_operator
    ident                              { _p->encapObjProp(
                                           $$,
                                           $1,
                                           !$2.num()
                                            ? HPHP::PropAccessType::Normal
                                            : HPHP::PropAccessType::NullSafe,
                                           $3
                                         );
                                       }
  | T_DOLLAR_OPEN_CURLY_BRACES
    expr '}'                           { _p->onDynamicVariable($$, $2, 1);}
  | T_DOLLAR_OPEN_CURLY_BRACES
    T_STRING_VARNAME '[' expr ']' '}'  { _p->encapArray($$, $2, $4);}
  | T_CURLY_OPEN variable '}'          { $$ = $2;}
;
encaps_var_offset:
    ident                              { $$ = $1; $$ = T_STRING;}
  | T_NUM_STRING                       { $$ = $1; $$ = T_NUM_STRING;}
  | T_VARIABLE                         { $$ = $1; $$ = T_VARIABLE;}
;

internal_functions:
    T_ISSET '(' variable_list ')'      { UEXP($$,$3,T_ISSET,1);}
  | T_EMPTY '(' variable ')'           { UEXP($$,$3,T_EMPTY,1);}
  | T_EMPTY '(' expr_no_variable ')'   { UEXP($$,$3,'!',1);}
  | T_EMPTY '(' lambda_or_closure ')'  { UEXP($$,$3,'!',1);}
  | T_EMPTY '(' lambda_or_closure_with_parens ')' { UEXP($$,$3,'!',1);}
  | T_EMPTY '(' expr_with_parens ')'   { UEXP($$,$3,'!',1);}
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
    T_DOUBLE_COLON ident               { _p->onClassConst($$, $1, $3, 0);}
  | static_class_name
    T_DOUBLE_COLON T_CLASS             { _p->onClassClass($$, $1, $3, 0);}
;

/* hack productions -- these allow some extra stuff in hack
 * mode, but simplify down to the original thing
 */

hh_opt_constraint:
    /* empty */
  | T_AS hh_type
  ;

hh_type_alias_statement:
    T_TYPE hh_name_with_typevar
      '=' hh_type ';'                  { $2.setText(_p->nsClassDecl($2.text()));
                                         _p->onTypedef($$, $2, $4);
                                         _p->popTypeScope(); }
  | non_empty_user_attributes
    T_TYPE hh_name_with_typevar
      '=' hh_type ';'                  { $3.setText(_p->nsClassDecl($3.text()));
                                         _p->onTypedef($$, $3, $5);
                                         _p->popTypeScope(); }
  | T_NEWTYPE hh_name_with_typevar
    hh_opt_constraint '=' hh_type ';'  { $2.setText(_p->nsClassDecl($2.text()));
                                         _p->onTypedef($$, $2, $5);
                                         _p->popTypeScope(); }
  | non_empty_user_attributes
    T_NEWTYPE hh_name_with_typevar
    hh_opt_constraint '=' hh_type ';'  { $3.setText(_p->nsClassDecl($3.text()));
                                         _p->onTypedef($$, $3, $6);
                                         _p->popTypeScope(); }
;

hh_name_with_type:  /* foo -> int foo */
    ident                              { $$ = $1; }
  | hh_type ident                      { only_in_hh_syntax(_p); $$ = $2; }
;

hh_name_with_typevar:  /* foo -> foo<X,Y>; this adds a typevar scope
                        * and must be followed by a call to
                        * popTypeScope() */
    ident                              { _p->pushTypeScope(); $$ = $1; }
  | ident
    T_TYPELIST_LT
    hh_typevar_list
    T_TYPELIST_GT                      { _p->pushTypeScope(); $$ = $1; }
;

hh_typeargs_opt:
    T_TYPELIST_LT
    hh_type_list
    T_TYPELIST_GT                      { $$ = $2; }
  |                                    { $$.reset(); }
;

hh_non_empty_type_list:
    hh_type                            { Token t; t.reset();
                                         _p->onTypeList($1, t);
                                         $$ = $1; }
  | hh_non_empty_type_list ',' hh_type { _p->onTypeList($1, $3);
                                         $$ = $1; }
;

hh_type_list:
    hh_non_empty_type_list
    possible_comma                     { $$ = $1; }
;

hh_func_type_list:
    hh_non_empty_type_list
    ',' T_ELLIPSIS                     { $$ = $1; }
  | hh_type_list                       { $$ = $1; }
  | T_ELLIPSIS                         { $$.reset(); }
  |                                    { $$.reset(); }
;

hh_opt_return_type:
                                       { $$.reset(); }
  | ':' hh_type                        { only_in_hh_syntax(_p); $$ = $2; }
;

hh_constraint:
    T_AS hh_type
 |  T_SUPER hh_type

hh_typevar_list:
    hh_typevar_list ','
    hh_typevar_variance ident          { _p->addTypeVar($4.text()); }
 |  hh_typevar_variance ident          { _p->addTypeVar($2.text()); }
 |  hh_typevar_list ','
    hh_typevar_variance ident
    hh_constraint                      { _p->addTypeVar($4.text()); }
 |  hh_typevar_variance ident
    hh_constraint                      { _p->addTypeVar($2.text()); }
;

hh_typevar_variance:
    '+'           {}
|   '-'           {}
|   /* empty */   {}
;

hh_shape_member_type:
    T_CONSTANT_ENCAPSED_STRING
      T_DOUBLE_ARROW
      hh_type                      { validate_shape_keyname($1, _p); }
 |   '?'
      T_CONSTANT_ENCAPSED_STRING
      T_DOUBLE_ARROW
      hh_type                      { validate_shape_keyname($2, _p); }
 |  class_namespace_string_typeargs
      T_DOUBLE_COLON
      ident
     T_DOUBLE_ARROW
      hh_type                      { }

;

hh_non_empty_shape_member_list:
    hh_non_empty_shape_member_list ','
      hh_shape_member_type
  | hh_shape_member_type
;

hh_shape_member_list:
    hh_non_empty_shape_member_list
    possible_comma                     { $$ = $1; }
  | /* empty */
{}

hh_shape_type:
    T_SHAPE
     '(' hh_shape_member_list ')'      { $$.setText("array"); }
;

hh_access_type_start:
    class_namespace_string_typeargs         { $$ = $1; }
;
hh_access_type:
    ident T_DOUBLE_COLON hh_access_type { Token t; t.reset();
                                          _p->onTypeAnnotation($$, $1, t);
                                          _p->onTypeList($$, $3); }
  | ident hh_typeargs_opt               { _p->onTypeAnnotation($$, $1, $2); }
;

/* extends non_empty_type_decl with some more types */
hh_type:
    /* double-optional types will be rejected by the typechecker; we
     * already allow plenty of nonsense types anyway */
    '?' hh_type                        { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization($2, '?');
                                         $$ = $2; }
  | '@' hh_type                        { only_in_hh_syntax(_p);
                                         _p->onTypeSpecialization($2, '@');
                                         $$ = $2; }
  | class_namespace_string_typeargs    { $$ = $1; }
  | T_ARRAY                            { Token t; t.reset();
                                         $1.setText("array");
                                         _p->onTypeAnnotation($$, $1, t); }
  | T_CALLABLE                         { Token t; t.reset();
                                         $1.setText("callable");
                                         _p->onTypeAnnotation($$, $1, t); }
  | hh_shape_type                      { $$ = $1; }
  | hh_access_type_start
    T_DOUBLE_COLON
    hh_access_type                     { only_in_hh_syntax(_p);
                                         _p->onTypeAnnotation($$, $1, $3);
                                         _p->onTypeSpecialization($$, 'a'); }
  | T_ARRAY T_TYPELIST_LT hh_type
    T_TYPELIST_GT                      { $1.setText("array");
                                         _p->onTypeAnnotation($$, $1, $3); }
  | T_ARRAY T_TYPELIST_LT hh_type ','
    hh_type T_TYPELIST_GT              { _p->onTypeList($3, $5);
                                         $1.setText("array");
                                         _p->onTypeAnnotation($$, $1, $3); }
  | T_XHP_LABEL                        { $1.xhpLabel();
                                         Token t; t.reset();
                                         _p->onTypeAnnotation($$, $1, t);
                                         _p->onTypeSpecialization($$, 'x'); }
  | '(' T_FUNCTION
    '(' hh_func_type_list ')'
    ':' hh_type ')'                   { only_in_hh_syntax(_p);
                                        _p->onTypeList($7, $4);
                                        _p->onTypeAnnotation($$, $2, $7);
                                        _p->onTypeSpecialization($$, 'f'); }
  | '(' hh_type ','
    hh_non_empty_type_list
    possible_comma ')'                { only_in_hh_syntax(_p);
                                        _p->onTypeList($2, $4);
                                        Token t; t.reset(); t.setText("array");
                                        _p->onTypeAnnotation($$, t, $2);
                                        _p->onTypeSpecialization($$, 't'); }
;

hh_type_opt:
    hh_type                            { $$ = $1; }
  |                                    { $$.reset(); }
;

%%
bool Parser::parseImpl() {
  return yyparse(this) == 0;
}
