<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

enum class NamedLabels: int {
  int A = 1;
  int B = 2;
}

function test(): void {
  $f = (named \HH\EnumClass\Label<NamedLabels, int> $first,
        named \HH\EnumClass\Label<NamedLabels, int> $second) ==> {};
  $a = #A;
  $f(second = #B, first = $a);
// ^ hover-at-caret
}
