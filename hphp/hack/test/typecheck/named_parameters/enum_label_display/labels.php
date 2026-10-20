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

function test(
  (function(named \HH\EnumClass\Label<NamedLabels, int>...): void) $collector,
  (function(...(\HH\EnumClass\Label<NamedLabels, int>,
               \HH\EnumClass\Label<NamedLabels, int>)): void) $splat,
): void {
  labels(second = #B, first = #A);
  labels(first = #A, second = #B);
  $a = #A;
  labels(second = #B, first = $a);
  $collector(first = #A, second = #B);
  $splat(#A, #B);
}
