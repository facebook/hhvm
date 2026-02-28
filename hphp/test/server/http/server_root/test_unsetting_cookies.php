<?hh

<<__EntryPoint>>
function test_unsetting_cookies_entrypoint(): void {
  var_dump(
    \HH\global_isset('_COOKIE'),
    \HH\global_isset('_COOKIE') ? \HH\Lib\Dict\sort_by_key(\HH\global_get('_COOKIE')) : null,
    \HH\Lib\Dict\sort_by_key(\HH\global_get('_REQUEST')),
    \HH\global_get('_SERVER')['HTTP_COOKIE'], // confirm it's still in \HH\global_get('_SERVER')
  );
}
