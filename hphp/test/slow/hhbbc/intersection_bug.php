<?hh

class X {
  function foo(Y $y) {
    $z = $y->get();
    if ($z === @$this) {
      return $y;
    }
  }

  function bar(Y $y) {
    return $y;
  }
}

class Y {

  private static $getI;
  function get() : ?Y {
    if (self::$getI++ & 1) return $this;
    return null;
  }
}


<<__EntryPoint>>
function main_intersection_bug() {
var_dump(X::foo(new Y));
}
