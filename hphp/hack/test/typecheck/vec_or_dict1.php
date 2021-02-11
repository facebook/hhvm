<?hh

function tk(dict<arraykey, int> $v): vec_or_dict<string, int> {
  return $v;
}

function tv(dict<arraykey, int> $v): vec_or_dict<string> {
  return $v;
}
