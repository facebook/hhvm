<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<Policied("A")>>
  public int $a = 0;

  <<Policied("B")>>
  public int $b = 0;
}

function switch_basic(C $c): void {
  switch ($c->a) {
    case 0:
      // illegal! leaks PC
      $c->b = 1;
      break;
    default:
      break;
  }
  $c->b = 1; // ok, PC is erased at this point
}

function leak_in_case_expr(C $c, int $x): void {
  switch ($x) {
    case $c->a:
      $c->b = $x;
      break;
    default:
      break;
  }
}

function leak_in_fallthrough(C $c, int $x): void {
  $y = 0;
  switch ($x) {
    case 0:
      $y = $c->a;
      // FALLTHROUGH
    case 1:
      $c->b = $y;
      break;
    default:
      break;
  }
}

function leak_after_switch(C $c, int $x): void {
  switch ($x) {
    case 0:
      $y = $c->a;
      break;
    default:
      $y = 1;
      break;
  }
  $c->b = $y;
}

function leak_fallthrough_pc(C $c, int $x): void {
  switch ($x) {
    case 0:
      if ($c->a > 0) {
        return;
      }
      // FALLTHROUGH
    default:
      $c->b = 1;
  }
}

function leak_pc_after_switch(C $c): void {
  switch ($c->a) {
    case 0:
      return;
    default:
      break;
  }
  $c->b = 1;
}

function leak_pc_complex(C $c, int $x, Exception $e): void {
  switch ($x) {
    case 0:
      $c->b = 0; // ok, no leak
      break;
    case $c->a:
      $c->b = 1; // illegal! we know A equals $x
      break;
    case 2:
      $c->b = 2; // illegal! we know A is not 2
      break;
    default:
      $c->b = 3; // illegal! we know that A is not $x
  }
}

function leak_pc_in_case_exp(C $c): int {
  $f = () ==> {
    $c->a = 0;
    return 1;
  };
  switch ($c->b) {
    case $f():
      return 0;
    default:
      return 1;
  }
}
