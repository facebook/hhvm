<?hh

<<file:__EnableUnstableFeatures(
  'capture_pipe_variables',
  'allow_extended_await_syntax',
  'allow_conditional_await_syntax',
)>>

class D implements IDisposable {
  public function __construct(public int $i) {}
  public function __dispose(): void { echo "disposing D(".$this->i.")\n"; }
}

async function make_d(int $i): Awaitable<D> {
  echo "start make_d($i)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end make_d($i)\n";
  return new D($i);
}
async function ret_i(int $i): Awaitable<int> {
  echo "start ret_i($i)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end ret_i($i)\n";
  return $i;
}
async function ret_t(): Awaitable<bool> {
  echo "start ret_t()\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end ret_t()\n";
  return true;
}
async function ret_f(): Awaitable<bool> {
  echo "start ret_f()\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end ret_f()\n";
  return false;
}
async function make_fail(int $i): Awaitable<D> {
  echo "start make_fail($i)\n";
  await RescheduleWaitHandle::create(0, 0);
  echo "end make_fail($i)\n";
  throw new Exception;
}
function sync_fail(): int {
  throw new Exception;
}

<<__EntryPoint>>
async function main() {
  using (
    new D(0),
    await make_d(1),
    await make_d(await ret_i(2)),
    await ret_t() ? await make_d(await ret_i(3) + await ret_i(0)) : D(100),
    await ret_f() ? D(200) : await make_d(await ret_i(await ret_i(4) + await ret_i(1)) + await ret_i(-1))
  ) {}

  try {
    using (
      new D(0),
      await make_d(1),
      await make_d(await ret_i(2)),
      await ret_t() ? await make_fail(await ret_i(3) + await ret_i(0)) : D(100),
      await ret_f() ? D(200) : await make_d(await ret_i(await ret_i(4) + await ret_i(1)) + await ret_i(-1))
    ) {}
  } catch (Exception $e) {
    echo "Caught exception\n";
  }

  try {
    using (
      new D(0),
      await make_d(1),
      await make_d(await ret_i(2)),
      await ret_t() ? await make_d(await ret_i(3) + sync_fail()) : D(100),
      await ret_f() ? D(200) : await make_d(await ret_i(await ret_i(4) + await ret_i(1)) + await ret_i(-1))
    ) {}
  } catch (Exception $e) {
    echo "Caught exception\n";
  }
}
