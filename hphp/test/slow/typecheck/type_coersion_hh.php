<?hh

function errHandler($errno, $errmsg, $file, $line) :mixed{
  throw new Exception($errmsg);
}

function check($kind, $builtin_fn) :mixed{
  echo "\n$kind\n";
  foreach (vec[True, 1, 3.14, "abc", vec[1, 2, 3], null] as $k => $v) {
    try { $builtin_fn($v); } catch (Exception $e) { echo 'WARNING: '.$e->getMessage()."\n"; }
  }
}


<<__EntryPoint>>
function main_type_coersion_hh() :mixed{
set_error_handler(errHandler<>);

check("Boolean", function ($v) { return sha1("abc", $v); });
check("Int64", function ($v) { return str_pad("abc", $v); });
check("Double", function ($v) { return number_format($v); });
check("String", function ($v) { return rtrim($v); });
check("varray",
      function ($v) { return __hhvm_intrinsics\dummy_varray_builtin($v); },
      function (varray $v) { });
check("Object", function ($v) { return get_object_vars($v); });
}
