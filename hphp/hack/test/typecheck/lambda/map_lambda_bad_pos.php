<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(Map<string, bool> $m): void {
  // We expect the error to be here, not in the .hhi file
  $x = $m->mapWithKey((int $k, bool $v) ==> $v);
}
