<?hh
function bar() : void {
  $a = 11;
  $b = "This ${a} is an interpolated   {$a}    string.";
}
