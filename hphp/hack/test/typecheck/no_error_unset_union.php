<?hh // strict

function test_unset(bool $cond): void {
  $container = $cond ? keyset[1] : dict[1 => 'One'];
  unset($container[1]);
}
