<?hh

function takes_int(int $_): void {}

async function test_await(dynamic $d): Awaitable<void> {
  $r = await $d;
  takes_int($r);
}
