<?hh
<<__EntryPoint>>
function entrypoint_1235(): void {

  var_dump(\HH\global_get('argc'), count(\HH\global_get('argv')));
  var_dump(\HH\global_get('_SERVER')['argc'], count(\HH\global_get('_SERVER')['argv']));
}
