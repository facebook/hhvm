<?hh

interface B {
  function foo();
}

class C implements B {
  function foo() { return 1; }
}

class D implements B {}

<<__EntryPoint>>
function main(): void {
  echo "Done\n";
}
