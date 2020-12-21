<?hh

abstract class Base {
  abstract const type T;
}

class Child extends Base {
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
