<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

enum class NamedLabels: int {
  int A = 1;
  int B = 2;
}

function labels(
  named \HH\EnumClass\Label<NamedLabels, int> $first,
  named \HH\EnumClass\Label<NamedLabels, int> $second,
): void {}

function test(): void {
  labels(second = #B, first = #A);
  //                           ^ at-caret
}
