<?hh
$c = (Vector {'a', 'b'})->addAll(Vector {'c', 'd'});
foreach ($c as $v) {
  echo $v . "\n";
}
