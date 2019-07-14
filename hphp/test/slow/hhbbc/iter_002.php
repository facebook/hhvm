<?hh

function foo($x) {
  foreach ($x as $k) yield $k;
}

<<__EntryPoint>> function main(): void {}
