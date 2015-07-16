<?hh // strict

function test(): string {
  $s = shape();
  $s['x'] = 3;
  $s['x'] = 'aaa';
  return $s['x'];
}
