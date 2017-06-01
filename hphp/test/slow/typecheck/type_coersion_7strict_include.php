<?php
declare(strict_types=1);

require_once('weak.inc');
require_once('strict.inc');

require_once('fix_exceptions.inc');
function check($kind, $user_fn) {
  echo "\n$kind\n";
  foreach ([True, 1, 3.14, "abc", [1, 2, 3], null] as $k => $v) {
    try {
      $user_fn($v);
      printf("No Error\n");
    } catch(TypeError $e) {
      $msg = fix_err($e->getMessage(), true);
      printf("[user] %s\n", $msg);
    }
  }
}

function strict() {
  check("Boolean", function ($v) { return \Strict\fBool($v); });
  check("Int64", function ($v) { return \Strict\fInt($v); });
  check("Double", function ($v) { return \Strict\fFloat($v); });
  check("String", function ($v) { return \Strict\fString($v); });
  check("Array", function ($v) { return \Strict\fArray($v); });
  check("Object", function ($v) { return \Strict\fObject($v); });
}
function weak() {
  check("Boolean", function ($v) { return \Weak\fBool($v); });
  check("Int64", function ($v) { return \Weak\fInt($v); });
  check("Double", function ($v) { return \Weak\fFloat($v); });
  check("String", function ($v) { return \Weak\fString($v); });
  check("Array", function ($v) { return \Weak\fArray($v); });
  check("Object", function ($v) { return \Weak\fObject($v); });
}
strict();
weak();
