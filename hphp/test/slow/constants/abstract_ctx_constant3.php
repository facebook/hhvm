<?hh

abstract class A {
  abstract const ctx C = [];
}
interface I {
  const ctx C = [defaults];
}
class C extends A implements I {}

function dependent(A $a)[$a::C]: void {}

function pure()[]: void {
  dependent(new C());
}

<<__EntryPoint>>
function main(): void {
  pure();
}
