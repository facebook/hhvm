<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function vecs(Vector<int> $v1, ImmVector<float> $v2, ConstVector<string> $v3, vec<bool> $v4): void {
  list($a, $b) = $v1;
  hh_show($a);
  hh_show($b);
  list($c, $d) = $v2;
  hh_show($c);
  hh_show($d);
  list($e, $f) = $v3;
  hh_show($e);
  hh_show($f);
  list($g, $h) = $v4;
  hh_show($g);
  hh_show($h);
}
