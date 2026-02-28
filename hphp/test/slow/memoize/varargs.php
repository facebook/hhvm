<?hh


<<__Memoize>>
function varargFn($a, ...$_) :mixed{ return 1; }

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
