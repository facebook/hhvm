
#ifndef __GENERATED_cls_DirectoryIterator_h523cccda__
#define __GENERATED_cls_DirectoryIterator_h523cccda__

#include <runtime/base/hphp_system.h>
#include <system/gen/sys/literal_strings_remap.h>
#include <system/gen/sys/scalar_arrays_remap.h>
#include <cls/SplFileInfo.h>
#include <cls/Traversable.h>
#include <cls/SeekableIterator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/directoryiterator.php line 11 */
FORWARD_DECLARE_CLASS(DirectoryIterator);
extern const ObjectStaticCallbacks cw_DirectoryIterator;
class c_DirectoryIterator : public c_SplFileInfo {
  public:

  // Properties

  // Class Map
  DECLARE_CLASS_NO_SWEEP(DirectoryIterator, DirectoryIterator, SplFileInfo)
  c_DirectoryIterator(const ObjectStaticCallbacks *cb = &cw_DirectoryIterator) : c_SplFileInfo(cb) {
    if (!hhvm) setAttribute(NoDestructor);
  }
  public: void t___construct(Variant v_path);
  public: c_DirectoryIterator *create(CVarRef v_path);
  public: Variant t_current();
  public: Variant t_key();
  public: void t_next();
  public: void t_rewind();
  public: void t_seek(CVarRef v_position);
  public: String t___tostring();
  public: bool t_valid();
  public: bool t_isdot();
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  DECLARE_METHOD_INVOKE_HELPERS(current);
  DECLARE_METHOD_INVOKE_HELPERS(key);
  DECLARE_METHOD_INVOKE_HELPERS(next);
  DECLARE_METHOD_INVOKE_HELPERS(rewind);
  DECLARE_METHOD_INVOKE_HELPERS(seek);
  DECLARE_METHOD_INVOKE_HELPERS(__tostring);
  DECLARE_METHOD_INVOKE_HELPERS(valid);
  DECLARE_METHOD_INVOKE_HELPERS(isdot);
};
ObjectData *coo_DirectoryIterator() NEVER_INLINE;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_DirectoryIterator_h523cccda__
