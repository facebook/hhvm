<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function getit():int { return 5; }

<<__EntryPoint>>
function testit():void {
  $w = getit() |> (($z ==> $z)($$));
  echo $w;
}
