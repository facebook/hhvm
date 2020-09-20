<?hh

function pt($f) {
  $trace = implode(
    ', ',
    array_map($x ==> $x['function'].':'.($x['line'] ?? '???'), debug_backtrace())
  );
  echo "$f: $trace\n";
}

<<__ALWAYS_INLINE>>
function red() {
  pt(__FUNCTION__);
}

<<__ALWAYS_INLINE>>
function green() {
  pt(__FUNCTION__);
  red();
}

<<__ALWAYS_INLINE>>
function blue() {
  pt(__FUNCTION__);
  green();
}

function main() {
  blue();
}
<<__EntryPoint>> function main_entry(): void {
for ($i = 0; $i < 10; $i++) main();
}
