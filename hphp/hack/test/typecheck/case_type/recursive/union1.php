<?hh
<<file:__EnableUnstableFeatures('case_types', 'recursive_case_types')>>

case type Lst<T> = (T, Lst<T>) | null;

function empty_list<T>(): Lst<T> {
  return null;
}

function singleton<T>(T $x): (T, Lst<T>) {
  return tuple($x, empty_list());
}

function list_with_2_elements<T>(T $x, T $y): (T, (T, Lst<T>)) {
  return tuple($x, singleton($y));
}

function list_with_1_or_2_elements<T>(
  bool $b,
  T $x,
  T $y,
): void {
  if ($b) {
    $l = singleton($x);
  } else {
    $l = list_with_2_elements($x, $y);
  }
  hh_expect<~(T, Lst<T>)>($l);
}
