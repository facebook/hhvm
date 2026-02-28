<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type CT = string | (int, string);

function foo<T as (num, CT)>(T $pair): void {
  if ($pair is (num, (int, string))) {
    hh_expect<(num, (int, string))>($pair);
    hh_expect<(num, CT)>($pair);
  } else {
    hh_expect<(num, string)>($pair);
    hh_expect<(num, CT)>($pair);
  }
}
