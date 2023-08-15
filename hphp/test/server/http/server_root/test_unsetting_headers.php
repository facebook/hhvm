<?hh

<<__EntryPoint>>
function test_unsetting_headers_entrypoint(): void {
  $headers = \HH\get_headers_secure();
  var_dump(
    \HH\Lib\Dict\sort_by_key($headers),
    \HH\Lib\Dict\sort_by_key(\HH\Lib\Dict\filter_keys(
      $_SERVER, $k ==> \HH\Lib\Str\starts_with($k, 'HTTP_'),
    )),
  );
}
