<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  try {
    invariant();
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    invariant(true);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    invariant(__hhvm_intrinsics\launder_value(true));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    invariant(false);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    invariant(__hhvm_intrinsics\launder_value(false));
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
