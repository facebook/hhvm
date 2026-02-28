<?hh
function my_implementation(B $c, ~arraykey $s): int;

function my_implementation2(B $c, arraykey $s, arraykey $i): int;

function my_implementation_poly<T as string, T2 as arraykey>(C<T> $c, T $t, T2 $t2): int;

function my_implementation_this<T as string>(C<T> $c, T $t, C<T> $t2): int;

function my_implementation_ret_C<T as string>(C<T> $c, ~T $t): C<T>;

function my_implementation_ret_C2<T as string>(C<T> $c, T $t, C<T> $t2): C<T>;

class B {}

final class C<T as string> extends B {
  <<__ImplementedBy('\my_implementation')>>
  public function myMethod(string $s): arraykey;

  <<__ImplementedBy('\my_implementation2')>>
  public function myMethod2(string $s, int $i): arraykey;

  <<__ImplementedBy('\my_implementation_poly')>>
  public function myMethod_poly<T2 as int>(T $t, T2 $t2): int;

  <<__ImplementedBy('\my_implementation_this')>>
  public function myMethod_this(T $t, this $t2): int;

  <<__ImplementedBy('\my_implementation_ret_C2')>>
  public function myMethod_ret_C2(T $t, this $t2): C<T>;

  <<__ImplementedBy('\my_implementation_ret_C')>>
  public function myMethod_ret_C(T $t): C<T>;
}
