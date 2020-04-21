<?hh

type Generic<T> = T;

/* HH_FIXME[4101] */
type Tany = Generic;

type TnullableInt = ?int;

/* Prove that null <: Tany */
function f(): Tany {
  return null;
}

class C {
  public ?int $x; // no error for nullable uninitialized prop
  public TnullableInt $y;
  public Tany $z;
  public dynamic $d;
  public ~int $li; // error here
}
