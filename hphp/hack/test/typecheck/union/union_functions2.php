<?hh // strict
function foo(): (function(float):mixed) {
  $f = $x ==> $x;
  if (count([])) {
    $f = fun('log1p');
  }
  return $f;
}
