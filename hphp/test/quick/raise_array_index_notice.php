<?hh

function rain() {
  $arr = darray[];
  for ($i = 0; $i < 4; $i++) {
    $arr[$i] = $i;
  }
  for ($i = 0; $i < 5; $i++) {
    print($arr[$i]."\n");
  }
}
<<__EntryPoint>>
function main() {
  rain();
  print("not_reached\n");
}
