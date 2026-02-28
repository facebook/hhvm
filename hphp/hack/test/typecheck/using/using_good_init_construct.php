<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose(): void {}
  public function foo(): void {}
}

class C {
  private int $x;
  public function __construct() {
    using new Handle();
    $this->x = 42;
  }
}
