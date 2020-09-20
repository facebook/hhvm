<?hh
<<__EntryPoint>>
function main_entry(): void {

  $s = str_repeat("\0", 200);
  var_dump($d = quoted_printable_encode($s));
  var_dump(quoted_printable_decode($d));

  $s = str_repeat("строка в юникоде", 50);
  var_dump($d = quoted_printable_encode($s));
  var_dump(quoted_printable_decode($d));

  echo "Done\n";
}
