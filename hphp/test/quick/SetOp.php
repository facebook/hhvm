<?hh

class C { public static $a; }

<<__EntryPoint>> function main(): void {
  $y = 20;
  var_dump($y += 10);

  $_SERVER = 20;
  var_dump($_SERVER += 10);

  C::$a = 20;
  var_dump(C::$a += 10);
}
