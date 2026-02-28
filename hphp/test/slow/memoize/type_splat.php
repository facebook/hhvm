<?hh


<<__Memoize>>
function splatFn<T as (mixed...)>(int $a, ...T $args) :mixed{ return $args; }

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
