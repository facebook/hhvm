<?hh

abstract class A {
  abstract const type T as this;

  private function f<Terase>(): void {
    HH\ReifiedGenerics\get_class_from_type<this::T>();

    // error
    HH\ReifiedGenerics\get_class_from_type<Terase>();
  }
}
final class C extends A {
  const type T = this;
}
