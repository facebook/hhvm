<?hh

function handler($_1, $_2, inout $_3) :mixed{
  return shape('prepend_this' => true, 'callback' => vec['C', 'bar']);
}

class C {
static function bar($_this, $arg) :mixed{
  echo "In bar\n";
  echo $_this . "\n";
}
static function foo($arg) :mixed{
  echo "In foo\n";
}
}

<<__EntryPoint>>
function main() :mixed{
  fb_intercept2('C::foo', handler<>);
  C::foo(1);
}
