<?hh

final class quickUnset {
  public static $y;
}

<<__EntryPoint>>
function f() :mixed{


  print ":".(string)(isset($x)).":\n";
  print ":".(string)(\HH\global_isset('y')).":\n";
  print ":".(string)(isset(quickUnset::$y)).":\n";

  unset($x);
  \HH\global_unset('y');
  quickUnset::$y = null;
  print ":".(string)(isset($x)).":\n";
  print ":".(string)(\HH\global_isset('y')).":\n";
  print ":".(string)(isset(quickUnset::$y)).":\n";

  $x = 0;
  \HH\global_set('y', 0);
  quickUnset::$y = 0;
  print ":".(string)(isset($x)).":\n";
  print ":".(string)(\HH\global_isset('y')).":\n";
  print ":".(string)(isset(quickUnset::$y)).":\n";

  unset($x);
  \HH\global_unset('y');
  quickUnset::$y = null;
  print ":".(string)(isset($x)).":\n";
  print ":".(string)(isset($y)).":\n";
  print ":".(string)(isset(quickUnset::$y)).":\n";
}
