<?hh

function dict_update(dict<arraykey, mixed> $d, arraykey $k, mixed $v): dict<arraykey, mixed> {
  $d[$k] = $v;
  return $d;
}

function foo($e, $m) :mixed{
  \HH\global_set('_REQUEST', dict_update(\HH\global_get('_REQUEST'), '_foo',  $e));
  \HH\global_set('_REQUEST', dict_update(\HH\global_get('_REQUEST'), '_bar',  $m));
  return $e;
}
function test($x) :mixed{
  return foo('a', $x);
}

<<__EntryPoint>>
function main_1830() :mixed{
var_dump(test('b'));
}
