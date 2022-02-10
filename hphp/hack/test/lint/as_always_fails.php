<?hh

class A {}
abstract class B {}
class C extends B {}

function test(A $a): void {
  $a as C;
}
