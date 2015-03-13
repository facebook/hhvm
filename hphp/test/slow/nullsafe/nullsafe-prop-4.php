<?hh // strict

$x = null;
if (false) {
  unset($x?->y); // parse error
}
