<?hh

function f() {
  var_dump($GLOBALS['a']);
  $GLOBALS['a'] = -1;
  var_dump($GLOBALS['a']);
}
<<__EntryPoint>>
function entrypoint_1387(): void {
  $GLOBALS['a'] = 100;
  f();
}
