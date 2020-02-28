<?hh

function foo(inout $arr) {
  if ($arr[0] == 0) return true;
  $arr[0] = 0;
}

function main() {
  $arr = darray[];
  $arr[0] = -1;
  while (true) {
    if (foo(inout $arr)) break;
  }
}


<<__EntryPoint>>
function main_loop_boxiness1() {
main();
main();
main();
echo "Done\n";
}
