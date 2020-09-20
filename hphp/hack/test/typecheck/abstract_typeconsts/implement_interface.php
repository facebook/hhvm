<?hh

interface I {
  abstract const type T;
}
abstract class A {
  abstract const type T = string;
}

final class C extends A implements I {}

function f(): void {
  $a = type_structure(C::class, 'T');
}
