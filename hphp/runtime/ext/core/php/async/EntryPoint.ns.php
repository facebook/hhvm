<?hh
namespace __SystemLib {
  function enter_async_entry_point<T>(
    (function(): Awaitable<T>) $entry_point
  ): T {
    $awaitable = async {
      // Make sure we are in the new AsioContext before eagerly executing
      // the entry point.
      await RescheduleWaitHandle::create(
        RescheduleWaitHandle::QUEUE_DEFAULT,
        0
      );

      // Call the entry point and wait for its completion.
      return await $entry_point();
    };
    return \HH\Asio\join($awaitable);
  }
}
