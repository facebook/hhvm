<?hh

final class quickUnset {
  public static $y;
}

<<__EntryPoint>>
function f() {


  print ":".isset($x).":\n";
  print ":".\HH\global_isset('y').":\n";
  print ":".isset(quickUnset::$y).":\n";

  unset($x);
  \HH\global_unset('y');
  quickUnset::$y = null;
  print ":".isset($x).":\n";
  print ":".\HH\global_isset('y').":\n";
  print ":".isset(quickUnset::$y).":\n";

  $x = 0;
  \HH\global_set('y', 0);
  quickUnset::$y = 0;
  print ":".isset($x).":\n";
  print ":".\HH\global_isset('y').":\n";
  print ":".isset(quickUnset::$y).":\n";

  unset($x);
  \HH\global_unset('y');
  quickUnset::$y = null;
  print ":".isset($x).":\n";
  print ":".isset($y).":\n";
  print ":".isset(quickUnset::$y).":\n";
}
