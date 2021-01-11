<?hh

function foreach_v(vec<int> $v)[$v::C]: void {
  foreach (vec[] as $v) {} // TODO(coeffects) buggy position
}

function foreach_kv_l(vec<int> $v)[$v::C]: void {
  foreach (dict[] as $v => $_) {} // TODO(coeffects) buggy position
}

function foreach_kv_r(vec<int> $v)[$v::C]: void {
  foreach (dict[] as $_ => $v) {} // TODO(coeffects) buggy position
}
