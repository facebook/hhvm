<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('capture_pipe_variables')>>

function getit():int { return 5; }

<<__EntryPoint>>
function testit():void {
  $f = getit() |> ($z ==> $z + $$);
  $w = $f(100);
  echo $w;
}
