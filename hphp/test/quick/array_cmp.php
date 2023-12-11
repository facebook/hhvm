<?hh
<<__EntryPoint>> function main(): void {
  $x = dict['a' => 1];
  $y = dict['b' => 2];
  var_dump($x == $y);
  var_dump($x != $y);
  echo "\n";
  var_dump($x === $y);
  var_dump($x !== $y);
  echo "\n";
}
