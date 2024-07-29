<?hh
function ret_true($x) :mixed{
 return true;
}
function dict_update(dict<arraykey, mixed> $d, arraykey $k, mixed $v): dict<arraykey, mixed> {
  $d[$k] = $v;
  return $d;
}
<<__EntryPoint>>
function entrypoint_1758(): void {

  switch (\HH\global_get('_POST')) {
  case dict[]: echo 'empty array';
   break;
  case \HH\global_get('_GET'):   echo 'get';
   break;
  default: echo 'default';
  }
  switch (\HH\global_get('_SERVER')) {
  case dict[]: echo 'empty array';
   break;
  default: echo 'default';
  }
  switch ((bool)\HH\global_get('_SERVER')) {
  case ret_true(\HH\global_set('_SERVER', dict_update(\HH\global_get('_SERVER'), 'foo', 10))): echo '1';
   break;
  case dict[];
   echo '2';
   break;
  default: echo '3';
  }
  var_dump(\HH\global_get('_SERVER')['foo']);
}
