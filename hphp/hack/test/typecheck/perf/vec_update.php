<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test2(int $id): void {
  $res = Map {};
  $res[$id] = vec[1];
  $res[$id][0] += 0; // + 3 tyvars
  $res[$id][1] += 0; // + 6 tyvars
  $res[$id][2] += 0; // + 9 tyvars...
  $res[$id][3] += 0;
  $res[$id][4] += 0;
  $res[$id][5] += 0;
  $res[$id][6] += 0;
  $res[$id][7] += 0;
  $res[$id][8] += 0;
  $res[$id][9] += 0;
  $res[$id][10] += 0;
  $res[$id][11] += 0;
  $res[$id][12] += 0;
  $res[$id][13] += 0;
  $res[$id][14] += 0;
  $res[$id][15] += 0;
  $res[$id][16] += 0;
  $res[$id][17] += 0;
  $res[$id][18] += 0;
  $res[$id][19] += 0;
  $res[$id][20] += 0;
  $res[$id][21] += 0;
  $res[$id][22] += 0;
//  hh_show_env();
}
