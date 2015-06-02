<?hh // strict

$x = null;
if (false) {
  $x?->y += 1; // parse error
}
