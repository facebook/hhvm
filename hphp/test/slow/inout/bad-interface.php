<?hh

interface I {
  function foo(inout $x):mixed;
}

class C implements I {
  function foo($x) :mixed{}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
