<?hh

class MyDisposable implements IDisposable {
  public function __construct(int $i) {}
  public function __dispose() {}
}
async function async_int(): Awaitable<int> { return 1; }
async function test() {
  using (new MyDisposable(await async_int())) {
    echo "FAIL\n";
  }
}

<<__EntryPoint>>
function main_await_as_expr_in_using() {
\HH\Asio\join(test());
}
