<?hh


<<__Memoize>>
function variadicFn($a, ...$args) { return $args; }

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
