<?hh

function handler($name, $obj, inout $args) {
  var_dump($obj);
  return 1;
}

function bar($_this, $arg) {
  echo "In bar\n";
  echo $_this . "\n";
}

class C {
static function foo($arg) {
  echo "In foo\n";
}
}

<<__EntryPoint>>
function main() {
  fb_intercept2('C::foo', 'handler');
  C::foo(1);
}
