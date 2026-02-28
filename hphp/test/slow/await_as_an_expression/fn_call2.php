<?hh

async function bar2(): Awaitable<A> {
  echo "bar2()\n";
  return new A();
}

class A {
  public async function xxx(): Awaitable<int> {
    echo "xxx()\n";
    return 1;
  }
}

async function async_main(): Awaitable<void> {
  echo "--- a\n";
  (print "b\n") + await (await bar2())->xxx() + (print "c\n");
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
