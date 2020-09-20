<?hh

class X {
  function getP() {

    return HhbbcThisType003::$g;
  }
  function getY() : Y {
    $n = $this;
    while (!($n is Y)) $n = $this->getP();
    return $n;
  }
}

class Y extends X {}

class Z extends X {
  function f() {
    return $this->getY();
  }
}

abstract final class HhbbcThisType003 {
  public static $g;
}
<<__EntryPoint>>
function entrypoint_this_type_003(): void {

  HhbbcThisType003::$g = new Y;
  (new Z)->f();
  echo "OK!\n";
}
