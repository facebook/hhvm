<?hh

function test(): void {
  $GLOBALS['hi'] = 10;
  var_dump($GLOBALS['hi']);
  var_dump($GLOBALS["hi"]);
  var_dump($_ENV['HOME']);
}
