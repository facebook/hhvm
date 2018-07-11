<?hh

class X {
  static function f($x) { return $x; }
}

class Y {
  function f() {
    $this->g();
    return X::f($this);
  }
  function g() {}
}

var_dump((new Y)->f());
