<?hh

namespace __SystemLib {

abstract final class InvariantCallback {
  public static ?(function(string, mixed...)[]: void) $cb = null;
}

/**
 * Sometimes people pass objects without a __toString() defined as an arg,
 * which causes a fatal. Handle them gracefully by displaying the class
 * name.
 */
function invariant_violation_helper(mixed $arg)[]: mixed {
  if (!\is_object($arg) || \method_exists($arg, '__toString')) {
    return $arg;
  }
  return 'Object of type '.
    HH\FIXME\UNSAFE_CAST<mixed, string>(\get_class($arg));
}

}

namespace HH {

class InvariantException extends \Exception {
  use \ExceptionWithPureGetMessageTrait;
}

/**
 * Pass a function that will be called if any `invariant` fails. The callback
 * will be called with all the invariant parameters after the condition.
 *
 * @param $callback - The function that will be called if your invariant fails.
 */
function invariant_callback_register(
  (function(string, mixed...)[]: void) $callback,
)[globals]: void {
  invariant(
    \is_callable($callback),
    'Callback not callable: %s',
    $callback,
  );
  invariant(
    \__SystemLib\InvariantCallback::$cb is null,
    'Callback already registered: %s',
    \__SystemLib\InvariantCallback::$cb,
  );
  \__SystemLib\InvariantCallback::$cb = $callback;
}

/**
 * Ensure that an invariant is satisfied. If it fails, it calls
 * `invariant_violation`
 *
 * This function provides a way to have a variable type checked as a more
 * specific type than it is currently declared. A source transformation in the
 * runtime modifies code that looks like:
 *
 * ```
 * invariant(<condition>, 'sprintf format: %s %d', 'string', ...);
 * ```
 * ... is transformed to be:
 *
 * ```
 *   if (!(<condition>)) { // an Exception is thrown
 *     invariant_violation('sprintf format: %s', 'string', ...);
 *   }
 *   // <condition> is known to be true in the code below
 * ```
 *
 * See also:
 *   - [`invariant` guide](/hack/types/refining#invariant)
 *
 * @param $test - The condition you are declaring as an expression of fact.
 * @param $format_str - The string that will be displayed when your
 *                      invariant fails, with possible placeholders.
 * @param $args - Actual values to placeholders in your format string.
 */
function invariant(
  mixed $test,
  mixed $format_str,
  mixed... $args
)[]: void {
  if (!$test) {
    \HH\invariant_violation(
      HH\FIXME\UNSAFE_CAST<mixed, string>($format_str),
      ...$args,
    );
  }
}

/**
 * Call this when one of your `invariant`s has been violated. It calls the
 * function you registered with `invariant_callback_register` and then throws
 * an `InvariantException`
 *
 * @param $format_str - The string that will be displayed when your invariant
 *                      fails.
 * @param $args - Actual values to placeholders in your format string.
 */
<<__IgnoreCoeffectLocalErrors>>
function invariant_violation(string $format_str, mixed... $args)[]: noreturn {
  // TODO(T94673071): The callback only ever gets set once at the beginning of
  // the request for FB WWW. This means that this global read is technically
  // pure within the request.
  $cb = \__SystemLib\InvariantCallback::$cb;
  if ($cb is nonnull) {
    $cb($format_str, ...$args);
  }

  foreach ($args as $i => $arg) {
    if (\is_object($arg)) {
      $args[$i] = \__SystemLib\invariant_violation_helper($arg);
    }
  }
  $message = \vsprintf($format_str, $args);

  throw new InvariantException($message);
}

}
