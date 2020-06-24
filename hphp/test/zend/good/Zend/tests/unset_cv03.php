<?hh
<<__EntryPoint>>
function main_entry(): void {
  \HH\global_set('x', "ok\n");
  echo \HH\global_get('x');
  include "unset.inc";
  unset_();
  echo \HH\global_get('x');
}
