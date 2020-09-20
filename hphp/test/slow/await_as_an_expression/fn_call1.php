<?hh

class A {
  public async function xxx(): Awaitable<int> {
    echo "xxx()\n";
    return 1;
  }
}

async function async_main(): Awaitable<void> {
  $a = new A();
  echo "--- a\n";
  (print "b\n") + await $a->xxx() + (print "c\n");
  echo "--- d\n";
}

<<__EntryPoint>>
function main(): void {
  \HH\Asio\join(async_main());
}
