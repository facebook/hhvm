<?hh

function main() {
  $s = "a" . "b";
  print $s."\n";

  $s = "a" . array();
  print $s."\n";

  $s = array() . "b";
  print $s."\n";

  $s = array() . array();
  print $s."\n";

  $s = "a" . 3;
  print $s."\n";

  $s = 3 . "a";
  print $s."\n";

  $s .= 4;
  print $s."\n";

  $s .= "a";
  print $s."\n";

  $b = varray[array()];
  $a = $b[0];
  $s = $a . "a";
  print $s."\n";
}

<<__EntryPoint>> function main_entry(): void {
// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);
main();
}
