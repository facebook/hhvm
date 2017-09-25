<?hh

class dtor {
  public function __construct(private int $i) {}
  public function __destruct() { echo "dtor: $this->i\n"; }
}

class invoker {
  public function __invoke($x) { return 2; }
  public function __destruct() { echo "~invoker\n"; }
}

function foo() {
  array_map(new invoker, array(new dtor(1), new dtor(2)));
}
foo();
