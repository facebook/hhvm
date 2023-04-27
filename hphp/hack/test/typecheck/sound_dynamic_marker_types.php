<?hh

function f(
  HH\FIXME\TANY_MARKER<int> $tany,
  HH\FIXME\POISON_MARKER<string> $poison,
  vec<HH\FIXME\TANY_MARKER<float>> $vec_tany,
  vec<HH\FIXME\TANY_MARKER<HH\FIXME\POISON_MARKER<float>>> $vec_tany2,
): void {
  hh_show($tany);
  hh_show($poison);
  hh_show($vec_tany);
  hh_show($vec_tany2);
}
