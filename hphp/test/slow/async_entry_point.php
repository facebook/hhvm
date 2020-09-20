<?hh
<<__EntryPoint>>
async function test() {
  await HH\Asio\later();
  var_dump("hello world!");
}
