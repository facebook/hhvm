<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function g2() :mixed{
  var_dump(\HH\global_isset('gl'));
  \HH\global_unset('gl'); // unsets global "version"
  var_dump(\HH\global_isset('gl'));
}

class C {
  public dict<arraykey, int> $prop = dict['a' => 10];
  public static dict<arraykey, int> $sprop = dict['a' => -5];
}

<<__EntryPoint>>
function entrypoint_intrinsics_unset(): void {
  error_reporting(-1);

  echo "---------- unsetting inside a function (\$GLOBALS) ------------\n";

  \HH\global_set('gl', 100);
  var_dump(\HH\global_isset('gl')); // still set

  g2();
  var_dump(\HH\global_isset('gl')); // no longer set

  echo "---------- unsetting array elements ------------\n";

  $a = dict[0 => 10, 1 => 20, "xx" => 30];
  print_r($a);
  unset($a[1]);
  print_r($a);

  unset($a[10]);
  print_r($a);

  unset($a["Xx"]);
  print_r($a);

  echo "---------- unsetting elements through a property ------------\n";

  $c = new C;
  unset($c->prop['a']);
  var_dump($c->prop);

  unset(C::$sprop['a']);
  var_dump(C::$sprop);
}
