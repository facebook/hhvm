<?hh

class A {}

class B extends A {}

trait T {
  require extends B;
}
