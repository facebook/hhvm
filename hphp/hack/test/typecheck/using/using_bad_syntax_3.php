<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose():void { }
  public function foo():void { }
}
class ScopeGuard implements IDisposable {
  public function __construct((function():void) $f) { }
  public function __dispose():void { }
}

function testit():void {
  // This is legal
  using $x = new Handle();

  using ($w = new Handle()) {
    // This is not
    using $q = new ScopeGuard(() ==> {});
  }
}
