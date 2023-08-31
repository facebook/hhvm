<?hh

<<__EntryPoint>>
function test_unsetting_cookies_entrypoint(): void {
  var_dump(
    isset($_COOKIE),
    isset($_COOKIE) ? \HH\Lib\Dict\sort_by_key($_COOKIE) : null,
    \HH\Lib\Dict\sort_by_key($_REQUEST),
    $_SERVER['HTTP_COOKIE'], // confirm it's still in $_SERVER
  );
}
