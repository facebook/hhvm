<?hh

<<__NEVER_INLINE>> function foo(inout $x) { echo "fail!\n"; }

function takes_varray(varray $x) {}

function handler($name, $target, inout $args, $ctx, inout $done) {
  $done = true;
  var_dump(is_varray($args));
  takes_varray($args);
  $args[0] = 'handler';
}

<<__EntryPoint>>
function main() {
  fb_intercept('foo', 'handler');
  $x = 'fail'; foo(inout $x); var_dump($x);
}
