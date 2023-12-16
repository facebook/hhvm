<?hh

<<__EntryPoint>>
function main(): void {
  $code = "function x(): void { \$a = \"\xFF\xFE\xF0\xF0\xFF\"; echo \"evalcode\\n\"; }";
  var_dump(eval($code));
  x();
  echo "done\n";
}
