<?hh

async function f() { return 42; }
async function g() { throw new Exception(__FUNCTION__); }
async function h() { return 37; }
final class MyAwaitable implements Awaitable<int> {
  public function getWaitHandle() { return f(); }
}
async function test0(): Awaitable<void> {
  await genva(f());
}
async function test1(): Awaitable<void> {
  await genva();
}
async function test2(): Awaitable<(int, int)> {
  return await genva(f(), f());
}
async function test3(): Awaitable<void> {
  await genva(f(), g());
}
async function test4(): Awaitable<void> {
  await genva(new MyAwaitable);
}
async function test5() {
  return await genva(f(), h(), f());
}

function main() {
  var_dump(function_exists('genva'));

  var_dump(test0()->join());
  var_dump(test1()->join());
  var_dump(test2()->join());
  try {
    var_dump(test3()->join());
  } catch (Exception $e) { print $e->getMessage()."\n"; }
  var_dump(test4()->join());
  var_dump(test5()->join());
}
main();
