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

bool isLoaded(const char* name);
inline bool isLoaded(const String& name) {
  return isLoaded(name.data());
}

Extension* get(const char* name);
inline Extension* get(const String& name) {
  return get(name.data());
}

Array getLoaded();

const std::vector<const Extension*> getExtensions();

// called by RuntimeOption to initialize all configurations of extension
void moduleLoad(const IniSetting::Map& ini, Hdf hdf);

// called by hphp_process_init/exit
void cliClientInit();
void moduleInit();
void moduleRegisterNative();
void moduleShutdown();
void threadInit();
void threadShutdown();
void requestInit();
void requestShutdown();

void moduleDeclInit();

bool modulesInitialised();

void serialize(jit::ProfDataSerializer& ser);
void deserialize(jit::ProfDataDeserializer& des);

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::ExtensionRegistry

}
