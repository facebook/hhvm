<?hh

function ok_lit(): dict<int, bool> {
  return dict[0 => true];
}

function ok_lit2(): darray<int, bool> {
  return dict[0 => true];
}

function ok_func(KeyedTraversable<int, bool> $kt): dict<int, bool> {
  return darray($kt);
}

function ok_hint(darray<int, bool> $v): dict<int, bool> {
  return $v;
}

function ok_hint2(dict<int, bool> $v): darray<int, bool> {
  return $v;
}

function err_string(darray<int, bool> $v): bool {
  return dict[0 => true];
}
