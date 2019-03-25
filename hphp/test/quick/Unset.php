<?hh

final class quickUnset {
  public static $y;

  public function foo() {
    unset($this);
  }
}

<<__EntryPoint>>
function f() {


  print ":".isset($x).":\n";
  print ":".isset($GLOBALS['y']).":\n";
  print ":".isset(quickUnset::$y).":\n";

  unset($x);
  unset($GLOBALS['y']);
  quickUnset::$y = null;
  print ":".isset($x).":\n";
  print ":".isset($GLOBALS['y']).":\n";
  print ":".isset(quickUnset::$y).":\n";

  $x = 0;
  $GLOBALS['y'] = 0;
  quickUnset::$y = 0;
  print ":".isset($x).":\n";
  print ":".isset($GLOBALS['y']).":\n";
  print ":".isset(quickUnset::$y).":\n";

  unset($x);
  unset($GLOBALS['y']);
  quickUnset::$y = null;
  print ":".isset($x).":\n";
  print ":".isset($y).":\n";
  print ":".isset(quickUnset::$y).":\n";

  $obj = new quickUnset;
  $obj->foo();
}
