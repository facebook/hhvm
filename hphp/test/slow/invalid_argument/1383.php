<?hh

function handler($err, $errstr) :mixed{
  $errstr = preg_replace('/given,.*$/','given', $errstr);
  var_dump($err, $errstr);
}
class Y {
  public $foo = vec[1,2,3];
}
class X {
  function __construct(<<__Soft>> Y $y) {
    var_dump($y->foo);
  }
}

<<__EntryPoint>>
function main_1383() :mixed{
set_error_handler(handler<>);
var_dump(new X(null));
}
