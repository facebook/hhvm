<?hh

async function gen($gen) {
  if ($gen == null) {
    yield 42;
    return;
  }

  foreach ($gen await as $foo) {
    yield $foo;
  }
}

<<__EntryPoint>>
async function main() {
  $gen = gen(null);
  for ($i = 0; $i < 10000; ++$i) {
    $gen = gen($gen);
  }

  foreach ($gen await as $foo) {
    var_dump($foo);
  }
}
