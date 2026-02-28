<?hh

class C { public static $a; }

<<__EntryPoint>> function main(): void {
  $y = 20;
  var_dump($y += 10);

  \HH\global_set('_SERVER', 20);
  \HH\global_set('_SERVER', \HH\global_get('_SERVER') + 10);
  var_dump(\HH\global_get('_SERVER'));

  C::$a = 20;
  var_dump(C::$a += 10);
}
