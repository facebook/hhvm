<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Ref<T> {
  public function __construct(public T $value) {}
  public function get(): T {
    return $this->value;
  }
  public function set(T $value): void {
    $this->value = $value;
  }
}

class C {}

/* HH_FIXME[4110] The line to which this HH_FIXME applies doesn't have
 * any type errors.  However, type checking of $r->set($c3) gives rise
 * to a subtyping constraint
 *
 * <expr#3> as C <: ?(<expr#1> as C | <expr#2> as C),
 *
 * which simplifies to the following disjunction
 *
 * C <: <expr#1> as C \/ C <: <expr#2> as C \/
 * C <: <expr#1> as C \/ C <: <expr#2> as C.
 *
 * All constraints here are unsatisfiable with the type errors having
 * the line to which the HH_FIXME applies as the primary position.
 * Allowing the HH_FIXME to suppress those errors would result in the
 * disjunction being accepted.  We don't want this kind of action
 * at a distance.
 */
function test2(bool $b1, bool $b2, C $c1, C $c2, C $c3): void {
  $r = new Ref(null);
  if ($b1) {
    $r->set($c1);
  }
  if ($b2) {
    $r->set($c2);
  }
  if ($r->get() !== null) {
    $r->set($c3);
  }
}
