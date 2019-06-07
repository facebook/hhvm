<?hh

class X extends Y {
  private $priv = 42;
  function foo() {
    var_dump($this->priv + self::C);
  }
}

$x = 1;
