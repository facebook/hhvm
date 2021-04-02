<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function autoload_path_symbol_inverse(): void {
  var_dump(HH\autoload_path_to_types(
    HH\autoload_type_to_path(A::class),
  ));
}
