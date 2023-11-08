<?hh


function f<reify T>(): void {
  try {
    var_dump(nameof T);
  } catch (OutOfBoundsException $e) {
    // TypeStructure["classname"]
    echo "Not a classname\n";
  }
}

<<__EntryPoint>>
function main(): void {
  f<C>();
  f<int>();
}
