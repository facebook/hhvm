<?hh

<<__ALWAYS_INLINE>>
function red() :mixed{
  trigger_error("test triggering error");
}

<<__ALWAYS_INLINE>>
function green() :mixed{
  trigger_error("test triggering error");
  red();
}

<<__ALWAYS_INLINE>>
function blue() :mixed{
  trigger_error("test triggering error");
  green();
}

function main() :mixed{
  blue();
}
<<__EntryPoint>>
function entrypoint_inlineeagerstack(): void {

  set_error_handler(($errno, $errstr, $errfile, $errline, $errctx) ==> {
    $bfile = basename($errfile);
    $trace = implode(
      ', ',
      array_map($x ==> $x['function'].':'.($x['line'] ?? '?'), debug_backtrace())
    );
    echo "[$errno] $errstr $bfile@$errline: $trace\n";
    return true;
  });

  for ($i = 0; $i < 10; $i++) main();
}
