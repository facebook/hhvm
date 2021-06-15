<?hh

abstract class V { abstract const ctx C; }

function i(inout V $v)[]: void {}

function f(inout V $v)[$v::C]: void {
  i(inout $v);
}
