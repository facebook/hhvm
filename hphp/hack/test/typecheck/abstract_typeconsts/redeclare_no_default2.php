<?hh

abstract class A {
  abstract const type T as mixed = arraykey;
}

abstract class B extends A {
  abstract const type T as string;
}

abstract class C extends B {}

final class D extends C {}
