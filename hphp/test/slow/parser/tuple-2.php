<?hh
function f1($x = tuple(1)) {
  var_dump($x);
}
function f2($x = tuple (2)) {
  var_dump($x);
}
function f3() {
  static $x = tuple(3,4);
  var_dump($x);
}
class C {
  public $a = tuple(5,6);
  public static $b = tuple (7, 8);
}
f1();
f2();
f3();
var_dump((new C)->a);
var_dump(C::$b);

