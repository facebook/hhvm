<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

type USELESS_TYPE_ACTUALLY_MIXED<T> = T;

/* HH_FIXME[4101] Sadly, yes */
type TEMPORARY_MISSING_TYPE_MARKER = USELESS_TYPE_ACTUALLY_MIXED;

function return_any():TEMPORARY_MISSING_TYPE_MARKER {
  return 1;
}

function expect_nullable_int(?int $i):void { }
function test():void {
  $any = return_any();
  invariant(
    $any is KeyedContainer<_, _>,
    'failed',
  );
  $x = idx($any, 'result');
  expect_nullable_int($x);
}
