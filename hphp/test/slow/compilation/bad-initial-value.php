<?hh

class B {}

function returns_true() {
  return true;
}

class A {
  private B $p = null;

  public function set(B $p) {
    if (returns_true()) $p = 100;
    $this->p = $p;
  }

  public function get(): B {
    return $this->p;
  }
}

<<__EntryPoint>>
function main() {
  $a = new A();
  var_dump($a->get());
}
