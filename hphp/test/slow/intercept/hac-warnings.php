<?hh

<<__NEVER_INLINE>>
function foo() :mixed{}

function intercept($n, $_2, inout $_3) :mixed{
  var_dump($n);
  return shape('value' => null);
}

<<__EntryPoint>>
function main() :mixed{
  fb_intercept2('foo', 'intercept');
  foo();
}
