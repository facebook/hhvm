<?hh

interface I {
  function foo(inout $x);
}

class C implements I {
  function foo($x) {}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
