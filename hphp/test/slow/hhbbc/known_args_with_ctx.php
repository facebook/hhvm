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


<<__EntryPoint>>
function main_known_args_with_ctx() {
var_dump((new Y)->f());
}
