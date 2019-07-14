<?hh

// Check MixedArray::MakeReserveLike on NVT (GitHub #3065)

function check_global($arg) {
  ExtArrayArrayMapGlobals::$x = 1;
  return false;
}

abstract final class ExtArrayArrayMapGlobals {
  public static $x;
}

$x = 0;
ExtArrayArrayMapGlobals::$x = 0;

array_map('check_global', $GLOBALS);
print ExtArrayArrayMapGlobals::$x."\n";
