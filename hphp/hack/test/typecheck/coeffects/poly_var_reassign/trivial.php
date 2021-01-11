<?hh

function trivial(vec<int> $v)[$v::C]: void {
  $v = 4;
}

function pluseq(vec<int> $v)[$v::C]: void {
  // kind of a non-sensical case, but demonstrates that += and friends
  // change expression ids
  $v += 3;
}
