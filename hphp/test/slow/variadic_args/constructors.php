<?hh

error_reporting(-1);

class CVarSome {
  public $x;
  public $v;
  public function __construct($x, ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_num_args());
    var_dump(isset($x));
    var_dump(is_array($v));
    var_dump($v);
    $this->x = $x;
    $this->v = $v;
    var_dump($this);
  }
}

class CVarMod {
  public function __construct(public $x, public ...$v) {
    echo "\n", '* ', __METHOD__, "\n";
    var_dump(func_num_args());
    var_dump(isset($x));
    var_dump(is_array($v));
    var_dump($v);
    var_dump($this);
  }
}

function test_constructor() {
  $inst = new CVarSome('a', 'b', 'c');
  $inst = new CVarSome('a');
  $inst = new CVarSome(); // some undefined variable warnings

  $inst = new CVarMod('a', 'b', 'c');
  $inst = new CVarMod('a');
  $inst = new CVarMod(); // some undefined variable warnings
}

function test_reflection() {
  // TODO: there's ReflectionClass::newInstance
}

function main() {
  test_constructor();
  test_reflection();
}
