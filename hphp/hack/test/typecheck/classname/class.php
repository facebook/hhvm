<?hh

<<file: __EnableUnstableFeatures('class_type')>>
class A {}
class C extends A {}

function g(class<C> $c): class<A> {
  hh_show($c);
  return $c;
}
function h(class<C> $c): classname<C> {
  hh_show($c);
  return $c;
}
function j(class<C> $c): classname<A> {
  hh_show($c);
  return $c;
}
function k(class<A> $c): class<C> {
  hh_show($c);
  return $c;
}
function l(class<A> $c): classname<C> {
  hh_show($c);
  return $c;
}
function m(class<A> $c): classname<A> {
  hh_show($c);
  return $c;
}
function n(classname<C> $c): class<C> {
  hh_show($c);
  return $c;
}
function p(classname<C> $c): class<A> {
  hh_show($c);
  return $c;
}
function q(classname<A> $c): class<C> {
  hh_show($c);
  return $c;
}
function r(classname<A> $c): class<A> {
  hh_show($c);
  return $c;
}
