<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// EnableIntrinsicsExtension
<<__EntryPoint>>
function main(): void {
  var_dump(ini_get('hhvm.abort_build_on_verify_error'));
}
