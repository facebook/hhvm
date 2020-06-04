<?hh
# NOTE: The assignments above must run at top-level.
<<__EntryPoint>>
function entrypoint_unset_cv08(): void {
  $a = "ok\n";
  $b = "ok\n";
  @array_unique($GLOBALS['GLOBALS']);
  echo $a;
  echo $b;
  echo "ok\n";
}
