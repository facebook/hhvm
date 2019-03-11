<?php

// Check MixedArray::MakeReserveLike on NVT (GitHub #3065)

$x = 0;
ExtArrayArrayMapGlobals::$x = 0;

function check_global($arg) {

  ExtArrayArrayMapGlobals::$x = 1;
  return false;
}

array_map('check_global', $GLOBALS);
print ExtArrayArrayMapGlobals::$x."\n";

abstract final class ExtArrayArrayMapGlobals {
  public static $x;
}
