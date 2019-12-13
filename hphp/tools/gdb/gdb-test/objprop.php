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

<<__EntryPoint>>
function main() {
  var_dump(new A());
}
