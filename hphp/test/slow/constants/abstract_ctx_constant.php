<?hh

abstract class A {
  abstract const ctx C = [defaults];
}

class C extends A {}

function dependent(C $c)[$c::C]: void {}

function pure()[]: void {
  dependent(new C());
}

<<__EntryPoint>>
function main(): void {
  pure();
}
