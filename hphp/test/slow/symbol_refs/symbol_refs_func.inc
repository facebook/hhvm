<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function some_func() {
  return __hhvm_intrinsics\launder_value("abc");
}
