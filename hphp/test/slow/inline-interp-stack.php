<?hh

set_error_handler(($errno, $errstr, $errfile, $errline, $errctx) ==> {
  if ($errno !== 8 || strpos($errstr, 'Intish') !== false) return;
  $bfile = basename($errfile);
  $trace = implode(
    ', ',
    array_map($x ==> $x['function'].':'.($x['line'] ?? '?'), debug_backtrace())
  );
  echo "[$errno] $errstr $bfile@$errline: $trace\n";
  return true;
});

<<__ALWAYS_INLINE>>
function junk() {
  junk2();
}

<<__ALWAYS_INLINE>>
function red($x, $y) {
  junk();
  hphp_array_idx($x, $y, 0);
}

<<__ALWAYS_INLINE>>
function green($x, $y) {
  junk();
  hphp_array_idx($x, $y, 0);
  red($x, $y);
}

<<__ALWAYS_INLINE>>
function blue($x, $y) {
  junk();
  hphp_array_idx($x, $y, 0);
  green($x, $y);
}

function main($x, $y) {
  junk();
  blue($x, $y);
}

for ($i = 0; $i < 10; $i++) main(varray[], false);

<<__ALWAYS_INLINE>>
function junk2() {
  junk3();
}

<<__ALWAYS_INLINE>>
function junk3() {
  hphp_array_idx(varray[0], '0', 0);
}
