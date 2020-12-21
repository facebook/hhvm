<?hh

class ThisShouldFatal implements Iterator, IteratorAggregate {}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
