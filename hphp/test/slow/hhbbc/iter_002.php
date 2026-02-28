<?hh

function foo($x) :AsyncGenerator<mixed,mixed,void>{
  foreach ($x as $k) yield $k;
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
