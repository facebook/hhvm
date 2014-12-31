<?hh

class dtor {
  public function __construct(private int $i) {}
  public function __destruct() { echo "dtor: $this->i\n"; }
}

class invoker {
  public function __invoke($x) { throw new exception; }
  public function __destruct() { echo "~invoker\n"; }
}

function foo() {
  try {
    array_map(new invoker, array(new dtor(1), new dtor(2)));
  } catch (exception $x) {
    echo "ok\n";
  }
}
foo();
echo "done\n";
