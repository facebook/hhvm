<?hh

<<file: __EnableUnstableFeatures('open_tuples', 'type_splat')>>

function proj1<T as (arraykey...)>((int, string, ...T) $tup):void {
  hh_expect_equivalent<int>($tup[0]);
  hh_expect_equivalent<string>($tup[1]);
  // This should be an array because the element may not exist
  $y = $tup[2];
  hh_expect_equivalent<?arraykey>($tup[2] ?? null);
}

function proj2<T as (float, bool, arraykey...)>((int, string, ...T) $tup):void {
  hh_expect_equivalent<int>($tup[0]);
  hh_expect_equivalent<string>($tup[1]);
  hh_expect_equivalent<float>($tup[2]);
  hh_expect_equivalent<bool>($tup[3]);
  // This should be an array because the element may not exist
  $y = $tup[4];
  hh_expect_equivalent<?arraykey>($tup[4] ?? null);
}
