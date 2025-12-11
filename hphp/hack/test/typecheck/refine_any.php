<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

namespace HH_FIXME {
  type MISSING_TYPE_IN_HIERARCHY = mixed;
}

namespace Test {

function return_any():\HH_FIXME\MISSING_TYPE_IN_HIERARCHY {
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
  }
