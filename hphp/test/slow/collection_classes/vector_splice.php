<?hh

function main($a) {
  var_dump($a);
  $a->splice(3, 1);
  var_dump($a);
}

main(Vector { 1, 2, 3, 4 });
