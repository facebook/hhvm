<?hh

abstract class A {
  abstract const type T as this;

  private function f(): void {
    HH\ReifiedGenerics\get_class_from_type<this::T>();
  }
}
final class C extends A {
  const type T = this;
}
