<?hh

function main() :mixed{
  $s = "a" . "b";
  print $s."\n";

  $s = "a" . 3;
  print $s."\n";

  $s = 3 . "a";
  print $s."\n";

  $s .= 4;
  print $s."\n";

  $s .= "a";
  print $s."\n";
}

<<__EntryPoint>> function main_entry(): void {
main();
}
