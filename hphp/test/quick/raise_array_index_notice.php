<?hh

function rain() {
  $arr = array();
  for ($i = 0; $i < 4; $i++) {
    $arr[$i] = $i;
  }
  for ($i = 0; $i < 5; $i++) {
    print($arr[$i]."\n");
  }
}

function main() {
  rain();
  print("not_reached\n");
}

main();
