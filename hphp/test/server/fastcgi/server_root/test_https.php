<?hh

<<__EntryPoint>>
function test_https_entrypoint() :mixed{
  var_dump(\HH\global_get('_SERVER')['HTTPS']);
}
