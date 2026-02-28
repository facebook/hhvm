<?hh

<<__EntryPoint>>
function entrypoint_autoloadfailhoist(): void {
  $x = new X();
  $x->method();
  echo 'NOTE: repo-mode doesn\'t invoke the autoloader at all (I1 is in repo)', "\n";
  echo 'Done', "\n";
}
