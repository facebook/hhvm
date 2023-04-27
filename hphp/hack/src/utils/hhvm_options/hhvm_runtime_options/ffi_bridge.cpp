// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

#include "hphp/util/process-cpu.h"
#include "hphp/util/process-host.h"

#include "rust/cxx.h"

rust::String Process_GetCPUModel() {
  return HPHP::Process::GetCPUModel();
}

rust::String Process_GetHostName() {
  return HPHP::Process::GetHostName();
}
