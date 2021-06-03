<?hh

class A {
  public function __construct(public int $p) {
    $this->init(); // can call
  }
  private function init()[write_this_props] {
    $this->p = 42;
  }
}

<<__EntryPoint>>
function main() {
  echo (new A(2))->p;
}
