<?hh
function my_implementation(B $c, arraykey $s): int;

function my_implementation_poly<T as string, T2 as arraykey>(C<T> $c, T $t, T2 $t2): int;

class B {}

class C<T as string> extends B {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(string $s): arraykey;

  <<__ImplementedBy('\my_implementation_poly')>>
  public function myMethod_poly<T2 as int>(T $t, T2 $t2): int;
}
