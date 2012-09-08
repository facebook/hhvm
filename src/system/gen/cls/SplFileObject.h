
#ifndef __GENERATED_cls_SplFileObject_h54740373__
#define __GENERATED_cls_SplFileObject_h54740373__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/SplFileInfo.h>
#include <cls/RecursiveIterator.h>
#include <cls/Traversable.h>
#include <cls/SeekableIterator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern StaticStringProxy s_sys_ssp7e5fc106;
#ifndef s_sys_ss7e5fc106
#define s_sys_ss7e5fc106 (*(StaticString *)(&s_sys_ssp7e5fc106))
#endif
extern StaticStringProxy s_sys_ssp5332baa7;
#ifndef s_sys_ss5332baa7
#define s_sys_ss5332baa7 (*(StaticString *)(&s_sys_ssp5332baa7))
#endif
extern StaticStringProxy s_sys_ssp559e789f;
#ifndef s_sys_ss559e789f
#define s_sys_ss559e789f (*(StaticString *)(&s_sys_ssp559e789f))
#endif
extern StaticStringProxy s_sys_ssp0d42ecf6;
#ifndef s_sys_ss0d42ecf6
#define s_sys_ss0d42ecf6 (*(StaticString *)(&s_sys_ssp0d42ecf6))
#endif

extern VariantProxy s_sys_svsp7e5fc106;
#ifndef s_sys_svs7e5fc106
#define s_sys_svs7e5fc106 (*(Variant *)&s_sys_svsp7e5fc106)
#endif
extern VariantProxy s_sys_svsp5332baa7;
#ifndef s_sys_svs5332baa7
#define s_sys_svs5332baa7 (*(Variant *)&s_sys_svsp5332baa7)
#endif
extern VariantProxy s_sys_svsp559e789f;
#ifndef s_sys_svs559e789f
#define s_sys_svs559e789f (*(Variant *)&s_sys_svsp559e789f)
#endif
extern VariantProxy s_sys_svsp0d42ecf6;
#ifndef s_sys_svs0d42ecf6
#define s_sys_svs0d42ecf6 (*(Variant *)&s_sys_svsp0d42ecf6)
#endif

/* SRC: classes/splfile.php line 384 */
FORWARD_DECLARE_CLASS(SplFileObject);
extern const ObjectStaticCallbacks cw_SplFileObject;
class c_SplFileObject : public c_SplFileInfo {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_NO_SWEEP(SplFileObject, SplFileObject, SplFileInfo)
  static const ClassPropTable os_prop_table;
  c_SplFileObject(const ObjectStaticCallbacks *cb = &cw_SplFileObject) : c_SplFileInfo(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: void t___construct(Variant v_filename, Variant v_open_mode = NAMSTR(s_sys_ss0d42ecf6, "r"), Variant v_use_include_path = false, Variant v_context = null);
  public: c_SplFileObject *create(CVarRef v_filename, CVarRef v_open_mode = NAMVAR(s_sys_svs0d42ecf6, "r"), CVarRef v_use_include_path = false_varNR, CVarRef v_context = null_variant);
  public: Variant t_current();
  public: bool t_eof();
  public: bool t_fflush();
  public: String t_fgetc();
  public: Variant t_fgetcsv(CVarRef v_delimiter = NAMVAR(s_sys_svs5332baa7, ","), CVarRef v_enclosure = NAMVAR(s_sys_svs7e5fc106, "\""), CVarRef v_escape = NAMVAR(s_sys_svs559e789f, "\\"));
  public: String t_fgets();
  public: String t_fgetss(CVarRef v_allowable_tags);
  public: bool t_flock(CVarRef v_operation, VRefParam rv_wouldblock);
  public: int64 t_fpassthru();
  public: Variant t_fscanf(int num_args, CVarRef v_format, Array args = Array());
  public: int64 t_fseek(CVarRef v_offset, CVarRef v_whence);
  public: Variant t_fstat();
  public: int64 t_ftell();
  public: bool t_ftruncate(CVarRef v_size);
  public: int64 t_fwrite(CVarRef v_str, CVarRef v_length);
  public: Variant t_getchildren();
  public: Variant t_getcsvcontrol();
  public: int64 t_getflags();
  public: int64 t_getmaxlinelen();
  public: bool t_haschildren();
  public: int64 t_key();
  public: void t_next();
  public: void t_rewind();
  public: void t_seek(CVarRef v_line_pos);
  public: void t_setcsvcontrol(CVarRef v_delimiter = NAMVAR(s_sys_svs5332baa7, ","), CVarRef v_enclosure = NAMVAR(s_sys_svs7e5fc106, "\""), CVarRef v_escape = NAMVAR(s_sys_svs559e789f, "\\"));
  public: void t_setflags(CVarRef v_flags);
  public: void t_setmaxlinelen(CVarRef v_max_len);
  public: bool t_valid();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(current);
  DECLARE_METHOD_INVOKE_HELPERS(eof);
  DECLARE_METHOD_INVOKE_HELPERS(fflush);
  DECLARE_METHOD_INVOKE_HELPERS(fgetc);
  DECLARE_METHOD_INVOKE_HELPERS(fgetcsv);
  DECLARE_METHOD_INVOKE_HELPERS(fgets);
  DECLARE_METHOD_INVOKE_HELPERS(fgetss);
  DECLARE_METHOD_INVOKE_HELPERS(flock);
  DECLARE_METHOD_INVOKE_HELPERS(fpassthru);
  DECLARE_METHOD_INVOKE_HELPERS(fscanf);
  DECLARE_METHOD_INVOKE_HELPERS(fseek);
  DECLARE_METHOD_INVOKE_HELPERS(fstat);
  DECLARE_METHOD_INVOKE_HELPERS(ftell);
  DECLARE_METHOD_INVOKE_HELPERS(ftruncate);
  DECLARE_METHOD_INVOKE_HELPERS(fwrite);
  DECLARE_METHOD_INVOKE_HELPERS(getchildren);
  DECLARE_METHOD_INVOKE_HELPERS(getcsvcontrol);
  DECLARE_METHOD_INVOKE_HELPERS(getflags);
  DECLARE_METHOD_INVOKE_HELPERS(getmaxlinelen);
  DECLARE_METHOD_INVOKE_HELPERS(haschildren);
  DECLARE_METHOD_INVOKE_HELPERS(key);
  DECLARE_METHOD_INVOKE_HELPERS(next);
  DECLARE_METHOD_INVOKE_HELPERS(rewind);
  DECLARE_METHOD_INVOKE_HELPERS(seek);
  DECLARE_METHOD_INVOKE_HELPERS(setcsvcontrol);
  DECLARE_METHOD_INVOKE_HELPERS(setflags);
  DECLARE_METHOD_INVOKE_HELPERS(setmaxlinelen);
  DECLARE_METHOD_INVOKE_HELPERS(valid);
};
ObjectData *coo_SplFileObject() NEVER_INLINE;
extern const int64 q_SplFileObject$$DROP_NEW_LINE;
extern const int64 q_SplFileObject$$READ_AHEAD;
extern const int64 q_SplFileObject$$SKIP_EMPTY;
extern const int64 q_SplFileObject$$READ_CSV;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_SplFileObject_h54740373__
