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
  function get() : ?Y {
    static $i;
    if ($i++ & 1) return $this;
    return null;
  }
}

var_dump(X::foo(new Y));
