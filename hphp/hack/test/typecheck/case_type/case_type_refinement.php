<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type VecLike<T> = vec<T> | dict<int, T> | KeyedIterable<int, T>;

function impl(VecLike<string> $v): ?string {
  if ($v is KeyedIterable<_, _>) {
    return $v->firstValue();
  }

  if ($v is vec<_>) {
    return $v[0];
  }

  if ($v is dict<_, _>) {
    foreach($v as $value) {
      return $value;
    }
    return null;
  }

  return null;
}

function generic_impl<T as VecLike<string>>(T $v): ?string {
  if ($v is KeyedIterable<_, _>) {
    return $v->firstValue();
  }

  if ($v is vec<_>) {
    return $v[0];
  }

  if ($v is dict<_, _>) {
    foreach($v as $value) {
      return $value;
    }
    return null;
  }

  return null;
}
