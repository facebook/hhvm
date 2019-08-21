<?hh

abstract final class ErrStatics {
  public static $x = 0;
}

function err() {
  if (++ErrStatics::$x == 2) throw new Exception('asd');
}

function main() {
  $x = null;

  try {
    // The second DIM (final op) will throw an exception from the
    // error handler here.  Test that the state with $x already
    // promoted to stdClass is propagated to the catch block.
    $x->foo->bar = 2;
  } catch (Exception $l) {
    var_dump($x);
  }
}



<<__EntryPoint>>
function main_minstr_throw_001() {
set_error_handler(fun('err'));

main();
}
