<?hh

<<__NEVER_INLINE>>
function foo() {}

function intercept($n, $_2, inout $_3, $_4, inout $_5) { var_dump($n); }

<<__EntryPoint>>
function main() {
  fb_intercept('foo', 'intercept');
  foo();
}
