<?hh // strict
function foo(): (function(mixed):mixed) {
  $f = $x ==> $x;
  if (count([])) {
    $f = fun('log1p');
  }
  return $f;
}
