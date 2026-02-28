<?hh
<<file:__EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type Lst<T> = (null, Lst<T>) | null;

function discriminate(
  Lst<int> $l,
): void {
  if ($l is (null, Lst<null>)) {}
}
