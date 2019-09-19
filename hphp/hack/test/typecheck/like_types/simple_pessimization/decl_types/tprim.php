<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function tnull(null $x): void {
  hh_show($x);
}

function tint(int $x): void {
  hh_show($x);
}

function tbool(bool $x): void {
  hh_show($x);
}

function tstring(string $x): void {
  hh_show($x);
}

function tresource(resource $x): void {
  hh_show($x);
}

function tnum(num $x): void {
  hh_show($x);
}

function tarraykey(arraykey $x): void {
  hh_show($x);
}
