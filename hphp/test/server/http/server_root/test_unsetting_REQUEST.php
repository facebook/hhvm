<?hh

<<__EntryPoint>>
function test_unsetting_request_entrypoint(): void {
  var_dump(
    \HH\Lib\Dict\sort_by_key(\HH\global_get('_GET')),
    isset(\HH\global_get('_REQUEST')),
    isset(\HH\global_get('_REQUEST')) ? \HH\Lib\Dict\sort_by_key(\HH\global_get('_REQUEST')) : \HH\global_get('_REQUEST'),
  );
}
