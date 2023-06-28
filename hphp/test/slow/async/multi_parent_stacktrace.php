<?hh

class MyClass {
    public static $awaitable;
}

async function foo(): Awaitable<void> {
  await cache();
}

async function bar(): Awaitable<void> {
  await SleepWaitHandle::create(100000);
  await cache();
}

async function baz(): Awaitable<void> {
  await SleepWaitHandle::create(300000);
  await cache();
}

<<__Memoize>>
async function cache(): Awaitable<void> {
  debug_print_backtrace(); print "\n";
  await SleepWaitHandle::create(200000);
  debug_print_backtrace(); print "\n";
  await SleepWaitHandle::create(200000);
  debug_print_backtrace(); print "\n";
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  concurrent {
    await foo();
    await bar();
    await baz();
  }
}
