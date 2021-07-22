<?hh

<<__NEVER_INLINE>>
function foo() {}

function intercept($n, $_2, inout $_3) {
  var_dump($n);
  return shape('value' => null);
}

<<__EntryPoint>>
function main() {
  fb_intercept2('foo', 'intercept');
  foo();
}
