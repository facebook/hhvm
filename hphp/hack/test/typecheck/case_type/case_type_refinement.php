<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type VecLike<T> = vec<T> | dict<int, T> | KeyedIterable<int, T>;

case type ShapeOrKey<T as arraykey> = shape('key' => T, ...) | T;

function get_key<T as arraykey>(ShapeOrKey<T> $t): T {
  if ($t is arraykey) {
    return $t;
  } else {
    return $t['key'];
  }
}

function impl1(VecLike<string> $v): ?string {
  if ($v is KeyedIterable<_, _>) {
    hh_expect<KeyedIterable<int, string>>($v);
    return $v->firstValue();
  } else if ($v is vec<_>) {
    hh_expect<vec<string>>($v);
    return $v[0];
  } else {
    hh_expect<dict<int, string>>($v);
    foreach($v as $value) {
      return $value;
    }
    return null;
  }
}

function impl2(VecLike<string> $v): ?string {
  if ($v is vec<_>) {
    hh_expect<vec<string>>($v);
    return $v[0];
  } else if ($v is dict<_, _>){
    hh_expect<dict<int, string>>($v);
    foreach($v as $value) {
      return $value;
    }
    return null;
  } else {
    hh_expect<KeyedIterable<int, string>>($v);
    return $v->firstValue();
  }
}

function impl3(VecLike<string> $v): ?string {
  if ($v is dict<_, _>){
    hh_expect<dict<int, string>>($v);
    foreach($v as $value) {
      return $value;
    }
    return null;
  } else if ($v is KeyedIterable<_, _>) {
    hh_expect<KeyedIterable<int, string>>($v);
    return $v->firstValue();
  } else {
    hh_expect<vec<string>>($v);
    return $v[0];
  }
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
