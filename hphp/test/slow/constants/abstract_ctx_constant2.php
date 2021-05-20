<?hh

abstract class A {
  abstract const ctx C = [];
}

class C extends A {}

function dependent(C $c)[$c::C]: void {
  echo "No coeffect violation\n";
}

function pure()[]: void {
  dependent(new C());
}

<<__EntryPoint>>
function main(): void {
  pure();
}
