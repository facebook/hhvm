<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectFun((function():void) $f):void { }
function badreturn(bool $b):void {
  expectFun(() ==> {
  if ($b) {
    // We don't expect this FIXME to silence the second return statement!
    /* HH_FIXME[4110] */
    return;
  } else {
    return 3;
  }
  });
}
