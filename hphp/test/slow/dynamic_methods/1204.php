<?hh

class A {
  public static function dyn_test(inout $a) {
    $s = "gs";
    $a = $s;
    return $s;
  }
}

<<__EntryPoint>> function main(): void {
$f = 'dyn_test';
$d = null;
$e = A::$f(inout $d);
var_dump($d);
var_dump($e);
}
