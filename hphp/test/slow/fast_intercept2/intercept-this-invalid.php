<?hh

function handler($name, $obj, inout $args) :mixed{
  return shape('callback' => bar<>, 'prepend_this' => true);
}

function bar($_this, $arg) :mixed{
  $_this->baz($arg);
}

class C {
static function foo($arg) :mixed{
  echo "In foo!\n";
  echo "Arg is: " . $arg . "\n";
}
function baz($arg) :mixed{
  echo "In baz!\n";
  echo "Arg is: " . $arg . "\n";
}
}

<<__EntryPoint>>
function main() :mixed{
  fb_intercept2('C::foo', handler<>);
  C::foo("Hey!");
}
