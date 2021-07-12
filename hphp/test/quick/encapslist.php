<?hh

<<__EntryPoint>> function main(): void {
  $a = 4.5;
  $b = 3000;
  var_dump("$a");
  $a__str = (string)($a);
  var_dump("$a__str$b");
  $a__str = (string)($a);
  var_dump("$a__str $b");
}
