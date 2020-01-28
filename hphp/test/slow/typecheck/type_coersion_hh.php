<?hh

function errHandler($errno, $errmsg, $file, $line) {
  $errmsg = str_replace('long', 'integer', $errmsg);
  printf("WARNING: $errmsg\n");
  return true;
}

function check($kind, $builtin_fn) {
  echo "\n$kind\n";
  foreach (varray[True, 1, 3.14, "abc", varray[1, 2, 3], null] as $k => $v) {
    try { $builtin_fn($v); } catch (Exception $e) { echo 'WARNING: '.$e->getMessage()."\n"; }
  }
}


<<__EntryPoint>>
function main_type_coersion_hh() {
set_error_handler(fun('errHandler'), E_WARNING);

check("Boolean", function ($v) { return sha1("abc", $v); });
check("Int64", function ($v) { return str_pad("abc", $v); });
check("Double", function ($v) { return number_format($v); });
check("String", function ($v) { return rtrim($v); });
check("Array",
      function ($v) { return __hhvm_intrinsics\dummy_array_builtin($v); },
      function (array $v) { });
check("Object", function ($v) { return get_object_vars($v); });
}
