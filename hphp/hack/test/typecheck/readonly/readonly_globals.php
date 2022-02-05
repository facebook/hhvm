<?hh

function get_readonly_global(string $key)[zoned]: void {
  // can't call from zoned context
  $y = HH\global_get($key);
  // ok
  $x = readonly HH\global_readonly_get($key);

}
