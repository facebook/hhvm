<?hh

abstract class A {
  abstract const ctx C;
}
class C {
  const type T = A;
  public function f()[this::T::C]: void {}
}

<<__EntryPoint>>
function main(): void {
  (new C())->f();
}
