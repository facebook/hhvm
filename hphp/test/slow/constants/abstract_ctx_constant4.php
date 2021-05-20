<?hh

abstract class A {
  abstract const ctx C = [defaults];
}
interface I {
  const ctx C = [];
}
class C extends A implements I {}

function dependent(A $a)[$a::C]: void {
  echo "No coeffect violation\n";
}

function pure()[]: void {
  dependent(new C());
}

<<__EntryPoint>>
function main(): void {
  pure();
}
