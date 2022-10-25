<?hh

function f(
  HH\FIXME\TANY_MARKER<int> $tany,
  HH\FIXME\POISON_MARKER<string> $poison,
  vec<HH\FIXME\TANY_MARKER<float>> $vec_float,
  vec<HH\FIXME\TANY_MARKER<HH\FIXME\POISON_MARKER<float>>> $vec_like_float,
): void {
  hh_show($tany);
  hh_show($poison);
  hh_show($vec_float);
  hh_show($vec_like_float);
}
