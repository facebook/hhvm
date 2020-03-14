<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  public ?int $foo = 42;
  public ?int $foo2 = 44;
  public ?int $foo3 = 45;
  public ?int $foo4 = 46;
  public ?int $foo5 = 47;
  public ?int $foo6 = 49;
  public ?int $foo7 = null;
  public $foo8 = "foo";
};

function h() {
  return new Exception();
}

function g($u, $v, $w, $x) {
  // create an exception so that we've loaded line number info
  // for all lines in the backtrace.
  h(); var_dump($u);
}

function f($a, $b) {
  g($a, $b, !$b, $a);
}

<<__EntryPoint>>
function main() {
  $a = new A();
  f($a, true);
  f($a, true);
  f($a, true);
}
