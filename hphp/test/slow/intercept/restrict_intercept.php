<?hh

function foo($i = 10) :mixed{
  var_dump(__METHOD__);
}

function foo1($i = 10) :mixed{
  var_dump(__METHOD__);
}

function bar($_1, $_2, inout $_3) :mixed{
  var_dump(__METHOD__);
  return shape('value' => null);
}


<<__EntryPoint>>
function main(): void {
  foo();
  fb_intercept2('foo', 'bar');
  foo();
  fb_intercept2('foo1', 'bar');
}
