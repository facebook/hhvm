<?hh

abstract final class quickEmpty {
  public static $y;
}

<<__EntryPoint>>
function f() {

  $x = 0;
  $GLOBALS['y'] = 0;
  quickEmpty::$y = 0;
  print ":".!($x ?? false).":\n";
  print ":".!($GLOBALS['y'] ?? false).":\n";
  print ":".!(quickEmpty::$y ?? false).":\n";

  $x = 1;
  $GLOBALS['y'] = 1;
  quickEmpty::$y = 1;
  print ":".!($x ?? false).":\n";
  print ":".!($GLOBALS['y'] ?? false).":\n";
  print ":".!(quickEmpty::$y ?? false).":\n";
}
