<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function b(): bool { return true; }

function f(dynamic $d): void {
  if (b()){
    $li = $d;
  } else {
    $li = 4;
  }

  hh_show($li);
}

function g(~int $li): void {
  hh_show($li);
}

function h(~int $li, dynamic $d): void {
  if (b()) {
    $lli = $li;
  } else {
    $lli = $d;
  }
  // ~~int is just ~int
  hh_show($lli);
}
