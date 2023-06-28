<?hh

function bare_break_continue() :mixed{
  // This should emit properly; it's not an include-time fatal
  break;
  continue;
}

<<__EntryPoint>> function test(): void {
  $three = varray[1, 2, 3];
  $four  = varray[1, 2, 3, 4];

  foreach ($three as $x) {
    if ($x == 2) {
      continue;
    }
    echo $x;
  }
  echo "\n";
}
