#ifndef incl_ZEND_EXTENSION_H_
#define incl_ZEND_EXTENSION_H_

#include "hphp/runtime/ext/extension.h"

typedef struct _zend_module_entry zend_module_entry;
struct _zend_module_entry;

namespace HPHP {

class ZendExtension final : public Extension {
private:
  zend_module_entry *getEntry();
public:
  /* implicit */ ZendExtension(const char* name);
  void moduleInit() override;
  void moduleShutdown() override;
  void threadShutdown() override;
  void requestInit() override;
  void requestShutdown() override;
  bool moduleEnabled() const override;
  static ZendExtension* GetByModuleNumber(int module_number);
};

}
#endif
