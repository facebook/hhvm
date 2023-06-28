<?hh

class P {
  function foo(inout $x) :mixed{}
}

class C extends P {
  function foo($x) :mixed{}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
