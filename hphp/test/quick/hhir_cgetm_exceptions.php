<?hh

/*
 * This is a test that checks that CGetM in the IR properly cleans the
 * stack before operations that can throw.
 */

class Dtor {
  public function __construct() { echo "hi\n"; }
}

class Something {
  public $wat;

  public function __construct() {
    $this->wat = new Dtor();
  }
}

class Unsetter {
  private $x;

  public function __construct() {
    unset($this->x);
  }

  public function useX($k) :mixed{
    echo "sup\n";
    $z = $k->wat + $this->x;
    return $z;
  }
}

function thrower() :mixed{
  throw new Exception("wat");
}

function main() :mixed{
  set_error_handler(thrower<>);
  $k = new Unsetter;
  $k->useX(new Something());
}
<<__EntryPoint>> function main_entry() :mixed{
try {
  main();
} catch (Exception $x) {
  echo "out\n";
}
}
