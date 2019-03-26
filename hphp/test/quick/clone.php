<?hh

class A {
  public $x;
  static $count = 0;

  public function __construct() {
    $this->x = ++self::$count;
  }

  public function __clone() {
    $this->x = ++self::$count;
  }
};

class C {}
function box(&$what) {}

function main() {
  $a = new A;
  $a->y = "foo";
  $b = clone $a;
  $a->y = "bar";
  var_dump($b);


  $d = new C();
  $d->thing = 10;
  box(&$d->thing);  // now the property is the only reference

  var_dump($d);
  $e = clone $d;
  var_dump($d);  // the reference doesn't persist across the clone
}
main();

