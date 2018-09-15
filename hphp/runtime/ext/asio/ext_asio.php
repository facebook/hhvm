<?hh

namespace HH {

/* A wait handle representing asynchronous operation
 */
<<__Sealed(StaticWaitHandle::class, WaitableWaitHandle::class)>>
abstract class Awaitable {

  final private function __construct() {
    throw new \InvalidOperationException(
      get_class($this) . "s cannot be constructed directly"
    );
  }

  /* Set callback for when the scheduler enters I/O wait
   * @param mixed $callback - A Closure to be called when I/O wait is entered
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnIOWaitEnterCallback(mixed $callback): void;

  /* Set callback for when the scheduler exits I/O wait
   * @param mixed $callback - A Closure to be called when I/O wait is exited
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnIOWaitExitCallback(mixed $callback): void;

  /* Set callback for when \HH\Asio\join() is called
   * @param mixed $callback - A Closure to be called on \HH\Asio\join()
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnJoinCallback(mixed $callback): void;

  /* Check if this wait handle finished (succeeded or failed)
   * @return bool - A boolean indicating whether this wait handle finished
   *
   * DEPRECATED: use HH\Asio\has_finished()
   */
  <<__Native>>
  final public function isFinished(): bool;

  /* Check if this wait handle succeeded
   * @return bool - A boolean indicating whether this wait handle succeeded
   *
   * DEPRECATED: use HH\Asio\has_succeeded()
   */
  <<__Native>>
  final public function isSucceeded(): bool;

  /* Check if this wait handle failed
   * @return bool - A boolean indicating whether this wait handle failed
   *
   * DEPRECATED: use HH\Asio\has_failed()
   */
  <<__Native>>
  final public function isFailed(): bool;

  /* Get name of the operation behind this wait handle
   * @return string - A name of the operation behind this wait handle
   *
   * TODO: replace this with a HH\Asio equivalent
   */
  <<__Native>>
  final public function getName(): string;
}

/* A wait handle that is always finished
 */
final class StaticWaitHandle extends Awaitable {}

/* A wait handle that can be waited upon
 */
<<__Sealed(
  AwaitAllWaitHandle::class,
  ConditionWaitHandle::class,
  ExternalThreadEventWaitHandle::class,
  RescheduleWaitHandle::class,
  ResumableWaitHandle::class,
  SleepWaitHandle::class
)>>
abstract class WaitableWaitHandle extends Awaitable {
}

/* A wait handle that can resume execution of PHP code
 */
<<__Sealed(AsyncFunctionWaitHandle::class, AsyncGeneratorWaitHandle::class)>>
abstract class ResumableWaitHandle extends WaitableWaitHandle {

  /* Set callback to be called when a ResumableWaitHandle is created
   * @param mixed $callback - A Closure to be called when a ResumableWaitHandle
   * is created
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnCreateCallback(mixed $callback): void;

  /* Set callback to be called when an async function blockingly awaits
   * @param mixed $callback - A Closure to be called when an async function
   * blockingly await
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnAwaitCallback(mixed $callback): void;

  /* Set callback to be called when a ResumableWaitHandle finishes successfully
   * @param mixed $callback - A Closure to be called when a ResumableWaitHandle
   * finishes
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnSuccessCallback(mixed $callback): void;

  /* Set callback to be called when a ResumableWaitHandle fails
   * @param mixed $callback - A Closure to be called when a ResumableWaitHandle
   * fails
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnFailCallback(mixed $callback): void;
}

/* A wait handle representing asynchronous execution of async function
 */
final class AsyncFunctionWaitHandle extends ResumableWaitHandle {}

/* A wait handle representing asynchronous execution of async generator
 */
final class AsyncGeneratorWaitHandle extends ResumableWaitHandle {}

/* A wait handle that waits for a list of other wait handles
 */
final class AwaitAllWaitHandle extends WaitableWaitHandle {

  /* Create a wait handle that waits for a given array of dependencies
   * @param array $dependencies - An Array of dependencies to wait for
   * @return object - A WaitHandle that will wait for a given array of
   * dependencies
   */
  <<__Native>>
  public static function fromArray(array $dependencies): Awaitable;

  /* Create a wait handle that waits for a given array of dependencies
   * @param array $dependencies - A DArray of dependencies to wait for
   * @return object - A WaitHandle that will wait for a given array of
   * dependencies
   */
  <<__Native>>
  public static function fromDArray(darray $dependencies): Awaitable;

  /* Create a wait handle that waits for a given array of dependencies
   * @param array $dependencies - A VArray of dependencies to wait for
   * @return object - A WaitHandle that will wait for a given array of
   * dependencies
   */
  <<__Native>>
  public static function fromVArray(varray $dependencies): Awaitable;

  /* Create a wait handle that waits for a given vec of dependencies
   * @param array $dependencies - A vec of dependencies to wait for
   * @return object - A WaitHandle that will wait for a given vec of
   * dependencies
   */
  <<__Native>>
  public static function fromVec(vec $dependencies): Awaitable;

  /* Create a wait handle that waits for a given dict of dependencies
   * @param array $dependencies - A dict of dependencies to wait for
   * @return object - A WaitHandle that will wait for a given dict of
   * dependencies
   */
  <<__Native>>
  public static function fromDict(dict $dependencies): Awaitable;

  /* Create a wait handle that waits for a given Map of dependencies
   * @param mixed $dependencies - A Map of dependencies to wait for
   * @return object - A WaitHandle that will wait for a given Map of
   * dependencies
   */
  <<__Native>>
  public static function fromMap(mixed $dependencies): Awaitable;

  /* Create a wait handle that waits for a given Vector of dependencies
   * @param mixed $dependencies - A Vector of dependencies to wait for
   * @return object - A WaitHandle that will wait for a given Vector of
   * dependencies
   */
  <<__Native>>
  public static function fromVector(mixed $dependencies): Awaitable;

  /* Set callback for when a AwaitAllWaitHandle is created
   * @param mixed $callback - A Closure to be called on creation
   */
  <<__HipHopSpecific, __Native>>
  public static function setOnCreateCallback(mixed $callback): void;
}

/* A wait handle representing a condition variable waiting for a notification
 */
final class ConditionWaitHandle extends WaitableWaitHandle {

  /* Create a wait handle that waits for a notification
   * @param mixed $child - A WaitHandle representing job responsible for
   * notifying the condition variable
   * @return object - A WaitHandle that will wait for a notification
   */
  <<__Native>>
  public static function create(mixed $child): \HH\ConditionWaitHandle;

  /* Set callback for when a ConditionWaitHandle is created
   * @param mixed $callback - A Closure to be called on creation
   */
  <<__HipHopSpecific, __Native>>
  public static function setOnCreateCallback(mixed $callback): void;

  /* Notify the condition variable and mark the ConditionWaitHandle as succeeded
   * @param mixed $result - A result to be set
   */
  <<__Native>>
  function succeed(mixed $result): void;

  /* Notify the condition variable and mark the ConditionWaitHandle as failed
   * @param mixed $exception - An exception to be set
   */
  <<__Native>>
  function fail(\Exception $exception): void;
}

/* A wait handle that succeeds with null once desired scheduling priority is
 * eligible for execution
 */
final class RescheduleWaitHandle extends WaitableWaitHandle {

  /* Create a wait handle that succeeds once desired scheduling priority is
   * eligible for execution
   * @param int $queue - A scheduling queue to use (defined by QUEUE_*
   * constants)
   * @param int $priority - A non-negative number indicating scheduling priority
   * (0 runs first)
   * @return object - A RescheduleWaitHandle that succeeds once desired
   * scheduling priority is eligible for execution
   */
  <<__Native>>
  public static function create(
    int $queue,
    int $priority,
  ): \HH\RescheduleWaitHandle;
}

/* A wait handle that succeeds with null after the desired timeout expires
 */
final class SleepWaitHandle extends WaitableWaitHandle {

  /* Create a wait handle that succeeds after the desired timeout expires
   * @param int $usecs - Non-negative number of microseconds to sleep for
   * @return object - A SleepWaitHandle that succeeds after the desired timeout
   * expires
   */
  <<__Native>>
  public static function create(int $usecs): \HH\SleepWaitHandle;

  /* Set callback to be called when a SleepWaitHandle is created
   * @param mixed $callback - A Closure to be called when a SleepWaitHandle is
   * created
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnCreateCallback(mixed $callback): void;

  /* Set callback to be called when a SleepWaitHandle finishes successfully
   * @param mixed $callback - A Closure to be called when a SleepWaitHandle
   * finishes
   */
  <<__HipHopSpecific, __Native>>
  final public static function setOnSuccessCallback(mixed $callback): void;
}

/* A wait handle that synchronizes against C++ operation in external thread
 */
final class ExternalThreadEventWaitHandle extends WaitableWaitHandle {

  /* Set callback to be called when an ExternalThreadEventWaitHandle is created
   * @param mixed $callback - A Closure to be called when an
   * ExternalThreadEventWaitHandle is created
   */
  <<__HipHopSpecific, __Native>>
  public static function setOnCreateCallback(mixed $callback): void;

  /* Set callback to be called when an ExternalThreadEventWaitHandle finishes
   * successfully
   * @param mixed $callback - A Closure to be called when an
   * ExternalThreadEventWaitHandle finishes
   */
  <<__HipHopSpecific, __Native>>
  public static function setOnSuccessCallback(mixed $callback): void;

  /* Set callback to be called when an ExternalThreadEventWaitHandle fails
   * @param mixed $callback - A Closure to be called when an
   * ExternalThreadEventWaitHandle fails
   */
  <<__HipHopSpecific, __Native>>
  public static function setOnFailCallback(mixed $callback): void;
}

/**
 * Get index of the current scheduler context, or 0 if there is none.
 */
<<__Native>>
function asio_get_current_context_idx(): int;

/**
 * Get currently running wait handle in a context specified by its index.
 */
<<__Native>>
function asio_get_running_in_context(int $ctx_idx): ResumableWaitHandle;

/**
 * Get currently running wait handle, or null if there is none.
 */
<<__Native>>
function asio_get_running(): ResumableWaitHandle;

} // namespace

namespace HH\Asio {

async function null<T>(): Awaitable<?T> { return null; }
async function void(): Awaitable<void> {}

/**
 * Wait for a given Awaitable to finish and return its result.
 *
 * Launches a new instance of scheduler to drive asynchronous execution
 * until the provided Awaitable is finished.
 */
<<__Native("NoFCallBuiltin")>>
function join<T>(Awaitable<T> $awaitable): mixed;

/**
 * Get result of an already finished Awaitable.
 *
 * Throws an InvalidOperationException if the Awaitable is not finished.
 */
function result<T>(Awaitable<T> $awaitable): T {
  return \hh\asm('
    CGetL $awaitable
    WHResult
  ');
}

/**
 * Check whether the given Awaitable has finished.
 */
function has_finished<T>(Awaitable<T> $awaitable): bool {
  return $awaitable->isFinished();
}

/**
 * Get the name of the Awaitable
 */
function name<T>(Awaitable<T> $awaitable): string {
  return $awaitable->getName();
}

/**
 * Cancel Awaitable, if it's still pending.
 *
 * If Awaitable has not been completed yet, fails Awaitable with
 * $exception and returns true.
 * Otherwise, returns false.
 *
 * Throws InvalidArgumentException, if Awaitable does not support cancellation.
 */
<<__Native>>
function cancel<T>(Awaitable<T> $awaitable, \Exception $exception): bool;

/**
 * Generates a backtrace for $awaitable.
 * Following conditions must be met to produce non-empty backtrace:
 * - $awaitable has not finished yet (i.e. has_finished($awaitable) === false)
 * - $awaitable is part of valid scheduler context
 *     (i.e. $awaitable->getContextIdx() > 0)
 * If either condition is not met, backtrace() returns empty array.
 *
 * @param $awaitable - Awaitable, to take backtrace from.
 * @param int $options - bitmask of the following options:
 *   DEBUG_BACKTRACE_PROVIDE_OBJECT
 *   DEBUG_BACKTRACE_PROVIDE_METADATA
 *   DEBUG_BACKTRACE_IGNORE_ARGS
 * @param int $limit - the maximum number of stack frames returned.
 *   By default (limit=0) it returns all stack frames.
 *
 * @return array - Returns an array of associative arrays.
 *   See debug_backtrace() for detailed format description.
 */
<<__Native>>
function backtrace<T>(Awaitable<T> $awaitable,
                      int $options = DEBUG_BACKTRACE_PROVIDE_OBJECT,
                      int $limit = 0): varray<darray>;


} // namespace
