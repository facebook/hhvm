<?php

namespace __SystemLib {

abstract final class InvariantCallback {
  public static $cb = null;
}

/**
 * Sometimes people pass objects without a __toString() defined as an arg,
 * which causes a fatal. Handle them gracefully by displaying the class
 * name.
 */
function invariant_violation_helper($arg) {
  if (!\is_object($arg) || \method_exists($arg, '__toString')) {
    return $arg;
  }
  return 'Object of type '.\get_class($arg);
}

}

namespace HH {

class InvariantException extends \Exception {}

/**
 * Pass a function that will be called if any `invariant` fails. The callback
 * will be called with all the invariant parameters after the condition.
 *
 * @param $callback - The function that will be called if your invariant fails.
 */
function invariant_callback_register(callable $callback) {
  invariant(!\__SystemLib\InvariantCallback::$cb,
            'Callback already registered: %s',
            \__SystemLib\InvariantCallback::$cb);
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
 * @param $args - Arguments supplied if the condition is not met. This is
 *                usually a string, with possible placeholders.
 */
function invariant(mixed $test, ...$args): void {
  if (!$test) {
    \HH\invariant_violation(...$args);
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
function invariant_violation(string $format_str, ...$args): void {
  if ($cb = \__SystemLib\InvariantCallback::$cb) {
    $cb($format_str, ...$args);
  }

  $args = \array_map('\__SystemLib\invariant_violation_helper', $args);
  $message = \vsprintf($format_str, $args);

  throw new InvariantException($message);
}

}
