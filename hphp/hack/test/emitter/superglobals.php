<?hh // strict

function test(): void {
  /* HH_FIXME[2050] */
  $GLOBALS['hi'] = 10;
  /* HH_FIXME[2050] */
  var_dump($GLOBALS['hi']);
  /* HH_FIXME[2050] */
  var_dump($GLOBALS["hi"]);
  /* HH_FIXME[2050] */
  var_dump($_ENV['HOME']);
}
