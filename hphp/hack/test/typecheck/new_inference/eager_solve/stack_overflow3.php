<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(
  Map<string, nonnull> $m,
): void {
  $a = Map {};
  $b = Map {};

  $a[''] = Map {};
  $b[''] = new Map($m);
  $b[''][''] = $a[''];
  $b[''] = new Map($m);
}
