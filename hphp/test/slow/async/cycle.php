<?hh

async function bar($a) {
  return $a;
}

async function foo() {
  for ($i = 0; $i < 42; $i++) {
    $b = await bar($i);
    var_dump($b);
  }
  return "finished!";
}

var_dump(foo()->join());

