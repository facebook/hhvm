<?hh // partial
class C {

  private static $barK = 0;

  private function bar() {
    ++self::$barK;
    echo self::$barK . "\n";
    var_dump($this);
  }
  public function foo1() {
    return () ==> () ==> $this->bar();
  }
  public function foo2() {
    $x = $this;
    return () ==> () ==> $x->bar();
  }
}
function main() {
  $x = $y ==> $z ==> $y + $z;
  var_dump(($x(3))(4));
  $x = (int $y = 5): int ==> $y;
  var_dump($x());
  var_dump($x(8));
  $x = (int $y = 7): (function(int): int) ==> (int $z): int ==> $y + $z;
  var_dump(($x(5))(9));
  $c = new C();
  $x = (($c->foo1())())();
  $x = (($c->foo2())())();
  $x = (
    string $x,
    (function(int):bool) $f,
    Vector<Vector<int>> $v = Vector {Vector {1, 2}, Vector {3, 4}},
  ): Vector<Vector<int>> ==> $v;
  var_dump($x is Closure);
}
main();
