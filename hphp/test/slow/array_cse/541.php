<?hh


<<__EntryPoint>>
function main_541() {
\HH\global_set('foo', 10);
\HH\global_set(
  'bar',
  darray[
      10 => varray[\HH\global_get('foo')],
      20 => varray[\HH\global_get('foo')]],
);
var_dump(\HH\global_get('bar'));
}
