<?hh

function test() :mixed{
  $var1 = 'no_var';
  $var2 = 'var1';
  var_dump($no_var ?? 0);
  var_dump($var1 ?? 0);
}
<<__EntryPoint>>
function entrypoint_cget_quiet(): void {
  test();
}
