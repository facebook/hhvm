<?hh


function jmpfuse($str) {
  $pos = 0;
  while (strlen($str) > $pos + 2 &&
         $str[$pos] == '/' &&
         $str[$pos + 1] == '*' &&
         ($pos = strpos($str, '*/', $pos + 2)) !== false) {
    $pos += 2;
  }
  return true;
}

function main() {
  foreach (range(1, 100) as $i) {
    jmpfuse("blah blah\n");
  }
  var_dump("jumps fused?");
}

main();
