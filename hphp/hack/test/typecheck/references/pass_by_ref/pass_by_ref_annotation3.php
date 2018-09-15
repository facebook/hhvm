<?hh // strict

function test(): void {
  $a = array('red' => 4, 'apple' => 12, 'foo' => 1);

  sort(&$a);
  ksort($a);
}
