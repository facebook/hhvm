<?hh
function main() {
  $x = varray();
  $y = $x['foo'];
  $x['foo'] = 1;
  var_dump($x);
}
main();
