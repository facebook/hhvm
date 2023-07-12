<?hh

<<__EntryPoint>>
function main() {
  if (function_exists('foo')) var_dump(foo());
  if (function_exists('bar')) var_dump(bar());
  if (function_exists('baz')) var_dump(baz());
  echo "Done.\n";
}
