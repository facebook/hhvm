<?hh

async function bar(): Awaitable<int> {
  return 3;
}

function foo(): dynamic {
  return bar();
}

async function testit((function(): dynamic) $fd, dynamic $d): Awaitable<void> {
  $x = foo();
  $y = await foo();
  $z = await $fd();
  $gd = () ==> $d;
  $w = await $gd();
}
