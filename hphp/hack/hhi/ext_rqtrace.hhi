<?hh

namespace HH\rqtrace {

  /**
   * Enable request tracing
   */
  function force_enable(): void;

  function is_enabled(): bool;

  /**
   * @return Dict
   * (
   *    [EVENT_NAME] => Dict
   *    (
   *        [duration] => int (microseconds)
   *        [count] => int
   *    )
   *    ...
   * )
   */
  function all_request_stats(): dict<string, dict<string, int>>;
  function all_process_stats(): dict<string, dict<string, int>>;

  /**
   * @return Dict
   * (
   *    [duration] => int (microseconds)
   *    [count] => int
   * )
   */
  function request_event_stats(string $event_name): dict<string, int>;
  function process_event_stats(string $event_name): dict<string, int>;

} // namespace HH\rqtrace
