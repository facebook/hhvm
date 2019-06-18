<?hh

function bar(): void {
  echo "bar()\n";
}

async function baz(int $value): Awaitable<int> {
  echo "baz()\n";
  return $value + 1;
}

async function bak(): Awaitable<int> {
  echo "bak()\n";
  return 1;
}

async function async_main(): Awaitable<void> {
  echo "async_main()\n";
  bar();
  echo "--- before x1\n";
  $x1 = (print "line 1\n") + 2 * await baz(5) + 3 * await bak();
  echo "--- before y1\n";
  $y1 = (print "line 2\n") + 3 * await baz(10) + 2 * await bak();
  echo "--- before concurrent\n";
  concurrent {
    $x2 = (print "line 1\n") + 2 * await baz(5) + 3 * await bak();
    $y2 = (print "line 2\n") + 3 * await baz(10) + 2 * await bak();
  };
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
