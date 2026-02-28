<?hh

<<__ALWAYS_INLINE>>
async function foo($b, $n) :Awaitable<mixed>{
  if ($b) {
    await RescheduleWaitHandle::create(0, 0);
  }
  return $n * 42;
}

<<__ALWAYS_INLINE>>
async function bar($a) :Awaitable<mixed>{
  $n1 = await $a;
  $n2 = await foo(false, 100);
  return $n1 + $n2;
}

async function main() :Awaitable<mixed>{
  $x = foo(false, 1);
  $y = foo(true, 1);
  $b = bar($x);
  $r1 = await bar($y);
  $r2 = await $b;
  var_dump($r1, $r2);
}
<<__EntryPoint>> function main_entry(): void {
for ($i = 0; $i < 10; $i++) HH\Asio\join(main());
}
