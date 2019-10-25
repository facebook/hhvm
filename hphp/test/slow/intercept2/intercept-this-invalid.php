<?hh

function handler($name, $obj, inout $args) {
  return shape('callback' => 'bar', 'prepend_this' => true);
}

function bar($_this, $arg) {
  $_this->baz($arg);
}

class C {
static function foo($arg) {
  echo "In foo!\n";
  echo "Arg is: " . $arg . "\n";
}
function baz($arg) {
  echo "In baz!\n";
  echo "Arg is: " . $arg . "\n";
}
}

<<__EntryPoint>>
function main() {
  fb_intercept2('C::foo', 'handler');
  C::foo("Hey!");
}
