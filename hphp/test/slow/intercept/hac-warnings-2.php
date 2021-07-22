<?hh

<<__NEVER_INLINE>> function foo(inout $x) { echo "fail!\n"; }

function takes_varray(varray $x) {}

function handler($name, $target, inout $args) {
  var_dump(is_varray($args));
  takes_varray($args);
  $args[0] = 'handler';
  return shape('value' => null);
}

<<__EntryPoint>>
function main() {
  fb_intercept2('foo', 'handler');
  $x = 'fail'; foo(inout $x); var_dump($x);
}
