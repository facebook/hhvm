<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class A {
  abstract const type T;
}

function in_param_parens((A with {}) $a): void {}
function in_param(A with {} $a): void {}

function in_return(): A with {} {}
function in_return_parens(): (A with {}) {}

function in_fun_hint((function(A with {}): A with {}) $_): void {}

abstract class B extends A {
  public abstract function in_meth_type_param<Ta as A with {}>(Ta $_): void;
  public abstract function in_meth_param(A with {} $a): A with {};
}
