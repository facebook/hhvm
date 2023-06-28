<?hh

class B {}

function returns_true() :mixed{
  return true;
}

class A {
  private B $p = null;

  public function set(B $p) :mixed{
    if (returns_true()) $p = 100;
    $this->p = $p;
  }

  public function get(): B {
    return $this->p;
  }
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A();
  var_dump($a->get());
}
