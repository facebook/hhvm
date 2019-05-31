<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class :a { attribute int a @required, int b @lateinit; }
class :b { attribute int a = 1; }

function foo(): void {
  $b = <b />;
  $a = <a {...$b} />;
}
