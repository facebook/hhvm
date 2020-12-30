<?hh

class X implements IDisposable {
  public function __dispose()[defaults]: void {}
}

function pure()[]: void {
  using (new X()) {}
}
