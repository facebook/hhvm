<?hh

class A {
  public $x;
  public static $count = 0;

  public function __construct() {
    ++self::$count;
    $this->x = self::$count;
  }

  public function __clone() :mixed{
    ++self::$count;
    $this->x = self::$count;
  }
}

<<__EntryPoint>>
function main(): void {
  $a = new A;
  $a->y = "foo";
  $b = clone $a;
  $a->y = "bar";
  var_dump($b);
}
