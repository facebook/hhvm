<?hh

abstract class V { abstract const ctx C; }

function trivial(V $v)[$v::C]: void {
  $v = 4;
}

function pluseq(V $v)[$v::C]: void {
  // kind of a non-sensical case, but demonstrates that += and friends
  // change expression ids
  $v += 3;
}
