<?hh // strict

function test(bool $b, mixed $x, $y): void {
  hh_show($x);
  if (!HH\is_php_array($x)) {
    $x = $y;
    if ($x === null) { // no complains here as $x is Tany
    }
  }
  if ($x === null) { // there shouldn't be any complain here either
  }
}
