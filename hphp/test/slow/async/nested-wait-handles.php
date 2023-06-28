<?hh

async function foo(): Awaitable<bool> { return __hhvm_intrinsics\launder_value(false); }

async function bar(): Awaitable<bool> {
  $a = await async { return foo(); };
  $a = await $a;
  return $a;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(HH\Asio\join(bar()));
  var_dump(HH\Asio\join(bar()));
  var_dump(HH\Asio\join(bar()));
  var_dump(HH\Asio\join(bar()));
  var_dump(HH\Asio\join(bar()));
}
