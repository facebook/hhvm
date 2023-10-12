<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test2(int $id): void {
  $res = Map {};
  $res[$id] = vec[1];
  $res[$id][0] += 0;
  $res[$id][1] += 0;
  $res[$id][2] += 0;
  $res[$id][3] += 0;
  $res[$id][4] += 0;
  $res[$id][5] += 0;
  hh_force_solve();
  hh_show($res);
}
