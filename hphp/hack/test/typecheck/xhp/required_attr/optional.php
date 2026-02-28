<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class :a extends XHPTest { attribute int a @required, int b @lateinit; }
class :b extends XHPTest { attribute int a = 1; }

function foo(): void {
  $b = <b />;
  $a = <a {...$b} />;
}
