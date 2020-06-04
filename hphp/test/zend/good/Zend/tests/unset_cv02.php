<?hh
<<__EntryPoint>>
function entrypoint_unset_cv02(): void {
  $GLOBALS["x"] = "ok\n";
  echo $GLOBALS["x"];
  unset($GLOBALS["x"]);
  echo $GLOBALS["x"];
}
