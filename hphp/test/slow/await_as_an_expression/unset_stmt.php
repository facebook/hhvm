<?hh

async function genString(): Awaitable<string> {
  return 'ree';
}

<<__EntryPoint>>
async function test(): Awaitable<void> {
  $x = dict[1 => 'two', 'three' => 4];
  unset($x["th" . await genString()]);
  var_dump($x);
}
