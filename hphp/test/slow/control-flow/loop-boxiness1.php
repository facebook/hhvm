<?hh

function foo(inout $arr) :mixed{
  if ($arr[0] == 0) return true;
  $arr[0] = 0;
}

function main() :mixed{
  $arr = dict[];
  $arr[0] = -1;
  while (true) {
    if (foo(inout $arr)) break;
  }
}


<<__EntryPoint>>
function main_loop_boxiness1() :mixed{
main();
main();
main();
echo "Done\n";
}
