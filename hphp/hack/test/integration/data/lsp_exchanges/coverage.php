<?hh

function f(X $x): void {
  $id = $x->getID();
}

class X {
  public function getID() : int { return 42; }
}
