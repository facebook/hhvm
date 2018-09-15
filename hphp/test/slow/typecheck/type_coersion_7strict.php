<?php
declare(strict_types=1);
require_once('fix_exceptions.inc');

function check($kind, $builtin_fn, $user_fn) {
  echo "\n$kind\n";
  foreach ([True, 1, 3.14, "abc", [1, 2, 3], null] as $k => $v) {
    try {
      $builtin_fn($v);
      printf("No Error\n");
    } catch(TypeError $e) {
      $msg = fix_err($e->getMessage(), false);
      printf("[builtin] %s\n", $msg);
    }
    try {
      $user_fn($v);
      printf("No Error\n");
    } catch(TypeError $e) {
      $msg = fix_err($e->getMessage(), true);
      printf("[user] %s\n", $msg);
    }
  }
}

check("Boolean", function ($v) { return sha1("abc", $v); },
      function (bool $v) { });
check("Int64", function ($v) { return str_pad("abc", $v); },
      function (int $v) { });
check("Double", function ($v) { return number_format($v); },
      function (float $v) { });
check("String", function ($v) { return rtrim($v); }, function (string $v) { });
check("Array",
      function ($v) { return __hhvm_intrinsics\dummy_array_builtin($v); },
      function (array $v) { });
check("Object", function ($v) { return get_object_vars($v); },
      function (object $v) { });
