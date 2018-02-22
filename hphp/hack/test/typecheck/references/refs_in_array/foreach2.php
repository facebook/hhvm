<?hh // strict

function test(): void {
  $x = array();
  foreach ($x as $k => &$v) {
  }
}
