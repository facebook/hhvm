<?hh

function non_bool_condition(int $x, string $s, bool $bool): void {
  if ($x) {
    ;
  }
  while ($x) {
    ;
  }
  do {
    ;
  } while ($x);
  for ($i = 0; $x; $i++) {
  }
  $x && $bool;
  $bool && $x;
  $x || $bool;
  $bool || $x;
  $x && $s;
  !$x;
  $x ? 'consq' : 'alt';
}
