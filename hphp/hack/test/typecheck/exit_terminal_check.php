<?hh // partial

function main() {
  try {
    $foo = 'bar';
  } catch (Exception $e) {
    exit();
  }
  var_dump($foo);
}

main();

function main2(): int {
  if (true) {
    $x = 1;
  } else {
    exit(1);
  }
  return $x;
}

main2();

function main3($a): int {
  if ($a) {
    return 1;
  } else {
    exit(1);
  }
}
