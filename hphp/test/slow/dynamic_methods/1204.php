<?hh

$i = 'gi';
DynamicMethods1204::$s = 'gs';
class A {
  public static function dyn_test(inout $a) {

    $a = DynamicMethods1204::$s;
    return DynamicMethods1204::$s;
  }
}
$f = 'dyn_test';
$d = null;
$e = A::$f(inout $d);
var_dump($d);
var_dump($e);

abstract final class DynamicMethods1204 {
  public static $s;
}
