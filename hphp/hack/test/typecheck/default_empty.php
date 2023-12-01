<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(darray<int,darray<int,int>> $d = dict[]): void {
  $x = $d[2][3];
}
