<?hh

class P {
  function foo($x) :mixed{}
}

class C extends P {
  function foo(inout $x) :mixed{}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
