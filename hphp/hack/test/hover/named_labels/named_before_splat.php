<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

enum class NamedLabels: int {
  int A = 1;
  int B = 2;
}

function test(
  (function(named \HH\EnumClass\Label<NamedLabels, int> $n,
            ...(\HH\EnumClass\Label<NamedLabels, int>,
                \HH\EnumClass\Label<NamedLabels, int>)): void) $f,
): void {
  $f(#A, #A, n = #B);
// ^ hover-at-caret
}
//
