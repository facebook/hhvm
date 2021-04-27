<?hh
<<file:__EnableUnstableFeatures('readonly')>>


function get_readonly_global(string $key)[policied]: void {
  // can't call from policied context
  $y = HH\global_get($key);
  // ok
  $x = readonly HH\global_readonly_get($key);

}
