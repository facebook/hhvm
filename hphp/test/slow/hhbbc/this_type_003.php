<?hh

class X {
  function getP() {
    global $g;
    return $g;
  }
  function getY() : Y {
    $n = $this;
    while (!($n instanceof Y)) $n = $this->getP();
    return $n;
  }
}

class Y extends X {}

class Z extends X {
  function f() {
    return $this->getY();
  }
}

$g = new Y;
(new Z)->f();
echo "OK!\n";
