<?hh
trait C { final function method1() { yield 1; } }
class A { use C; }
class B extends A { use C; }

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
