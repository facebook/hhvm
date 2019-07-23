<?hh

function gen($g) {
  yield from $g;
}

<<__EntryPoint>> function main(): void {
foreach(gen(8) as $val) {
  var_dump($val);
}
}
