<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function getit():int { return 5; }

<<__EntryPoint>>
function testit():void {
  $f = getit() |?> ($z ==> $z + $$);
  if ($f is null) {
    return;
  }
  $w = $f(100);
  echo $w;
}
