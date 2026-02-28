<?hh

class MyDisposable implements IDisposable {
  public function __construct(int $i)[] {}
  public function __dispose() :mixed{}
}
async function async_int(int $x): Awaitable<int> { return $x; }
async function test() :Awaitable<mixed>{
  using (new MyDisposable(await async_int(42)));
  using (new MyDisposable(await async_int(43))) {
    echo "PASS\n";
  }
  using (new MyDisposable(await async_int(44)));
}

<<__EntryPoint>>
function main_await_as_expr_in_using() :mixed{
\HH\Asio\join(test());
}
