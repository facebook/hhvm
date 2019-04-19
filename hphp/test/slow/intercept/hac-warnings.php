<?hh

<<__NEVER_INLINE>>
function foo() {}

function intercept($n) { var_dump($n); }

<<__EntryPoint>>
function main() {
  fb_intercept('foo', 'intercept');
  foo();
}
