<?hh

set_error_handler(() ==> { throw new Exception; });

function foo(inout dict<int, int> $arr) {
  foreach ($arr as $val) {
    if ($arr[$val]) {
      $arr = array();
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

try {
  main();
} catch (Exception $e) {
  echo "caught!\n";
}
