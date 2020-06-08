<?hh

<<__ALWAYS_INLINE>>
function junk() {
  junk2();
}

<<__ALWAYS_INLINE>>
function red() {
  junk();
  trigger_error('test_error');
}

<<__ALWAYS_INLINE>>
function green() {
  junk();
  trigger_error('test_error');
  red();
}

<<__ALWAYS_INLINE>>
function blue() {
  junk();
  trigger_error('test_error');
  green();
}

<<__EntryPoint>>
function main() {
  set_error_handler(($errno, $errstr, $errfile, $errline, $errctx) ==> {
    if ($errno !== E_USER_NOTICE || strpos($errstr, 'test_error') === false) return;
    $bfile = basename($errfile);
    $trace = implode(
      ', ',
      array_map($x ==> $x['function'].':'.($x['line'] ?? '?'), debug_backtrace())
    );
    echo "[$errno] $errstr $bfile@$errline: $trace\n";
    return true;
  });

  for ($i = 0; $i < 10; $i++) {
    junk();
    blue();
  }
}

<<__ALWAYS_INLINE>>
function junk2() {
  junk3();
}

<<__ALWAYS_INLINE>>
function junk3() {
  hphp_array_idx(varray[0], '0', 0);
}
