<?hh

case type CheckGeneric<T, Tni as ?int> =
  | bool where T super bool
  | Tni where T super Tni;

function check<T, Tni as ?int>(CheckGeneric<T, Tni> $value): T {
  if ($value is bool) {
    return $value;
  } else {
    return $value;
  }
}
