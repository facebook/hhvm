<?hh

function i(inout vec<int> $v)[]: void {}

function f(inout vec<int> $v)[$v::C]: void {
  i(inout $v);
}
