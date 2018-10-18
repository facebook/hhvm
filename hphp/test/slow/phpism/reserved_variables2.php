<?hh //strict

<<__EntryPoint>>
function f(): void {
  ini_set('track_errors', true);
  @strpos();
  var_dump($php_errormsg);
}
