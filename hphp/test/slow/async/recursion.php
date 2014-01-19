<?hh

async function fibonacci($a) {
  if ($a <= 1) return 1;
  $b = await fibonacci($a-1);
  $c = await fibonacci($a-2);
  return $b + $c;
}

var_dump(fibonacci(12)->join());
