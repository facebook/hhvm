<?hh

function main(string $x = null) {
  if (!$x) {
    var_dump($x);
  }
}


<<__EntryPoint>>
function main_jmp_local_005() {
main("");
main("0");
}
