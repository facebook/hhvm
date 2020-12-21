<?hh

class P {
  function foo(inout $x) {}
}

class C extends P {
  function foo($x) {}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
