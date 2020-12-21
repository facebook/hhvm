<?hh

class B {
  function foo() {}
}

abstract class C extends B {
  abstract function foo();
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
