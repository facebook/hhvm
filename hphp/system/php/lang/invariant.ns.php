<?php

namespace __SystemLib {

class InvariantCallback {
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
 * Pass a function that will be called if any invariant fails. The callback
 * will be called with all the invariant parameters after the condition.
 */
function invariant_callback_register(callable $callback) {
  invariant(!\__SystemLib\InvariantCallback::$cb,
            'Callback already registered: %s',
            \__SystemLib\InvariantCallback::$cb);
  \__SystemLib\InvariantCallback::$cb = $callback;
}

/**
 * Ensure that an invariant is satisfied. If it fails, it calls
 * invariant_violation
 */
function invariant(mixed $test, ...): void {
  if (!$test) {
    $args = \array_slice(\func_get_args(), 1);
    \call_user_func_array('\HH\invariant_violation', $args);
  }
}

/**
 * Call this when one of your invariants has been violated. It calls the
 * function you registered with invariant_callback_register and then throws an
 * InvariantException
 */
function invariant_violation(string $format_str, ...): void {
  if (\__SystemLib\InvariantCallback::$cb) {
    $args = func_get_args();
    \call_user_func_array(\__SystemLib\InvariantCallback::$cb, $args);
  }

  $args = \array_slice(\func_get_args(), 1);
  $args = \array_map('\__SystemLib\invariant_violation_helper', $args);
  $message = \vsprintf($format_str, $args);

  throw new InvariantException($message);
}

}
