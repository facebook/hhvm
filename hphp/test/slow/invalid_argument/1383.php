<?hh

function handler($err, $errstr) {
  $errstr = preg_replace('/given,.*$/','given', $errstr);
  var_dump($err, $errstr);
}
class y {
  public $foo = varray[1,2,3];
}
class x {
  function __construct(@y $y) {
    var_dump($y->foo);
  }
}

<<__EntryPoint>>
function main_1383() {
set_error_handler(fun('handler'));
var_dump(new X(null));
}
