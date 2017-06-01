<?php

require_once('fix_exceptions.inc');

function errHandler($errno, $errmsg, $file, $line) {
  $errmsg = str_replace('long', 'integer', $errmsg);
  printf("WARNING: ".fix_err($errmsg, false)."\n");
  return true;
}

set_error_handler('errHandler', E_ALL);

function check($kind, $builtin_fn, $user_fn) {
  echo "\n$kind\n";
  foreach ([True, 1, 3.14, "abc", [1, 2, 3], null] as $k => $v) {
    printf("Builtin:\n");
    $builtin_fn($v);
    printf("User:\n");
    $user_fn($v);
  }
}

check("Boolean", function ($v) { return sha1("abc", $v); },
      function (boolean $v) { });
check("Int64", function ($v) { return str_pad("abc", $v); },
      function (integer $v) { });
check("Double", function ($v) { return number_format($v); },
      function (float $v) { });
check("String", function ($v) { return rtrim($v); },
      function (string $v) { });
check("Array", function ($v) { return array_count_values($v); },
      function (array $v) { });
check("Object", function ($v) { return get_object_vars($v); },
      function (object $v) { });
