<?hh

function main() {
  $a = miarray();
  $a[0] = 10;
  $a[-20] = array(1,2,3,4);
  $a[50] = "some string";
  var_dump($a);
}

main();
