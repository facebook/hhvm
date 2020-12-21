<?hh

interface abc {
}

class A {
    use abc;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
