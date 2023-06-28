<?hh

function return_true() :mixed{
 return true;
 }
function f(inout $x, $y) :mixed{
  $x = $y;
  if (return_true())
    unset($x);
  $x = 0;
}

<<__EntryPoint>>
function main_1118() :mixed{
$myvar = 10;
f(inout $myvar, 30);
var_dump($myvar);
}
