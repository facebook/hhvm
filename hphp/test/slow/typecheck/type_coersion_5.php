<?hh

function errHandler($errno, $errmsg, $file, $line) {
  $errmsg = str_replace('long', 'integer', $errmsg);
  printf("WARNING: ".fix_err($errmsg, false)."\n");
  return true;
}

function check($kind, $builtin_fn, $user_fn) {
  echo "\n$kind\n";
  foreach (varray[True, 1, 3.14, "abc", varray[1, 2, 3], null] as $k => $v) {
    printf("Builtin:\n");
    try { $builtin_fn($v); } catch (Exception $e) { echo 'WARNING: '.$e->getMessage()."\n"; }
    printf("User:\n");
    $user_fn($v);
  }
}


<<__EntryPoint>>
function main_type_coersion_5() {
require_once('fix_exceptions.inc');

set_error_handler(fun('errHandler'), E_ALL);

check("Boolean", function ($v) { return sha1("abc", $v); },
      function (@bool $v) { });
check("Int64", function ($v) { return str_pad("abc", $v); },
      function (@int $v) { });
check("Double", function ($v) { return number_format($v); },
      function (@float $v) { });
check("String", function ($v) { return rtrim($v); },
      function (@string $v) { });
check("Array",
      function ($v) { return __hhvm_intrinsics\dummy_array_builtin($v); },
      function (@array $v) { });
check("Object", function ($v) { return get_object_vars($v); },
      function (@object $v) { });
}
