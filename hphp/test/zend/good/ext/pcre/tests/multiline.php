<?hh
<<__EntryPoint>> function main(): void {
  var_dump(preg_match_all('/^.{2,3}$/', "aei\nou"));
  var_dump(preg_match_all('/^.{2,3}$/', "aei\nou\n"));
  var_dump(preg_match_all('/^.{2,3}$/m', "aei\nou"));
  var_dump(preg_match_all('/^.{2,3}$/m', "aei\nou\n"));

  echo "done\n";
}
