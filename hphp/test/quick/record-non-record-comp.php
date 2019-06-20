<?hh

record A {
  x: int,
}

$a = A['x' => 10];
try {
  $b = $a > 1;
} catch (Exception $e) {
  echo $e->getMessage(), "\n";
}
