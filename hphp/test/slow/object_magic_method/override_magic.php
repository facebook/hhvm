<?hh

class Y {
  private $x = null;
  function __construct() { unset($this->x); }
  function z() { var_dump($this->x); }
}
class X extends Y {
  public function __get($x) { echo "get: $x\n"; return "heh"; }
}

class Y1 {
  private $x = null;
  function __construct() { unset($this->x); }
  function z() { var_dump($this->x); } // no magic getters
}
class X1 extends Y1 {}


<<__EntryPoint>>
function main_override_magic() {
(new X)->z();
(new X1)->z();
}
