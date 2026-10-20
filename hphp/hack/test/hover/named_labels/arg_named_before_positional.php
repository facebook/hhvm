<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

enum class NamedLabels: int {
  int A = 1;
  int B = 2;
}

function test(
  (function(\HH\EnumClass\Label<NamedLabels, int>,
            named \HH\EnumClass\Label<NamedLabels, int> $n): void) $f,
): void {
  $f(n = #B, #A);
// ^ hover-at-caret
}
