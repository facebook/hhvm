<?hh

async function ret1() :Awaitable<mixed>{ return 1; }
async function await1() :Awaitable<mixed>{
  $b = await ret1();
  return 1 + $b;
}


<<__EntryPoint>>
function main_simple_func() :mixed{
var_dump(HH\Asio\join(ret1()));
var_dump(HH\Asio\join(await1()));
}
