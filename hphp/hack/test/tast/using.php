<?hh // strict

class Handle implements IDisposable {
  public function __dispose(): void {}
}

function test(): void {
  using ($x = new Handle(), $y = new Handle()) {
    using ($z = new Handle()) {}
  }
}
