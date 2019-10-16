<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function getit():int { return 32; }
<<__EntryPoint>>
function leaky():void {
  try {
    $x = getit() |> $$ / 0;
  }
  catch (Exception $_) {
    echo($$);
  }
}
