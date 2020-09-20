<?hh

function handler($name, $obj, inout $args) {
  return shape('prepend_this' => true,
               'callback' =>
    ($_this, $arg) ==> { var_dump("In callback!", $_this, $arg); });
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
