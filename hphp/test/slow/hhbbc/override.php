<?hh

trait Base {
  public function get(): bool {
    return false;
  }
}

abstract class A {
  use Base;
};
abstract class B extends A {}

final class C extends B {
  public function get(): bool {
    return true;
  }
}

final class D extends B {}

function get_object(): A {
  return __hhvm_intrinsics\launder_value(new C());
}

<<__EntryPoint>> function main() {
  $obj = get_object();
  var_dump($obj->get());
}
