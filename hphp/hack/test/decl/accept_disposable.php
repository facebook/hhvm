<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose(): void {}
  public function foo(): void {}
}

function f(<<__AcceptDisposable>> Handle $h): void {
}
