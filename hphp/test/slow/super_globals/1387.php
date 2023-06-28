<?hh

function f() :mixed{
  var_dump(\HH\global_get('a'));
  \HH\global_set('a', -1);
  var_dump(\HH\global_get('a'));
}
<<__EntryPoint>>
function entrypoint_1387(): void {
  \HH\global_set('a', 100);
  f();
}
