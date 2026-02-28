<?hh

function foo() :AsyncGenerator<mixed,mixed,void>{
  $i = 0;
  foreach (range("a","e") as $letter) {
    yield $letter => ++$i;
  }
}
<<__EntryPoint>> function main(): void {
foreach (foo() as $k => $v) {
  var_dump($k);
  var_dump($v);
}
}
