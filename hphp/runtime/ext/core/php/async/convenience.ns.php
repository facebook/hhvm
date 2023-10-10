<?hh

namespace HH\Asio {

/**
 * Wrap an `Awaitable` into an `Awaitable` of `ResultOrExceptionWrapper`.
 *
 * The actual `ResultOrExceptionWrapper` in the returned `Awaitable` will only
 * be available after you `await` or `join` the returned `Awaitable`.
 *
 * @param $awaitable - The `Awaitable` to wrap.
 *
 * @return - the `Awaitable` of `ResultOrExceptionWrapper`.
 */
async function wrap<Tv>(
  Awaitable<Tv> $awaitable,
): Awaitable<ResultOrExceptionWrapper<Tv>> {
  try {
    $result = await $awaitable;
    return new WrappedResult($result);
  } catch (\Exception $e) {
    return new WrappedException($e);
  }
}

/**
 * Reschedule the work of an async function until some other time in the
 * future.
 *
 * The common use case for this is if your async function actually has to wait
 * for some blocking call, you can tell other `Awaitable`s in the async
 * scheduler that they can work while this one waits for the blocking call to
 * finish (e.g., maybe in a polling situation or something).
 *
 * @return - `Awaitable` of `void`.
 */
async function later(): Awaitable<void> {
  // reschedule to the lowest priority
  return await RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_DEFAULT,
    0,
  );
}

/**
 * Wait for a certain length of time before an async function does any more
 * work.
 *
 * This is similar to calling the PHP builtin
 * [`usleep`](http://php.net/manual/en/function.usleep.php) funciton, but is
 * in the context of async, meaning that other `Awaitable`s in the async
 * scheduler can run while the async function that called `usleep()` waits until
 * the length of time before asking to resume again.
 *
 * @param $usecs - The amount of time to wait, in microseconds.
 *
 * @return - `Awaitable` of `void`.
 */
async function usleep(
  int $usecs,
): Awaitable<void> {
  return await SleepWaitHandle::create($usecs);
}

} // namespace HH\Asio
