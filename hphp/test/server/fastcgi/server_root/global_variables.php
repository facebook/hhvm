<?hh

<<__EntryPoint>>
function global_variables_entrypoint() :mixed{
  var_dump(HH\global_get('HTTP_RAW_POST_DATA'));
  var_dump(count(\HH\global_get('_ENV')) > 0);
  var_dump(\HH\global_get('_GET'));
  var_dump(\HH\global_get('_POST'));
  var_dump(\HH\global_get('_COOKIE'));
  var_dump(count(\HH\global_get('_SERVER')) > 0);
  var_dump(\HH\global_get('_REQUEST'));
}
