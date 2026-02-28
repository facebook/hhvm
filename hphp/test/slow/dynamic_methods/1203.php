<?hh

class A {
  public function dyn_test(inout $a) :mixed{
    $i = "gi";
    $a = $i;
    return $i;
  }
}

<<__EntryPoint>> function main(): void {
$obj = new A();
$f = 'dyn_test';
$b = null;
$c = $obj->$f(inout $b);
var_dump($b);
var_dump($c);
}
