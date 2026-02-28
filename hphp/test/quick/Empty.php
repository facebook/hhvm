<?hh

abstract final class quickEmpty {
  public static $y;
}

<<__EntryPoint>>
function f() :mixed{

  $x = 0;
  \HH\global_set('y', 0);
  quickEmpty::$y = 0;
  print ":".(string)(!($x ?? false)).":\n";
  print ":".(string)(!(\HH\global_get('y') ?? false)).":\n";
  print ":".(string)(!(quickEmpty::$y ?? false)).":\n";

  $x = 1;
  \HH\global_set('y', 1);
  quickEmpty::$y = 1;
  print ":".(string)(!($x ?? false)).":\n";
  print ":".(string)(!(\HH\global_get('y') ?? false)).":\n";
  print ":".(string)(!(quickEmpty::$y ?? false)).":\n";
}
