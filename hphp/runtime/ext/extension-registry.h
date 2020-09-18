#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/util/hdf.h"

namespace HPHP {

namespace jit {
struct ProfDataSerializer;
struct ProfDataDeserializer;
}

namespace ExtensionRegistry {
/////////////////////////////////////////////////////////////////////////////

void registerExtension(Extension* ext);

bool isLoaded(const char* name, bool enabled_only = true);
inline bool isLoaded(const String& name, bool enabled_only = true) {
  return isLoaded(name.data(), enabled_only);
}

Extension* get(const char* name, bool enabled_only = true);
inline Extension* get(const String& name, bool enabled_only = true) {
  return get(name.data(), enabled_only);
}

Array getLoaded(bool enabled_only = true);

// called by RuntimeOption to initialize all configurations of extension
void moduleLoad(const IniSetting::Map& ini, Hdf hdf);

// called by hphp_process_init/exit
void moduleInit();
void moduleShutdown();
void threadInit();
void threadShutdown();
void requestInit();
void requestShutdown();

bool modulesInitialised();

void serialize(jit::ProfDataSerializer& ser);
void deserialize(jit::ProfDataDeserializer& des);

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::ExtensionRegistry

}

