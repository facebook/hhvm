// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
#pragma once

#include "hphp/util/process.h"

#include "rust/cxx.h"

inline rust::String Process_GetCPUModel() {
  return HPHP::Process::GetCPUModel();
}
inline rust::String Process_GetHostName() {
  return HPHP::Process::GetHostName();
}
