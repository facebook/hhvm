<?hh // strict

namespace HH\Rx {

/*
 * A noop implementation of what will become a builtin that freezes mutable
 * variable pointing to an object.
 */
<<__Rx>>
function freeze<T>(T $obj): T {
  invariant(\is_object($obj), 'HH\\Rx\\freeze() operates only on objects');
  return $obj;
}

/*
 * A noop implementation of what will become a builtin that allows a mutable
 * result of an expression to be assigned to a mutable variable.
 */
<<__Rx>>
function mutable<T>(T $obj): T {
  invariant(\is_object($obj), 'HH\\Rx\\mutable() operates only on objects');
  return $obj;
}

/*
 * A noop implementation of what will become a builtin that allows a mutable
 * object that is owned by the current scope to be passed to another function
 * together with the ownership
 */
<<__Rx, __MutableReturn>>
function move<T>(<<__OwnedMutable>> T $obj): T {
  invariant(\is_object($obj), 'HH\\Rx\\move() operates only on objects');
  return $obj;
}

} // namespace
