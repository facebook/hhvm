<?hh

function test() {
  $var1 = 'no_var';
  $var2 = 'var1';
  var_dump($no_var ?? 0);
  var_dump($var1 ?? 0);
  var_dump($GLOBALS['no_var'] ?? 0);
  var_dump($GLOBALS['global_var'] ?? 0);
}
<<__EntryPoint>>
function entrypoint_cget_quiet(): void {

  $GLOBALS['global_var'] = 2;

  test();
}
