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
  public static ?int $x; // no error for nullable uninitialized prop
  public static TnullableInt $y;
  public static Tany $z;
  public static dynamic $d;
  public static ~int $li; // error here
}
