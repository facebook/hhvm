<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type Recursive = Recursive | int;

function f(Recursive $rec): void {
  hh_expect<Recursive>($rec);
  hh_expect<Recursive>(1);
  hh_expect<Recursive>(true);
}
