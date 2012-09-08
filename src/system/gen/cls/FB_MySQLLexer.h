
#ifndef __GENERATED_cls_FB_MySQLLexer_h486f2669__
#define __GENERATED_cls_FB_MySQLLexer_h486f2669__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern StaticStringProxy s_sys_ssp00000000;
#ifndef s_sys_ss00000000
#define s_sys_ss00000000 (*(StaticString *)(&s_sys_ssp00000000))
#endif

extern const VarNR &s_sys_svif01bca90;

extern VariantProxy s_sys_svsp00000000;
#ifndef s_sys_svs00000000
#define s_sys_svs00000000 (*(Variant *)&s_sys_svsp00000000)
#endif

/* SRC: classes/fbmysqllexer.php line 40 */
FORWARD_DECLARE_CLASS(FB_MySQLLexer);
extern const ObjectStaticCallbacks cw_FB_MySQLLexer;
class c_FB_MySQLLexer : public ExtObjectData {
  public:

  // Properties
  Variant m_symbols;
  Variant m_tokPtr;
  Variant m_tokStart;
  Variant m_tokLen;
  Variant m_tokText;
  Variant m_lineNo;
  Variant m_lineBegin;
  Variant m_string;
  Variant m_stringLen;
  Variant m_tokAbsStart;
  Variant m_skipText;
  Variant m_lookahead;
  Variant m_tokenStack;
  Variant m_stackPtr;

  // Destructor
  ~c_FB_MySQLLexer() NEVER_INLINE {}
  // Class Map
  DECLARE_CLASS_NO_SWEEP(FB_MySQLLexer, FB_MySQLLexer, ObjectData)
  static const ClassPropTable os_prop_table;
  c_FB_MySQLLexer(const ObjectStaticCallbacks *cb = &cw_FB_MySQLLexer) : ExtObjectData(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  void init();
  public: void t___construct(Variant v_string = NAMSTR(s_sys_ss00000000, ""), Variant v_lookahead = 0LL);
  public: c_FB_MySQLLexer *create(CVarRef v_string = NAMVAR(s_sys_svs00000000, ""), CVarRef v_lookahead = NAMVAR(s_sys_svif01bca90, 0LL));
  public: Variant t_get();
  public: void t_unget();
  public: Variant t_skip();
  public: void t_revert();
  public: bool t_iscompop(CVarRef v_c);
  public: void t_pushback();
  public: Variant t_lex();
  public: Variant t_nexttoken();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(get);
  DECLARE_METHOD_INVOKE_HELPERS(unget);
  DECLARE_METHOD_INVOKE_HELPERS(skip);
  DECLARE_METHOD_INVOKE_HELPERS(revert);
  DECLARE_METHOD_INVOKE_HELPERS(iscompop);
  DECLARE_METHOD_INVOKE_HELPERS(pushback);
  DECLARE_METHOD_INVOKE_HELPERS(lex);
  DECLARE_METHOD_INVOKE_HELPERS(nexttoken);
};
ObjectData *coo_FB_MySQLLexer() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_FB_MySQLLexer_h486f2669__
