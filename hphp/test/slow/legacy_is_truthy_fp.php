<?hh

<<__EntryPoint>>
function test(): void {
  $fp = HH\legacy_is_truthy<>;
  var_dump($fp("0"));
}
