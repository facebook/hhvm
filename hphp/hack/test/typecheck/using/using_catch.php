<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose():void { }
  public function bar():void { echo "Handle::bar "; }
}
class Bary {
  public function bar():void { echo "Bary::bar "; }
}

<<__EntryPoint>>
function testit():void {
  $h = new Bary();
  try {
  using ($h = new Handle()) {
    // Still in scope
    $h->bar();
    throw new Exception("E");
  }
  } catch (Exception $_) {
    // Should be an error!
    $h->bar();
  }
}
