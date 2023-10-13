<?hh

async function genString(string $s): Awaitable<string> { return $s; }

function test(): void {
  $f0 = async () ==> {
    $str = await genString('foo');
  };
  $f1 = async $x ==> {
    $str = await genString($x);
  };
}
