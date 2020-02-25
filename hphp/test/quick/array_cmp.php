<?hh
<<__EntryPoint>> function main(): void {
  $x = darray['a' => 1];
  $y = darray['b' => 2];
  var_dump($x <= $y);
  var_dump($x > $y);
  echo "\n";
  var_dump($x < $y);
  var_dump($x >= $y);
  echo "\n";
  var_dump($x == $y);
  var_dump($x != $y);
  echo "\n";
  var_dump($x === $y);
  var_dump($x !== $y);
  echo "\n";
}
