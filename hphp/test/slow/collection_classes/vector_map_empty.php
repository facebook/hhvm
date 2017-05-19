<?hh

function main() {
  $v = Vector {};
  var_dump($v->map($x ==> $x + 1));
}
main();
