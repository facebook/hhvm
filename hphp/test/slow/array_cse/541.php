<?hh


<<__EntryPoint>>
function main_541() :mixed{
\HH\global_set('foo', 10);
\HH\global_set(
  'bar',
  dict[
      10 => vec[\HH\global_get('foo')],
      20 => vec[\HH\global_get('foo')]],
);
var_dump(\HH\global_get('bar'));
}
