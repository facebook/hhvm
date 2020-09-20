<?hh
<<__EntryPoint>>
function entrypoint_unset_cv02(): void {
  \HH\global_set("x", "ok\n");
  echo \HH\global_get("x");
  \HH\global_unset("x");
  echo \HH\global_get("x");
}
