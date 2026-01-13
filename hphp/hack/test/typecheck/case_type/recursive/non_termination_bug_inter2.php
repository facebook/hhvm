<?hh
<<file:__EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type Lst<T> = Cons<T> | null;
type Cons<T> = (null, Lst<T>);

function discriminate(
  Lst<int> $l,
): void {
  if ($l is Cons<null>) {}
}
