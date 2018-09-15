<?hh // decl

function err() { throw new Exception; }

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

<<__EntryPoint>>
function main_exception_dtor_order2() {
set_error_handler('err');
foo();
echo "done\n";
}
