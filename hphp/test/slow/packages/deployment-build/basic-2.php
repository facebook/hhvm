<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

module prod;

<<__EntryPoint>>
function main(): void {
  var_dump(ini_get('hhvm.active_deployment'));
}
