<?hh

interface I {
  const T = 0;
}

abstract class P {
  const type T = int;
}

class C extends P implements I {}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
