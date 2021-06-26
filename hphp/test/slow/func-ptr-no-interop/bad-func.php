<?hh

class A {
  <<__DynamicallyCallable>> static public function f1($a) { return $a; }
}

<<__EntryPoint>>
function main() {
  $a = "A::f1";
  var_dump($a(1));

  $a = HH\class_meth(A::class, "f1");
  var_dump($a);
  var_dump($a(3));
  var_dump($a[1](4));
}
