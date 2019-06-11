<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function tnull(~null $i): null {
  return $i; // corner case: localizes to ?dynamic (T45650596)
}

function tint(~int $i): int {
  return $i; // ok
}

function tfloat(~float $i): float {
  return $i; // ok
}

function tbool(~bool $i): bool {
  return $i; // ok
}

function tstring(~string $i): string {
  return $i; // ok
}

function tresource(~resource $i): resource {
  return $i; // ok
}

function tnum(~num $i): num {
  return $i; // ok
}

function tarraykey(~arraykey $i): arraykey {
  return $i; // ok
}

function tarraykey2(~int $i): arraykey {
  return $i; // ok
}

function tarraykey3(~string $i): arraykey {
  return $i; // ok
}
