<?hh

function breaker(inout $x) :mixed{
  $x = (string)mt_rand();
}

<<__EntryPoint>> function foo(): void {
  $x = "";
  breaker(inout $x);
  // Bug #2240782: HHIR needs to think of $x as a Str, not a
  // StaticStr here.
  echo "Num: ";
  echo $x;
  echo "\n";
}
