<?hh

class P {
  function foo($x) {}
}

class C extends P {
  function foo(inout $x) {}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
