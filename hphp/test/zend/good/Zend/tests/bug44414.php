<?hh
abstract class A {
  abstract function foo();
}
interface B {
  function bar();
}
class C extends A implements B {
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
