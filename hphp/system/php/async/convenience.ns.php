<?hh // strict

namespace HH\Asio {

/**
 * Wrap an Awaitable into a ResultOrExceptionWrapper.
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
 * Wait until some later time in the future.
 */
async function later(): Awaitable<void> {
  // reschedule to the lowest priority
  return await RescheduleWaitHandle::create(
    RescheduleWaitHandle::QUEUE_DEFAULT,
    0,
  );
}

/**
 * Convenience wrapper for SleepWaitHandle
 */
async function usleep(
  int $usecs,
): Awaitable<void> {
  return await SleepWaitHandle::create($usecs);
}

} // namespace HH\Asio
