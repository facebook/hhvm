<?hh

function main(): void {
  try {
    $foo = 'bar';
  } catch (Exception $e) {
    exit();
  }
  var_dump($foo);
}

function main2(): int {
  if (true) {
    $x = 1;
  } else {
    exit(1);
  }
  return $x;
}

function main3(bool $a): int {
  if ($a) {
    return 1;
  } else {
    exit(1);
  }
}
