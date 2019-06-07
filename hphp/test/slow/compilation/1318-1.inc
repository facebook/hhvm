<?hh

class X {
  private static $i = null;
  function foo() {
    self::$i = $this;
  }
}

$x = 1;
