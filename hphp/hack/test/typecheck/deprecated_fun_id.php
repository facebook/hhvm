<?hh

<<__Deprecated('use bar() instead')>>
function foo() {}

function f() {
  call_user_func(fun('foo'));
}
