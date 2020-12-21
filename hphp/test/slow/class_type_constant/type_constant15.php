<?hh

class P {
  const type T = int;
}

class C extends P {
  const T = 'int';
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
