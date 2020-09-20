<?hh

function foo(inout dict<int, int> $arr) {
  foreach ($arr as $val) {
    if ($arr[$val]) {
      $arr = varray[];
      return true;
    }
  }
  return false;
}

function main() {
  $d = dict[0=>0];
  foo(inout $d);
  $d = dict[1=>1];
  foo(inout $d);
  return $d;
}


<<__EntryPoint>>
function main_inout_loop_1() {
set_error_handler(() ==> { throw new Exception; });

try {
  main();
} catch (Exception $e) {
  echo "caught!\n";
}
}
