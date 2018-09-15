<?hh

async function f() { return 42; }
async function g() { throw new Exception(__FUNCTION__); }
async function h() { return 37; }
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
async function test4() {
  return await genva(f(), h(), f());
}

function main() {
  var_dump(function_exists('genva'));

  var_dump(\HH\Asio\join(test0()));
  var_dump(\HH\Asio\join(test1()));
  var_dump(\HH\Asio\join(test2()));
  try {
    var_dump(\HH\Asio\join(test3()));
  } catch (Exception $e) { print $e->getMessage()."\n"; }
  var_dump(\HH\Asio\join(test4()));
}

<<__EntryPoint>>
function main_genva() {
main();
}
