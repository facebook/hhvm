<?hh

class Parent_ {
  <<__Memoize>>
  public function foo(): string {
    return "hello";
  }

  <<__Memoize>>
  private function bar(): string {
    return "hello";
  }
}

class Child extends Parent_ {
  <<__Override, __Memoize>>
  public function foo(): int {
    return __hhvm_intrinsics\launder_value(42);
  }

  <<__Memoize>>
  public function bar(): int {
    return __hhvm_intrinsics\launder_value(42);
  }
}

<<__EntryPoint>>
function main(): void {
  $c = new Child();
  __hhvm_intrinsics\debug_var_dump_lazy_class($c->foo());
  __hhvm_intrinsics\debug_var_dump_lazy_class($c->bar());
}
