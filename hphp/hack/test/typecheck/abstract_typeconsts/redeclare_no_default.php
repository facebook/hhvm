<?hh

abstract class A {
  abstract const type T as mixed = arraykey;
}

abstract class B extends A {
  abstract const type T as string;
}

final class C extends B {}

function f(): void {
  $c = type_structure(C::class, 'T');
}
