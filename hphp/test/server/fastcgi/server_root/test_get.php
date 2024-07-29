<?hh

<<__EntryPoint>>
function test_get_entrypoint() :mixed{
  var_dump(\HH\global_get('_GET')['name']);
}
