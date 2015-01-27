<?hh

function err() {} set_error_handler('err');

class dtor {
  public function __construct(private int $i) {}
  public function __destruct() { echo "dtor: $this->i\n"; }
}

function foo() {
  try {
    hash(new dtor(1), new dtor(2));
  } catch (exception $x) {
    echo "ok\n";
  }
}
foo();
echo "done\n";
