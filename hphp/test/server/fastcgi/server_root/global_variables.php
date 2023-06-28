<?hh

<<__EntryPoint>>
function global_variables_entrypoint() :mixed{
  var_dump(HH\global_get('HTTP_RAW_POST_DATA'));
  var_dump(count($_ENV) > 0);
  var_dump($_GET);
  var_dump($_POST);
  var_dump($_COOKIE);
  var_dump(count($_SERVER) > 0);
  var_dump($_REQUEST);
}
