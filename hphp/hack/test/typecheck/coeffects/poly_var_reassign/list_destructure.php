<?hh

function destruct(vec<int> $v)[$v::C]: void {
  list($v, $_) = $v;
}
