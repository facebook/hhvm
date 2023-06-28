<?hh

<<__NEVER_INLINE>> function foo(inout $x) :mixed{ echo "fail!\n"; }

function takes_varray(varray $x) :mixed{}

function handler($name, $target, inout $args) :mixed{
  var_dump(is_varray($args));
  takes_varray($args);
  $args[0] = 'handler';
  return shape('value' => null);
}

<<__EntryPoint>>
function main() :mixed{
  fb_intercept2('foo', 'handler');
  $x = 'fail'; foo(inout $x); var_dump($x);
}
