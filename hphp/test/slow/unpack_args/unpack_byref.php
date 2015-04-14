<?hh
function foo(&$x) {}
function main() {
  $x = array(1);
  foo(...$x);
}
main();
