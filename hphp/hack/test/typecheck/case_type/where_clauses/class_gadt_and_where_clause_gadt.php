<?hh

interface ISomething<T> {}

interface IntSomething extends ISomething<int> {}

case type Combo<T, Tw> =
  | ISomething<T> where Tw super bool
  | string;

function test_combo<T, Tw>(Combo<T, Tw> $x): (T, Tw) {
  if ($x is IntSomething) {
    // we should infer: IntSomething <: ISomething<T> && bool <: Tw
    return tuple(0, true);
  } else {
    throw new Exception();
  }
}
