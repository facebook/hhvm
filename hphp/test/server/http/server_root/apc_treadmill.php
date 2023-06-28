<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function test_treadmill() :mixed{
  $v = keyset['a'.mt_rand() % 8];
  $key = "apc.key";
  apc_store($key, $v);
  $succ = false;
  $v = apc_fetch($key, inout $succ);
  apc_add($key, $v);
  echo 'see if it crashes after this request finishes';
}
