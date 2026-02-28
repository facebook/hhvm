<?hh

class B {
  function foo() :mixed{}
}

abstract class C extends B {
  abstract function foo():mixed;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
