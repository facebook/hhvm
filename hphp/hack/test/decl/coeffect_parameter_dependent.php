<?hh

abstract class A {
  abstract const ctx C;
}

function f(A $a)[$a::C]: void {}

function f_option(?A $a)[$a::C]: void {}

function f_like(~A $a)[$a::C]: void {}

function f_like_option(~?A $a)[$a::C]: void {}
