<?hh

function f(...$args) :mixed{}
function g($required, ...$args) :mixed{}
function h($optional = null, ...$args) :mixed{}

<<__EntryPoint>> function test() :mixed{
  f();
  g(1);
  h();

  f(1,2,3,4);
  g(1,2,3,4);
  h(1,2,3,4);
}
