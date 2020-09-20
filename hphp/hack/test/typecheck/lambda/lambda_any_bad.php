<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.
function foo(bool $b): void {
  $f1 = () ==> { return vec[3]; };
  $x = ($f1())[0];
  $x->foo();
}
