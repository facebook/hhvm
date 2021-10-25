<?hh
<<file: __EnableUnstableFeatures('readonly')>>

function takes_readonly_tuple(readonly (int, string) $tuple): (int, string) {
  return HH\Readonly\as_mut($tuple); // expect no error
}

class Ref<T> {}

function passes_mutable_union_to_as_mut(
  int $int,
  readonly Ref<mixed> $ref,
  bool $b,
): void {
  $union = $b ? $int : $ref;
  HH\Readonly\as_mut($union); // not ok, $ref is not primitive
}
