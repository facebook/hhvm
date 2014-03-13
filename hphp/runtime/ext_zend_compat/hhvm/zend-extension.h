#ifndef incl_ZEND_EXTENSION_H_
#define incl_ZEND_EXTENSION_H_

#include "hphp/runtime/ext/extension.h"

typedef struct _zend_module_entry zend_module_entry;
struct _zend_module_entry;

class ZendExtension : public HPHP::Extension {
private:
  zend_module_entry *getEntry();
public:
  /* implicit */ ZendExtension(const char* name);
  virtual void moduleInit() override;
  virtual void moduleShutdown() override;
  virtual void threadShutdown() override;
  virtual void requestInit() override;
  virtual void requestShutdown() override;
  static ZendExtension* GetByModuleNumber(int module_number);
};

#endif
