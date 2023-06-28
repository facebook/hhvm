<?hh

class a {
  function r(inout $x) :mixed{
    $x = 20;
  }
}
function id($x) :mixed{
 return $x;
 }

<<__EntryPoint>>
function main_1508() :mixed{
$a = new a();
$x = null;
id($a)->r(inout $x);
var_dump($x);
}
