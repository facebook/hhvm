<?hh

class A {
  <<__DynamicallyCallable>> static public function f1($a) :mixed{ return $a; }
}

<<__EntryPoint>>
function main() :mixed{
  $a = "A::f1";
  var_dump($a(1));

  $a = A::f1<>;
  var_dump($a);
  var_dump($a(3));
  var_dump($a[1](4));
}
