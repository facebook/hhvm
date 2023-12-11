<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(handler<>);
  handler(E_NOTICE, 'my test', __FILE__, __LINE__);
  trigger_error('foobar');
}

class Handler {
  static function derp($errno, $errstr, $errfile, $errline, ...) :mixed{
    $errnames = dict[
      E_NOTICE => 'E_NOTICE',
      E_WARNING => 'E_WARNING',
      E_USER_NOTICE => 'E_USER_NOTICE',
    ];
    echo "Handler: $errnames[$errno]: $errstr in $errfile:$errline\n";
  }
}

function handler(...$args) :mixed{
  $h = __hhvm_intrinsics\launder_value(Handler::class);
  $h::derp(...$args);
}
