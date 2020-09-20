<?hh

function main(bool $x, bool $y) {
  if ($x && $y) {
    $z = $x == $y;
    var_dump($z);
  }
}


<<__EntryPoint>>
function main_jmp_local_003() {
main(true, true);
}
