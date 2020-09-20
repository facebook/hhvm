<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class FakeArray<+Tk as arraykey, +T> { }

function foo<T>(T $input): T {
  if ($input is int || HH\is_php_array($input)) {
    return $input;
  }
  invariant_violation('!!!');
}

function mimc<T>(T $input): T {
  if ($input is int || $input is FakeArray<_,_>) {
    return $input;
  }
  invariant_violation('!!!');
}

function foo2<T>(T $input): T {
  if (HH\is_php_array($input)) {
    return $input;
  }
  invariant_violation('!!!');
}

function mimc2<T>(T $input): T {
  if ($input is FakeArray<_,_>) {
    return $input;
  }
  invariant_violation('!!!');
}
