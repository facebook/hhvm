<?hh

abstract class V { abstract const ctx C; }

class X implements IDisposable {
  public function __dispose()[]: void {}
}

function using_expr(V $v)[$v::C]: void {
  using ($v = new X()) {}
}
