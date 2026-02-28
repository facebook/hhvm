<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose():void { }
  public function foo():void { }
}

function testit(bool $b):void {
  // This is legal
  using $x = new Handle();
  using $y = new Handle() {
  }

  // This is not
  if ($b) {
    using $z = new Handle();
  }
}
