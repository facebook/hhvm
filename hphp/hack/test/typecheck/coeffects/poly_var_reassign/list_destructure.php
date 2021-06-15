<?hh

abstract class V { abstract const ctx C; }

function destruct(V $v)[$v::C]: void {
  list($v, $_) = vec[1,2];
}
