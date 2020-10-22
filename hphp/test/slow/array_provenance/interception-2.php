<?hh

<<__NEVER_INLINE>>
function handler($name, $obj, inout $args) {
  return shape('prepend_this' => false, 'callback' => 'callback');
}

<<__NEVER_INLINE>>
function callback(...$args) {
  print('$args: '.HH\get_provenance($args)."\n");
  print('$args[0]: '.HH\get_provenance($args[0])."\n");
}

<<__NEVER_INLINE>>
function test($a) {
  return 17;
}

<<__EntryPoint>>
function main() {
  fb_intercept2('test', 'handler');
  test(varray[]);
  test(vec[]);
}
