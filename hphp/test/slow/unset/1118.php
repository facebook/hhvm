<?hh

function return_true() {
 return true;
 }
function f(inout $x, $y) {
  $x = $y;
  if (return_true())
    unset($x);
  $x = 0;
}

<<__EntryPoint>>
function main_1118() {
$myvar = 10;
f(inout $myvar, 30);
var_dump($myvar);
}
