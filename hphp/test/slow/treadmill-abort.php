<?hh

<<__EntryPoint>>
function main(): void {
  set_time_limit(60*60*24);
  sleep(10);
  __hhvm_intrinsics\check_for_stuck_treadmill();
  echo "DONE\n";
}
