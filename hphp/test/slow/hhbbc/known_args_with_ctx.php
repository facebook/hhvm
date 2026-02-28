<?hh

class X {
  static function f($x) :mixed{ return $x; }
}

class Y {
  function f() :mixed{
    $this->g();
    return X::f($this);
  }
  function g() :mixed{}
}


<<__EntryPoint>>
function main_known_args_with_ctx() :mixed{
var_dump((new Y)->f());
}
