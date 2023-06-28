<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

namespace BuiltinBadParams;

<<__EntryPoint>>
function main() :mixed{
  try {
    \HH\prefetch_units(vec[], false);
  } catch (\Exception $e) {
    echo "Caught exception... " . $e->getMessage() . "\n";
  }

  try {
    \HH\prefetch_units(keyset['a', 'b', 3, 'c'], false);
  } catch (\Exception $e) {
    echo "Caught exception... " . $e->getMessage() . "\n";
  }
}
