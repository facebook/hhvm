<?hh

abstract final class HehStatics {
  public static $x = 0;
}

function heh() :mixed{
  $x = HehStatics::$x;
  echo "ok $x\n";
  if (HehStatics::$x++ == 0) {
    throw new Exception('a');
  }
}

function foo() :mixed{
  try {
    $x++;
  } catch (Exception $y) {
    // $x: Uninit here.  This is a divergence from php5 (where it will
    // be int(1)).
    try {
      var_dump($x);
    } catch (UndefinedVariableException $e) {
      var_dump($e->getMessage());
    }
  }
}


<<__EntryPoint>>
function main_exceptions_001() :mixed{
  error_reporting(-1);
  set_error_handler(heh<>);

  foo();
}
