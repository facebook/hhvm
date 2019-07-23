<?hh

function gen() {
  yield from null;
}
<<__EntryPoint>> function main(): void {
foreach(gen() as $val) {
  echo "Iteration!\n";
  var_dump($val);
}

echo "Done\n";
}
