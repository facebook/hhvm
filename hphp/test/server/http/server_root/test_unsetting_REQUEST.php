<?hh

<<__EntryPoint>>
function test_unsetting_request_entrypoint(): void {
  var_dump(
    \HH\Lib\Dict\sort_by_key($_GET),
    isset($_REQUEST),
    isset($_REQUEST) ? \HH\Lib\Dict\sort_by_key($_REQUEST) : $_REQUEST,
  );
}
