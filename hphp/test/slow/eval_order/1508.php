<?hh

class a {
  function r(inout $x) {
    $x = 20;
  }
}
function id($x) {
 return $x;
 }

<<__EntryPoint>>
function main_1508() {
$a = new a();
$x = null;
id($a)->r(inout $x);
var_dump($x);
}
