<?hh

function handler($name, $obj, inout $args) :mixed{
  var_dump($obj);
  return shape('prepend_this' => true, 'callback' => bar<>);
}

function bar($_this, $arg) :mixed{
  echo "In bar\n";
  echo $_this . "\n";
}

class C {
static function foo($arg) :mixed{
  echo "In foo\n";
}
}

<<__EntryPoint>>
function main() :mixed{
  fb_intercept2('C::foo', handler<>);
  C::foo(1);
}
