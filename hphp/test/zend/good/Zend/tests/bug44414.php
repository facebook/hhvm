<?hh
abstract class A {
  abstract function foo():mixed;
}
interface B {
  function bar():mixed;
}
class C extends A implements B {
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
