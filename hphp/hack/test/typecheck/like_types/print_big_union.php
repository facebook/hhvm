<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function b(): bool { return true; }

function f(dynamic $d): void {
  if (b()) {
    $ak = null;
  } else if (b()){
    $ak = $d;
  } else if (b()){
    $ak = 4;
  } else {
    $ak = "str";
  }

  hh_show($ak);
}

function g(~?arraykey $ak): void {
  hh_show($ak);
}

function h(~?int $i, ~?string $s): void {
  if (b()) {
    $ak = $i;
  } else {
    $ak = $s;
  }
  hh_show($ak);
}
