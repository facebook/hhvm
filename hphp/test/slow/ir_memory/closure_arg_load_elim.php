<?hh

class Foo {
  public static $x;
}

<<__NEVER_INLINE>>
function foo(): mixed {
  Foo::$x = 42;
  var_dump(89);
}


<<__EntryPoint>>
function main(): mixed {
  $x = __hhvm_intrinsics\launder_value(42);
  $a = () ==> $x;

  foo();

  $b = $a();
  $e = $a();
  var_dump($b, $e);
}
