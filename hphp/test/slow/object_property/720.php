<?hh

function error_handler($errno, $errstr) {
 echo "Error\n";
 }
class f {
  public $bar = 'hi there';
}
class c {
  private $foo;
  function __construct($c) {
    if ($c) {
      $this->foo = new f;
    }
  }
  function get() {
    return $this->foo->bar;
  }
}
function main() {
  set_error_handler(fun('error_handler'));
  $c = new c(false);
  $c->get();
  echo "Error\n";
}

<<__EntryPoint>>
function main_720() {
main();
}
