<?hh

async function f1() : Awaitable<int> {
  return 2;
}

async function f2() : Awaitable<int> {
  return 3;
}

async function foo() : Awaitable<void> {
  $v = vec[4];
  foreach ($v as $elem) {
    if ($elem == 2) return;
  }

  $x = -1;
  $y = -1;
  concurrent {
    $x = await f1();
    $y = await f2();
  }

  var_dump($x);
  var_dump($y);
}

<<__EntryPoint>>
function main() : void {
  \HH\Asio\join(foo());
}
