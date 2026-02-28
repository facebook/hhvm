<?hh

<<__EntryPoint>>
function entrypoint_anonymous_func_002(): void {

  $test = $v ==> $v;

  \HH\global_set('arr', vec[() ==> \HH\global_get('arr'), 2]);

  var_dump(\HH\global_get('arr')[$test(1)]);
  var_dump(\HH\global_get('arr')[$test(0)]() == \HH\global_get('arr'));
}
