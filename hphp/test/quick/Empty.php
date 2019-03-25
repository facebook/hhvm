<?hh

abstract final class quickEmpty {
  public static $y;
}

<<__EntryPoint>>
function f() {

  $x = 0;
  $GLOBALS['y'] = 0;
  quickEmpty::$y = 0;
  print ":".empty($x).":\n";
  print ":".empty($GLOBALS['y']).":\n";
  print ":".empty(quickEmpty::$y).":\n";

  $x = 1;
  $GLOBALS['y'] = 1;
  quickEmpty::$y = 1;
  print ":".empty($x).":\n";
  print ":".empty($GLOBALS['y']).":\n";
  print ":".empty(quickEmpty::$y).":\n";
}
