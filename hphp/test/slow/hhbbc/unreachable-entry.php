<?hh

class A {
  public function foo(T $t) {
    try {
      $this->bar($t);
    } catch (Exception $e) {
      __hhvm_intrinsics\launder_value(dict['a' => $t]);
    }
  }

  public function bar($x) { __hhvm_intrinsics\launder_value($x); }
}

<<__EntryPoint>>
function main() {
  $a = new A();
  $a->foo(__hhvm_intrinsics\launder_value(true));
}
