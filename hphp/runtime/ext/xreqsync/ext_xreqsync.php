<?hh

namespace HH {
  <<__NativeData>>
  final class XReqSync {
    // Should be obtain via XReqSync::get()
    private function __construct()[] {}

    /*
     * Create a new instance of XReqSync wrapping around an internal
     * singleton corresponding to the provided name.
     *
     * @param string $name - The lock name
     * @return XReqSync - an instance of XReqSync for the internal lock
     */
    <<__Native>>
    public static function get(string $name)[]: XReqSync;

    /*
     * Tries to obtain the named lock. If the thread already holds the lock,
     * the awaitable is resolved immediately.
     *
      @param int $wait_timeout_ms - Time to wait, -1 for no timeout
     */
    <<__Native>>
    public function genLock(int $wait_timeout_ms = -1)[]: Awaitable<bool>;

    /*
     * Tries to release the named lock. If the thread doesn't holds the lock,
     * the method does nothing.
     */
    <<__Native>>
    public function unlock()[]: void;
  }
}
