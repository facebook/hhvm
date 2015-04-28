<?hh
function foo(&$x) { var_dump($x); $x = 5; }
function main() {
  $x = array(1);
  foo(...$x);
  var_dump($x);
  $y =& $x[0];
  foo(...$x);
  var_dump($x);
}
main();
