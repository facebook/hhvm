<?hh

function main() {
  $v = vec["bar"];
  $v[1] = 1;
  var_dump($v);
}

main();
