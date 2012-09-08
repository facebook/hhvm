
#ifndef __GENERATED_cls_RecursiveDirectoryIterator_h20f53fae__
#define __GENERATED_cls_RecursiveDirectoryIterator_h20f53fae__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/DirectoryIterator.h>
#include <cls/RecursiveIterator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const VarNR &s_sys_svi86af027e;

/* SRC: classes/directoryiterator.php line 122 */
FORWARD_DECLARE_CLASS(RecursiveDirectoryIterator);
extern const ObjectStaticCallbacks cw_RecursiveDirectoryIterator;
class c_RecursiveDirectoryIterator : public c_DirectoryIterator {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_NO_SWEEP(RecursiveDirectoryIterator, RecursiveDirectoryIterator, DirectoryIterator)
  static const ClassPropTable os_prop_table;
  c_RecursiveDirectoryIterator(const ObjectStaticCallbacks *cb = &cw_RecursiveDirectoryIterator) : c_DirectoryIterator(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: void t___construct(Variant v_path, Variant v_flags = 16LL /* RecursiveDirectoryIterator::CURRENT_AS_FILEINFO */);
  public: c_RecursiveDirectoryIterator *create(CVarRef v_path, CVarRef v_flags = NAMVAR(s_sys_svi86af027e, 16LL) /* RecursiveDirectoryIterator::CURRENT_AS_FILEINFO */);
  public: Variant t_current();
  public: Variant t_key();
  public: void t_next();
  public: void t_rewind();
  public: void t_seek(CVarRef v_position);
  public: String t___tostring();
  public: bool t_valid();
  public: bool t_haschildren();
  public: Object t_getchildren();
  public: String t_getsubpath();
  public: String t_getsubpathname();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(current);
  DECLARE_METHOD_INVOKE_HELPERS(key);
  DECLARE_METHOD_INVOKE_HELPERS(next);
  DECLARE_METHOD_INVOKE_HELPERS(rewind);
  DECLARE_METHOD_INVOKE_HELPERS(seek);
  DECLARE_METHOD_INVOKE_HELPERS(__tostring);
  DECLARE_METHOD_INVOKE_HELPERS(valid);
  DECLARE_METHOD_INVOKE_HELPERS(haschildren);
  DECLARE_METHOD_INVOKE_HELPERS(getchildren);
  DECLARE_METHOD_INVOKE_HELPERS(getsubpath);
  DECLARE_METHOD_INVOKE_HELPERS(getsubpathname);
};
ObjectData *coo_RecursiveDirectoryIterator() NEVER_INLINE;
extern const int64 q_RecursiveDirectoryIterator$$CURRENT_AS_SELF;
extern const int64 q_RecursiveDirectoryIterator$$CURRENT_AS_FILEINFO;
extern const int64 q_RecursiveDirectoryIterator$$CURRENT_AS_PATHNAME;
extern const int64 q_RecursiveDirectoryIterator$$KEY_AS_PATHNAME;
extern const int64 q_RecursiveDirectoryIterator$$KEY_AS_FILENAME;
extern const int64 q_RecursiveDirectoryIterator$$NEW_CURRENT_AND_KEY;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_RecursiveDirectoryIterator_h20f53fae__
