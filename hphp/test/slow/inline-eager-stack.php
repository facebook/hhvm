<?hh

set_error_handler(($errno, $errstr, $errfile, $errline, $errctx) ==> {
  $bfile = basename($errfile);
  $trace = implode(
    ', ',
    array_map($x ==> $x['function'].':'.($x['line'] ?? '?'), debug_backtrace())
  );
  echo "[$errno] $errstr $bfile@$errline: $trace\n";
  return true;
});

<<__ALWAYS_INLINE>>
function red() {
  get_called_class();
}

<<__ALWAYS_INLINE>>
function green() {
  get_called_class();
  red();
}

<<__ALWAYS_INLINE>>
function blue() {
  get_called_class();
  green();
}

function main() {
  blue();
}

for ($i = 0; $i < 10; $i++) main();
