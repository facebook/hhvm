<?hh

function f(
  HH\FIXME\TANY_MARKER<arraykey> $_tany,
  HH\FIXME\POISON_MARKER<arraykey> $_poison,
): void {}

function g(
  HH\FIXME\TANY_MARKER<int> $tany,
  HH\FIXME\POISON_MARKER<string> $poison,
): void {
  f($tany, $poison);
}
