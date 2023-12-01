<?hh

function ok_lit(): vec<int> {
  return vec[0];
}

function ok_lit2(): varray<int> {
  return vec[0];
}

function ok_func(Traversable<int> $t): vec<int> {
  return varray($t);
}

function ok_hint(varray<int> $v): vec<int> {
  return $v;
}

function ok_hint2(vec<int> $v): varray<int> {
  return $v;
}

function err_string(varray<int> $v): bool {
  return vec[0];
}
