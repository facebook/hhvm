<?hh

function bar() { return count(func_get_args()); }
function baz($_1, $_2, inout $_3) { return shape('value' => null); }

function test($f, $x) {
  call_user_func_array($f, varray[$x]);
  baz(null, null, inout $f);
}

<<__EntryPoint>> function main(): void {
  $x = varray[1];
  fb_intercept2('bar', 'baz');
  for ($i = 0; $i < 10000; $i++) {
    test('bar', $x);
  }
  var_dump('done');
}
