<?hh

interface B {
  function foo():mixed;
}

class C implements B {
  function foo() :mixed{ return 1; }
}

class D implements B {}

<<__EntryPoint>>
function main(): void {
  echo "Done\n";
}
