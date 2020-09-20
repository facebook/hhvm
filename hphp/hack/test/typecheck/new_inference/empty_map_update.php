<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_empty_map_update(Map<string,int> $msi): void {
  $m = Map {};
  $data = idx($m, 3);
  if ($data === null) {
    $data = $msi;
  }
  $data['x'] = 5;
  $m[4] = $data;
}
