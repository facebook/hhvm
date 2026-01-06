<?hh

class A {
  public function f() {
    if (__hhvm_intrinsics\launder_value(true)) {
      $maybe_uninit = __hhvm_intrinsics\launder_value(123);
    }
    $dummy1 = (int)__hhvm_intrinsics\launder_value(0);
    $dummy2 = (int)__hhvm_intrinsics\launder_value(0);
    return $x ==> { return $x + $maybe_uninit + $dummy1 + $dummy2; };
  }
}

<<__EntryPoint>>
function main() {
  $a = new A();
  var_dump($a->f()(__hhvm_intrinsics\launder_value(100)));
}
