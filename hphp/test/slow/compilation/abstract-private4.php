<?hh

abstract class C1 {
  private function f1() { return 1; }
}
abstract class C2 extends C1 {
}
abstract class C3 extends C2 {
}
abstract class C4 extends C2 {
  private function f1() { return 2; }
}

<<__EntryPoint>>
function main() {
  echo "Done\n";
}
