<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

enum class NamedLabels: mixed {
  int A = 1;
  string B = 'b';
}

function labels(
  named \HH\EnumClass\Label<NamedLabels, int> $first,
  named \HH\EnumClass\Label<NamedLabels, string> $second,
): void {}

function test(): void {
  labels(second = #A, first = #B);
}
//
