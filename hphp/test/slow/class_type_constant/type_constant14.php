<?hh

interface I {
  const type T = int;
}

class C implements I {
  const type T = string;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
