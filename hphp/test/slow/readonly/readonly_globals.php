<?hh
<<file:__EnableUnstableFeatures('readonly')>>


function get_readonly_global(string $key)[read_globals]: void {
  $y = readonly HH\global_readonly_get($key);
  echo $y;
}

<<__EntryPoint>>
function test(): void {
  HH\global_set("foo", 5);
  get_readonly_global("foo");
}
