<?hh

function handler($_1, $_2, inout $_3) {
  return shape('prepend_this' => true, 'callback' => varray['C', 'bar']);
}

class C {
static function bar($_this, $arg) {
  echo "In bar\n";
  echo $_this . "\n";
}
static function foo($arg) {
  echo "In foo\n";
}
}

<<__EntryPoint>>
function main() {
  fb_intercept2('C::foo', 'handler');
  C::foo(1);
}
