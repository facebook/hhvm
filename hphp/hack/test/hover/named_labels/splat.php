<?hh

enum class SplatLabels: int {
  int A = 1;
  int B = 2;
}

function test(
  (function(...(\HH\EnumClass\Label<SplatLabels, int>,
               \HH\EnumClass\Label<SplatLabels, int>)): void) $f,
): void {
  $f(#A, #B);
// ^ hover-at-caret
}
