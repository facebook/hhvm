<?hh

<<file: __EnableUnstableFeatures('class_type')>>

class A {}
class B extends A {}

function f(class<B> $t): class<B> {
  return $t;
}
function g(class<B> $t): class<A> {
  return $t;
}
function h(class<A> $t): class<B> {
  return $t;
}
