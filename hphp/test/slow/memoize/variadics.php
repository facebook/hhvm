<?hh


<<__Memoize>>
function variadicFn($a, ...$args) :mixed{ return $args; }

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
