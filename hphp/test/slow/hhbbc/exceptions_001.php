<?hh

abstract final class HehStatics {
  public static $x = 0;
}

function heh() {
  $x = HehStatics::$x;
  echo "ok $x\n";
  if (HehStatics::$x++ == 0) {
    throw new Exception('a');
  }
}

function foo() {
  try {
    $x++;
  } catch (Exception $y) {
    // $x: Uninit here.  This is a divergence from php5 (where it will
    // be int(1)).
    var_dump($x);
  }
}


<<__EntryPoint>>
function main_exceptions_001() {
error_reporting(-1);
set_error_handler(fun('heh'));

foo();
}
