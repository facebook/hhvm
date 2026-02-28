<?hh

class A {
  public function foo(T $t) :mixed{
    try {
      $this->bar($t);
    } catch (Exception $e) {
      __hhvm_intrinsics\launder_value(dict['a' => $t]);
    }
  }

  public function bar($x) :mixed{ __hhvm_intrinsics\launder_value($x); }
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A();
  $a->foo(__hhvm_intrinsics\launder_value(true));
}
