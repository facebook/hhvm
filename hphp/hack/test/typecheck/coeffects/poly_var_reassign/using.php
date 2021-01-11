<?hh

class X implements IDisposable {
  public function __dispose()[]: void {}
}

function using_expr(vec<int> $v)[$v::C]: void {
  using ($v = new X()) {}
}
