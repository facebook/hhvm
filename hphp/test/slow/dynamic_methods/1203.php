<?hh

DynamicMethods1203::$i = 'gi';
$s = 'gs';
class A {
  public function dyn_test(inout $a) {

    $a = DynamicMethods1203::$i;
    return DynamicMethods1203::$i;
  }
}
$obj = new A();
$f = 'dyn_test';
$b = null;
$c = $obj->$f(inout $b);
var_dump($b);
var_dump($c);

abstract final class DynamicMethods1203 {
  public static $i;
}
