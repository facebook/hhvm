<?hh

function bar() { return count(func_get_args()); }
function baz($_1, $_2, inout $_3, $_4, inout $_5) {}

function test($f, $x) {
  call_user_func_array($f, array($x));
  baz(null, null, inout $f, null, inout $x);
}

<<__EntryPoint>> function main(): void {
  $x = array(1);
  fb_intercept('bar', 'baz', 'fiz');
  for ($i = 0; $i < 10000; $i++) {
    test('bar', $x);
  }
  var_dump('done');
}
