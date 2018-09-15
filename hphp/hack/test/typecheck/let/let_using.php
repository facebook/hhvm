<?hh // experimental

class D implements IDisposable {
  public function __dispose(): void {}
}

function foo(): void {
  using ($d = new D()) {
    let dd : D = $d;
  }
}
