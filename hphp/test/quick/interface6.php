<?hh
abstract class B {
  abstract public function foo();
}
abstract class C extends B{}
abstract class D extends C{}
class E extends D {}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
