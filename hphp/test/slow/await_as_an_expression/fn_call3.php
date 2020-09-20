<?hh

async function bar1(): Awaitable<int> {
  echo "bar1()\n";
  return 1;
}

async function bar2(): Awaitable<A> {
  echo "bar2()\n";
  return new A();
}

class A {
  public function xxx(int $x): int {
    echo "xxx($x)\n";
    return 1;
  }
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  (print "b\n") + (await bar2())->xxx(await bar1()) + (print "c\n");
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
