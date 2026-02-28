<?hh

// Lazy infinite list!
function fibonacci($first, $second) :AsyncGenerator<mixed,mixed,void>{
  $a = $first; $b = $second;
  while (true) {
    yield $b;
    $temp = $b;
    $b = $a + $b;
    $a = $temp;
  }
}
<<__EntryPoint>> function main(): void {
foreach (fibonacci(0, 1) as $k) {
  if ($k > 1000) break;
  echo $k . "\n";
}
}
