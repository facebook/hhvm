<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
function foo(bool $b): void {
  $f1 = () ==> { return vec[3]; };
  $f2 = () ==> { $x = vec[3]; return $x; };
  $f3 = () ==> vec[3];

  $g1 = () ==> { return $b ? vec[3] : vec["a"]; };
  $g2 = () ==> { $x = $b ? vec[3] : vec["a"]; return $x; };
  $g3 = () ==> $b ? vec[3] : vec["a"];
  $g4 = () ==> { if ($b) return vec[3]; return vec["a"]; };
  hh_show($f1); hh_show($f2); hh_show($f3);
  hh_show($g1); hh_show($g2); hh_show($g3); hh_show($g4);
}
