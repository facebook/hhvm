<?hh

function errHandler($errno, $errmsg, $file, $line) :mixed{
  if (HH\Lib\Str\contains($errmsg, "@")) {
    printf("WARNING: ".$errmsg."\n");
    return true;
  }
  throw new Exception($errmsg);
}

function check($kind, $builtin_fn, $user_fn) :mixed{
  echo "\n$kind\n";
  foreach (vec[True, 1, 3.14, "abc", vec[1, 2, 3], null] as $k => $v) {
    printf("Builtin:\n");
    try { $builtin_fn($v); } catch (Exception $e) { echo 'WARNING: '.$e->getMessage()."\n"; }
    printf("User:\n");
    $user_fn($v);
  }
}


<<__EntryPoint>>
function main_type_coersion_5(): mixed {
  set_error_handler(errHandler<>, E_ALL);

  check("Boolean", function ($v) { return sha1("abc", $v); },
        function (<<__Soft>> bool $v) { });
  check("Int64", function ($v) { return str_pad("abc", $v); },
        function (<<__Soft>> int $v) { });
  check("Double", function ($v) { return number_format($v); },
        function (<<__Soft>> float $v) { });
  check("String", function ($v) { return rtrim($v); },
        function (<<__Soft>> string $v) { });
  check("varray",
        function ($v) { return __hhvm_intrinsics\dummy_varray_builtin($v); },
        function (<<__Soft>> varray $v) { });
  check("Object", function ($v) { return get_object_vars($v); },
        function (<<__Soft>> object $v) { });
}
