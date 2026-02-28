<?hh

function pt($f) :mixed{
  $trace = implode(
    ', ',
    array_map($x ==> $x['function'].':'.($x['line'] ?? '???'), debug_backtrace())
  );
  echo "$f: $trace\n";
}

<<__ALWAYS_INLINE>>
function red() :mixed{
  pt(__FUNCTION__);
}

<<__ALWAYS_INLINE>>
function green() :mixed{
  pt(__FUNCTION__);
  red();
}

<<__ALWAYS_INLINE>>
function blue() :mixed{
  pt(__FUNCTION__);
  green();
}

function main() :mixed{
  blue();
}
<<__EntryPoint>> function main_entry(): void {
for ($i = 0; $i < 10; $i++) main();
}
