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
\HH\Asio\join(test());
