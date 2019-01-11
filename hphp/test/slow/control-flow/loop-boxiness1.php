<?php

function foo(&$arr) {
  if ($arr[0] == 0) return true;
  $arr[0] = 0;
}

function main() {
  $arr = array();
  $arr[0] = -1;
  while (true) {
    if (foo(&$arr)) break;
  }
}


<<__EntryPoint>>
function main_loop_boxiness1() {
main();
main();
main();
echo "Done\n";
}
