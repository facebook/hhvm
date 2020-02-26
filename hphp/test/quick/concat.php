<?hh

function main() {
  $s = "a" . "b";
  print $s."\n";

  $s = "a" . varray[];
  print $s."\n";

  $s = varray[] . "b";
  print $s."\n";

  $s = varray[] . varray[];
  print $s."\n";

  $s = "a" . 3;
  print $s."\n";

  $s = 3 . "a";
  print $s."\n";

  $s .= 4;
  print $s."\n";

  $s .= "a";
  print $s."\n";

  $b = varray[varray[]];
  $a = $b[0];
  $s = $a . "a";
  print $s."\n";
}

<<__EntryPoint>> function main_entry(): void {
// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);
main();
}
