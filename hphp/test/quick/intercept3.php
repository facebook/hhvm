<?hh

function bar() :mixed{ return count(func_get_args()); }
function baz($_1, $_2, inout $_3) :mixed{ return shape('value' => null); }

function test($f, $x) :mixed{
  call_user_func_array($f, vec[$x]);
  baz(null, null, inout $f);
}

<<__EntryPoint>> function main(): void {
  $x = vec[1];
  fb_intercept2('bar', 'baz');
  for ($i = 0; $i < 10000; $i++) {
    test('bar', $x);
  }
  var_dump('done');
}
