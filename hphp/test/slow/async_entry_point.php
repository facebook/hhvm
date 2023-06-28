<?hh
<<__EntryPoint>>
async function test() :Awaitable<mixed>{
  await HH\Asio\later();
  var_dump("hello world!");
}
